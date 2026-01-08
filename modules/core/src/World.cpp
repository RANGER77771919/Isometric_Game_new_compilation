/**
 * @file World.cpp
 * @brief Implementación de la clase World
 *
 * Implementa la generación procedural del mundo usando ruido Perlin,
 * gestión dinámica de chunks, y sistema de biomas.
 */

#define FNL_IMPL
#include "core/World.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>  // Para std::clamp, std::max
#include <vector>     // Para std::vector

/**
 * @brief Constructor del mundo
 * @param seed Semilla para generación (0 usa time(nullptr))
 *
 * Inicializa el generador Mersenne Twister y configura los 3
 * generadores de ruido con parámetros óptimos para terreno.
 * Inicia el thread de background para generación de chunks.
 */
World::World(uint32_t seed)
    : m_seed(seed)
    , m_rng(seed)
    , m_biomeCacheInitialized(true)  // Cache se inicializa ahora en constructor
    , m_chunkGeneratorShouldStop(false)  // Inicializar flag de thread
{
    initNoiseGenerators();

    // OPTIMIZACIÓN FASE 1: Inicializar cache de biomas en constructor (no lazy)
    // Esto elimina el micro-stutter del primer frame
    for (int i = 0; i < BIOME_CACHE_SIZE; i++) {
        // Mapear índice [0, 1023] a valor de ruido [-1, 1]
        float value = (static_cast<float>(i) / BIOME_CACHE_SIZE) * 2.0f - 1.0f;

        if (value < -0.3f) {
            m_biomeCache[i] = BlockType::NIEVE;
        } else if (value < 0.0f) {
            m_biomeCache[i] = BlockType::HIERBA_SANGRE;
        } else if (value < 0.4f) {
            m_biomeCache[i] = BlockType::PASTO;
        } else if (value < 0.7f) {
            m_biomeCache[i] = BlockType::ARENA;
        } else {
            m_biomeCache[i] = BlockType::PASTO_FULL;
        }
    }

    // Pre-asignar espacio para chunks típicos (LOAD_RADIUS=5 -> ~121 chunks)
    m_chunks.reserve(150);

    // OPTIMIZACIÓN 6: Iniciar thread de background para generación de chunks
    m_chunkGeneratorThread = std::thread(&World::chunkGenerationWorker, this);
}

/**
 * @brief Destructor - detiene el thread de background
 *
 * Espera a que el thread de generación termine antes de destruir el mundo.
 */
World::~World() {
    // Señalar al thread que debe detenerse
    m_chunkGeneratorShouldStop = true;
    m_chunkQueueCV.notify_one();

    // Esperar a que el thread termine
    if (m_chunkGeneratorThread.joinable()) {
        m_chunkGeneratorThread.join();
    }
}

/**
 * @brief Inicializa los generadores de ruido Perlin
 *
 * Configura 3 generadores independientes:
 *
 * 1. Terreno (m_noiseTerrain):
 *    - Tipo: Perlin (suave, natural)
 *    - Frecuencia: 0.01 (baja, terreno grande)
 *    - Octavas: 4 (detalle moderado)
 *    - Lacunaridad: 2.0 (doble frecuencia por octava)
 *    - Ganancia: 0.5 (mitad de amplitud por octava)
 *    - Fractal: FBM (Fractal Brownian Motion)
 *    Resultado: colinas suaves con variación natural
 *
 * 2. Cuevas (m_noiseCaves):
 *    - Tipo: OpenSimplex2 (mejor para 3D)
 *    - Frecuencia: 0.05 (media, cuevas de tamaño medio)
 *    - Semilla: seed+1 (diferente al terreno)
 *    Resultado: cuevas irregulares y conectadas
 *
 * 3. Biomas (m_noiseBiome):
 *    - Tipo: Perlin
 *    - Frecuencia: 0.002 (muy baja, biomas MUY grandes ~2.5x más que antes)
 *    - Semilla: seed+2 (única)
 *    Resultado: regiones extensas de biomas distintos
 */
void World::initNoiseGenerators() {
    // Configurar ruido para terreno base
    m_noiseTerrain.SetNoiseType(FNL_NOISE_PERLIN);
    m_noiseTerrain.SetSeed(m_seed);
    m_noiseTerrain.SetFrequency(0.01f);           // Terreno suave y grande
    m_noiseTerrain.SetFractalOctaves(4);          // 4 niveles de detalle
    m_noiseTerrain.SetFractalLacunarity(2.0f);    // Cada octava tiene 2x frecuencia
    m_noiseTerrain.SetFractalGain(0.5f);          // Cada octava tiene 0.5x amplitud
    m_noiseTerrain.SetFractalType(FNL_FRACTAL_FBM);

    // Configurar ruido para cuevas (3D)
    m_noiseCaves.SetNoiseType(FNL_NOISE_OPENSIMPLEX2);
    m_noiseCaves.SetSeed(m_seed + 1);
    m_noiseCaves.SetFrequency(0.05f);             // Cuevas de tamaño medio

    // Configurar ruido para biomas
    m_noiseBiome.SetNoiseType(FNL_NOISE_PERLIN);
    m_noiseBiome.SetSeed(m_seed + 2);
    m_noiseBiome.SetFrequency(0.02f);            // Frecuencia muy baja = biomas MUY grandes (~2.5x más grandes)
}

/**
 * @brief Obtiene un bloque en coordenadas mundiales
 * @return Referencia al bloque o bloque inválido si el chunk no existe
 *
 * Proceso:
 * 1. Convertir coordenadas mundiales a ChunkPos
 * 2. Obtener el chunk (si no existe, devolver bloque inválido)
 * 3. Convertir coordenadas mundiales a locales del chunk
 * 4. Obtener bloque del chunk
 */
Block& World::getBlock(int x, int y, int z) {
    ChunkPos chunkPos = BlockUtils::worldToChunk(x, z);
    Chunk* chunk = getChunk(chunkPos);

    if (!chunk) {
        static Block invalidBlock(BlockType::AIRE);
        return invalidBlock;
    }

    int localX, localZ;
    BlockUtils::worldToLocal(x, z, localX, localZ);
    return chunk->getBlock(localX, y, localZ);
}

/** @brief Versión const de getBlock() */
const Block& World::getBlock(int x, int y, int z) const {
    ChunkPos chunkPos = BlockUtils::worldToChunk(x, z);
    const Chunk* chunk = getChunk(chunkPos);

    if (!chunk) {
        static Block invalidBlock(BlockType::AIRE);
        return invalidBlock;
    }

    int localX, localZ;
    BlockUtils::worldToLocal(x, z, localX, localZ);
    return chunk->getBlock(localX, y, localZ);
}

/**
 * @brief Establece un bloque en coordenadas mundiales
 *
 * Si el chunk no existe, la operación se ignora silenciosamente.
 * Útil para modificación del terreno por el jugador.
 *
 * OPTIMIZACIÓN: Usa BlockUtils en lugar de crear objetos temporales
 */
void World::setBlock(int x, int y, int z, BlockType type) {
    // OPTIMIZACIÓN: Usar BlockUtils directamente sin crear BlockPos temporal
    ChunkPos chunkPos = BlockUtils::worldToChunk(x, z);
    Chunk* chunk = getChunk(chunkPos);

    if (!chunk) {
        return;  // Chunk no existe, ignorar
    }

    int localX, localZ;
    BlockUtils::worldToLocal(x, z, localX, localZ);
    chunk->setBlock(localX, y, localZ, type);
}

/**
 * @brief Genera un chunk proceduralmente
 *
 * Proceso:
 * 1. Verificar si ya existe (evitar duplicados)
 * 2. Crear nuevo chunk vacío
 * 3. Generar terreno con ruido Perlin
 * 4. Marcar como generado
 * 5. Almacenar en el mapa
 */

/**
 * @brief Worker thread para generación asíncrona de chunks
 *
 * Este método corre en un thread separado y genera chunks en background.
 * Procesa la cola de chunks pendientes de generación.
 *
 * OPTIMIZACIÓN 6: Multithreaded Chunk Generation
 * - Evita bloquear el main thread durante generación de terreno
 * - Mejora la fluidez del juego al explorar nuevo territorio
 * - Usa condition_variable para esperar sin consumir CPU
 */
void World::chunkGenerationWorker() {
    while (true) {
        ChunkPos posToGenerate;

        // Esperar a que haya trabajo o señal de parada
        {
            std::unique_lock<std::mutex> lock(m_chunkQueueMutex);

            // Esperar hasta que haya chunks en la cola o debamos detenernos
            m_chunkQueueCV.wait(lock, [this] {
                return !m_chunkGenerationQueue.empty() || m_chunkGeneratorShouldStop;
            });

            // Verificar si debemos detenernos
            if (m_chunkGeneratorShouldStop && m_chunkGenerationQueue.empty()) {
                break;  // Salir del loop
            }

            // Si hay chunks en la cola, tomar el siguiente
            if (!m_chunkGenerationQueue.empty()) {
                posToGenerate = m_chunkGenerationQueue.front();
                m_chunkGenerationQueue.pop();
            } else if (m_chunkGeneratorShouldStop) {
                break;  // Señal de parada sin chunks pendientes
            } else {
                continue;  // Spurious wakeup, volver a esperar
            }
        }

        // Generar el chunk FUERA del lock (permite que otros threads agreguen a la cola)
        // Generar el chunk (llamada sincrónica interna al worker)
        std::unique_ptr<Chunk> chunk;

        // OPTIMIZACIÓN: Intentar reutilizar del object pool
        if (!m_chunkPool.empty()) {
            chunk = std::move(m_chunkPool.back());
            m_chunkPool.pop_back();
            chunk->clear();
            chunk->setPosition(posToGenerate);
        } else {
            chunk = std::make_unique<Chunk>(posToGenerate);
        }

        // Generar terreno
        generateTerrain(chunk.get());
        chunk->setGenerated(true);

        // Intentar insertar el chunk - si ya existe, descartar este
        // try_emplace es thread-safe: solo inserta si la clave no existe
        {
            std::lock_guard<std::mutex> lock(m_chunkQueueMutex);
            auto [it, inserted] = m_chunks.try_emplace(posToGenerate, std::move(chunk));

            // Si no se insertó (ya existía), devolver al object pool
            if (!inserted && it->second.get()) {
                // El chunk generado no se usó, devolverlo al pool
                // Nota: it->second es el chunk que ya existía, no el que acabamos de crear
                // El chunk que creamos se destruye automáticamente porque unique_ptr se movió
            }
        }
    }
}

/**
 * @brief Solicita generación asíncrona de un chunk
 * @param pos Posición del chunk a generar
 *
 * Agrega el chunk a la cola de generación para que el thread de
 * background lo genere. Si el chunk ya existe o ya está en cola,
 * no hace nada.
 *
 * OPTIMIZACIÓN 6: Non-blocking chunk generation
 */
void World::requestChunkGeneration(ChunkPos pos) {
    std::lock_guard<std::mutex> lock(m_chunkQueueMutex);

    // Verificar si ya existe
    if (m_chunks.find(pos) != m_chunks.end()) {
        return;  // Ya existe
    }

    // Agregar a la cola y notificar al worker
    m_chunkGenerationQueue.push(pos);
    m_chunkQueueCV.notify_one();
}

void World::generateChunk(ChunkPos pos) {
    // Verificar si ya existe
    if (m_chunks.find(pos) != m_chunks.end()) {
        return;
    }

    std::unique_ptr<Chunk> chunk;

    // OPTIMIZACIÓN: Intentar reutilizar del object pool
    if (!m_chunkPool.empty()) {
        // Reutilizar chunk del pool
        chunk = std::move(m_chunkPool.back());
        m_chunkPool.pop_back();

        // Limpiar y reconfigurar para nueva posición
        chunk->clear();
        chunk->setPosition(pos);
    } else {
        // Pool vacío, crear nuevo chunk
        chunk = std::make_unique<Chunk>(pos);
    }

    // Generar terreno
    generateTerrain(chunk.get());

    // Marcar como generado y almacenar
    chunk->setGenerated(true);
    m_chunks[pos] = std::move(chunk);
}

/**
 * @brief Genera terreno para un chunk completo
 *
 * Algoritmo para cada columna (x,z):
 *
 * 1. Calcular coordenadas mundiales:
 *    worldX = chunkX * CHUNK_SIZE + x
 *    worldZ = chunkZ * CHUNK_SIZE + z
 *
 * 2. Obtener altura del terreno:
 *    noise = m_noiseTerrain.GetNoise(worldX, worldZ)  // Valor [-1, 1]
 *    height = (noise + 1) * 0.5 * 60 + 50  // Valor [50, 110]
 *    Fórmula mapea [-1,1] -> [50,110]
 *
 * 3. Determinar bioma (superficie):
 *    biomeNoise = m_noiseBiome.GetNoise(worldX, worldZ)
 *    biomeBlock = getBiomeAt(worldX, worldZ, height)
 *
 * 4. Llenar columna verticalmente:
 *    Y=0: PIEDRA (bedrock)
 *    Y < height-4: PIEDRA (con cuevas si caveNoise > 0.4)
 *    Y < height: TIERRA
 *    Y = height: biomeBlock (PASTO, NIEVE, ARENA, etc.)
 *    Y = height + 1: Posible árbol (probabilidad 0.2 según bioma)
 *    Y > height: AIRE
 *
 * Generación de cuevas:
 * - Usa ruido 3D en (worldX, Y, worldZ)
 * - Si caveNoise > 0.4 y 10 < Y < height-5: crea cueva (AIRE)
 * - Las cuevas son más comunes a profundidad media
 */
void World::generateTerrain(Chunk* chunk) {
    ChunkPos pos = chunk->getPosition();

    // Calcular coordenadas mundiales del inicio del chunk
    int worldXStart = pos.x * BlockConfig::CHUNK_SIZE;
    int worldZStart = pos.z * BlockConfig::CHUNK_SIZE;

    // OPTIMIZACIÓN 1: Pre-calcular ruido 2D para todo el chunk
    float terrainNoise[BlockConfig::CHUNK_SIZE][BlockConfig::CHUNK_SIZE];
    float biomeNoise[BlockConfig::CHUNK_SIZE][BlockConfig::CHUNK_SIZE];

    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            int worldX = worldXStart + x;
            int worldZ = worldZStart + z;
            terrainNoise[x][z] = m_noiseTerrain.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
            biomeNoise[x][z] = m_noiseBiome.GetNoise(static_cast<float>(worldX), static_cast<float>(worldZ));
        }
    }

    // OPTIMIZACIÓN FASE 2: Pre-calcular alturas del terreno primero
    // Ajustado para WORLD_HEIGHT=32: rango [3, 28] en lugar de [50, 110]
    int terrainHeights[BlockConfig::CHUNK_SIZE][BlockConfig::CHUNK_SIZE];

    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            float noiseValue = terrainNoise[x][z];
            // Mapear [-1, 1] a [3, 28] para WORLD_HEIGHT=32
            terrainHeights[x][z] = static_cast<int>((noiseValue + 1.0f) * 0.5f * 25.0f) + 3;
        }
    }

    // OPTIMIZACIÓN FASE 2: Pre-calcular ruido 3D de cuevas para rangos necesarios
    // Array 1D PLANO en lugar de 3D anidado - MUCHO mejor cache locality
    // Encontrar altura máxima de terreno en el chunk
    int maxTerrainHeight = 3;  // Altura mínima del terreno (ajustado para WORLD_HEIGHT=32)
    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            maxTerrainHeight = std::max(maxTerrainHeight, terrainHeights[x][z]);
        }
    }

    // Rango de cuevas: Y=[2, maxTerrainHeight-2] (ajustado para WORLD_HEIGHT=32)
    const int caveYStart = 2;
    const int caveYEnd = maxTerrainHeight - 2;
    const int caveYRange = caveYEnd - caveYStart + 1;

    // OPTIMIZACIÓN FASE 2: Array 1D plano en lugar de vector<vector<vector<float>>>
    // Layout: [x * CHUNK_SIZE * caveYRange + z * caveYRange + (y - caveYStart)]
    // Beneficio: Mejor cache locality, menos allocations, acceso secuencial
    const size_t caveNoiseSize = BlockConfig::CHUNK_SIZE * BlockConfig::CHUNK_SIZE * caveYRange;
    std::vector<float> caveNoise(caveNoiseSize);

    // Pre-calcular noise de cuevas para todo el rango Y
    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            int worldX = worldXStart + x;
            int worldZ = worldZStart + z;

            for (int y = caveYStart; y <= caveYEnd; y++) {
                // Calcular índice plano: x * SIZE * YRANGE + z * YRANGE + y_offset
                size_t caveIndex = x * BlockConfig::CHUNK_SIZE * caveYRange + z * caveYRange + (y - caveYStart);
                caveNoise[caveIndex] = m_noiseCaves.GetNoise(
                    static_cast<float>(worldX),
                    static_cast<float>(y),
                    static_cast<float>(worldZ)
                );
            }
        }
    }

    // Generar terreno para cada columna x,z usando ruido cacheado
    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            int worldX = worldXStart + x;
            int worldZ = worldZStart + z;

            // Usar altura pre-calculada
            int terrainHeight = terrainHeights[x][z];

            // Obtener bioma usando valor cacheado
            BlockType biomeBlock = getBiomeFromNoise(biomeNoise[x][z]);

            // Llenar la columna verticalmente
            for (int y = 0; y < BlockConfig::WORLD_HEIGHT; y++) {
                BlockType blockType = BlockType::AIRE;

                if (y == 0) {
                    blockType = BlockType::PIEDRA;
                } else if (y < terrainHeight - 4) {
                    blockType = BlockType::PIEDRA;

                    // OPTIMIZACIÓN FASE 2: Usar noise de cuevas cacheado con índice plano
                    if (y > caveYStart && y < caveYEnd) {
                        size_t caveIndex = x * BlockConfig::CHUNK_SIZE * caveYRange + z * caveYRange + (y - caveYStart);
                        if (caveNoise[caveIndex] > 0.4f) {
                            blockType = BlockType::AIRE;
                        }
                    }
                } else if (y < terrainHeight) {
                    blockType = BlockType::TIERRA;
                } else if (y == terrainHeight) {
                    blockType = biomeBlock;
                }

                chunk->setBlockUnsafe(x, y, z, blockType);
            }

            // Generar árboles con probabilidad 0.1 (10%) - árboles frecuentes
            float treeChance = (m_rng() % 100) / 100.0f;  // Valor [0.0, 0.99]

            if (treeChance < 0.1f && terrainHeight + 1 < BlockConfig::WORLD_HEIGHT) {
                // Determinar tipo de árbol según bioma
                BlockType treeType = BlockType::AIRE;

                if (biomeBlock == BlockType::ARENA || biomeBlock == BlockType::TIERRA) {
                    // Biomas secos: árboles secos
                    treeType = BlockType::ARBOL_SECO;
                } else if (biomeBlock == BlockType::PASTO || biomeBlock == BlockType::PASTO_FULL) {
                    // Biomas de grass: árboles vivos
                    treeType = BlockType::ARBOL_GRASS;
                } else if (biomeBlock == BlockType::HIERBA_SANGRE) {
                    // Biomas de blood_grass: árboles de sangre
                    treeType = BlockType::ARBOL_SANGRE;
                }

                // Colocar el árbol un bloque encima de la superficie
                if (treeType != BlockType::AIRE) {
                    // Colocar el árbol visual en terrainHeight + 1
                    chunk->setBlockUnsafe(x, terrainHeight + 1, z, treeType);

                    // Actualizar heightmap para incluir el árbol
                    chunk->setMaxY(x, z, terrainHeight + 1);
                } else {
                    // Sin árbol, el heightmap es la superficie del terreno
                    chunk->setMaxY(x, z, terrainHeight);
                }
            } else {
                // No se generó árbol, el heightmap es la superficie del terreno
                chunk->setMaxY(x, z, terrainHeight);
            }
        }
    }
}

/**
 * @brief Determina el bioma en una posición dada
 * @return Tipo de bloque para la superficie
 *
 * Usa ruido de baja frecuencia para crear grandes regiones:
 *
 * biomeNoise [-1, 1]:
 * - < -0.3: NIEVE (15% del mundo) - bioma frío
 * - < 0.0: HIERBA_SANGRE (15%) - bioma extraño/corrupto
 * - < 0.4: PASTO (20%) - bioma normal/llanuras
 * - < 0.7: ARENA (15%) - bioma desierto
 * - >= 0.7: PASTO_FULL (35%) - bioma denso/bosque
 *
 * Los biomas se generan como "manchas" grandes debido a la baja
 * frecuencia del ruido (0.005), creando transiciones naturales.
 */
BlockType World::getBiomeAt(int x, int z, int height) const {
    float biomeValue = m_noiseBiome.GetNoise(static_cast<float>(x), static_cast<float>(z));
    return getBiomeFromNoise(biomeValue);
}

/**
 * @brief Determina el bioma desde valor de ruido pre-calculado
 * @param biomeValue Valor de ruido de bioma [-1, 1]
 * @return Tipo de bloque para la superficie
 *
 * Versión optimizada que usa el valor de ruido ya calculado y cache.
 */
BlockType World::getBiomeFromNoise(float biomeValue) const {
    // OPTIMIZACIÓN FASE 1: Cache ya está inicializado en constructor (no lazy init)
    // Usar cache en lugar de if-else chain - lookup O(1)
    // Mapear valor de ruido [-1, 1] a índice [0, 1023]
    int index = static_cast<int>((biomeValue + 1.0f) * 0.5f * BIOME_CACHE_SIZE);
    index = std::clamp(index, 0, BIOME_CACHE_SIZE - 1);
    return m_biomeCache[index];
}

/**
 * @brief Obtiene todos los chunks alrededor de una posición
 * @param center ChunkPos central
 * @param radius Radio en chunks (ej: 3 devuelve 7x7=49 chunks)
 * @return Vector de punteros a chunks generados
 *
 * Proceso:
 * 1. Iterar x desde center.x - radius hasta center.x + radius
 * 2. Iterar z desde center.z - radius hasta center.z + radius
 * 3. Para cada ChunkPos:
 *    - Si no existe, generar el chunk
 *    - Si está generado, agregar al vector
 * 4. Devolver vector (puede estar vacío si ningún chunk está generado)
 *
 * Se usa para obtener los chunks visibles desde la cámara/jugador.
 */
std::vector<Chunk*> World::getChunksAround(ChunkPos center, int radius) {
    // Pre-asignar capacidad esperada: (radius*2+1)²
    const int expectedSize = (radius * 2 + 1) * (radius * 2 + 1);
    std::vector<Chunk*> chunks;
    chunks.reserve(expectedSize);

    // Recopilar todas las posiciones que necesitamos
    std::vector<ChunkPos> positions;
    positions.reserve(expectedSize);

    for (int x = center.x - radius; x <= center.x + radius; x++) {
        for (int z = center.z - radius; z <= center.z + radius; z++) {
            positions.push_back(ChunkPos(x, z));
        }
    }

    // Copiar todos los punteros de chunks DENTRO del lock (evita use-after-free)
    std::vector<Chunk*> chunksToCheck;
    std::vector<ChunkPos> missingChunks;

    {
        std::lock_guard<std::mutex> lock(m_chunkQueueMutex);
        chunksToCheck.reserve(expectedSize);
        missingChunks.reserve(expectedSize);

        for (const ChunkPos& pos : positions) {
            auto it = m_chunks.find(pos);
            if (it != m_chunks.end()) {
                chunksToCheck.push_back(it->second.get());
            } else {
                missingChunks.push_back(pos);
            }
        }
    }

    // FUERA del lock: procesar los chunks y solicitar los faltantes
    for (Chunk* chunk : chunksToCheck) {
        if (chunk && chunk->isGenerated()) {
            chunks.push_back(chunk);
        }
    }

    // Solicitar generación de chunks faltantes
    for (const ChunkPos& pos : missingChunks) {
        requestChunkGeneration(pos);
    }

    return chunks;
}

/**
 * @brief Descarga chunks lejanos para liberar memoria
 * @param center Posición central (generalmente el jugador)
 * @param maxDistance Distancia máxima para mantener chunks
 * @return Cantidad de chunks descargados
 *
 * Algoritmo:
 * 1. Para cada chunk en el mapa:
 *    - Calcular distancia Manhattan: dist = |chunkX - centerX| + |chunkZ - centerZ|
 *    - Si distX > maxDistance O distZ > maxDistance: marcar para descarga
 * 2. Eliminar todos los chunks marcados
 * 3. unique_ptr libera la memoria automáticamente
 *
 * Nota: Se usa distancia Manhattan (|dx| + |dz|) en lugar de euclidiana (sqrt(dx² + dz²))
 * porque es más rápida (no hay sqrt) y suficiente para este propósito.
 *
 * Ejemplo con maxDistance=9:
 * - Si el jugador está en (0,0), el chunk (10,0) se descarga
 * - El chunk (9,0) se mantiene
 * - El chunk (5,5) se mantiene (distancia Manhattan = 10, pero distX=5, distZ=5)
 */
int World::unloadChunksFarFrom(ChunkPos center, int maxDistance) {
    std::vector<ChunkPos> chunksToUnload;

    // Encontrar todos los chunks que están fuera del rango permitido
    for (const auto& pair : m_chunks) {
        ChunkPos pos = pair.first;

        // Calcular distancia Manhattan desde el centro
        int distX = abs(pos.x - center.x);
        int distZ = abs(pos.z - center.z);

        // Si el chunk está muy lejos en cualquiera de los ejes, marcarlo para descarga
        if (distX > maxDistance || distZ > maxDistance) {
            chunksToUnload.push_back(pos);
        }
    }

    // OPTIMIZACIÓN: Object Pooling - Devolver chunks al pool en lugar de destruir
    for (ChunkPos pos : chunksToUnload) {
        auto it = m_chunks.find(pos);
        if (it != m_chunks.end()) {
            // Mover chunk al pool para reutilización
            m_chunkPool.push_back(std::move(it->second));

            // Eliminar del mapa activo (el unique_ptr ahora es nullptr)
            m_chunks.erase(it);
        }
    }

    return static_cast<int>(chunksToUnload.size());
}

/**
 * @brief Obtiene la altura del terreno en una posición mundial
 * @return Altura Y del bloque sólido más alto
 *
 * OPTIMIZACIÓN FASE 2: Ajustado para WORLD_HEIGHT=32
 * Usa la misma fórmula que generateTerrain():
 * height = (noise + 1) * 0.5 * 25 + 3  (Valor [3, 28])
 * Fórmula mapea [-1,1] -> [3,28]
 *
 * Útil para spawnear entities en la superficie.
 * Nota: No considera cuevas, solo la superficie del terreno.
 */
int World::getTerrainHeight(int x, int z) const {
    float noiseValue = m_noiseTerrain.GetNoise(static_cast<float>(x), static_cast<float>(z));
    // OPTIMIZACIÓN FASE 2: Ajustado para WORLD_HEIGHT=32
    return static_cast<int>((noiseValue + 1.0f) * 0.5f * 25.0f) + 3;
}
