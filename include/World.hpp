/**
 * @file World.hpp
 * @brief Define la clase World para generación procedural infinita
 *
 * Este archivo contiene la definición del mundo procedural infinito,
 * incluyendo generación de terreno con ruido Perlin, sistema de biomas,
 * y gestión dinámica de chunks.
 */

#pragma once

#include "Chunk.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <random>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

#include "FastNoiseLite/FastNoiseLite.h"

/**
 * @class FastNoiseLiteWrapper
 * @brief Wrapper C++ para la API estilo C de FastNoiseLite
 *
 * FastNoiseLite es una biblioteca de C para generación de ruido.
 * Este wrapper encapsula la API en una clase C++ con métodos más convenientes.
 *
 * Usos:
 * - Terreno: Ruido Perlin con fractales FBM
 * - Cuevas: Ruido OpenSimplex2 en 3D
 * - Biomas: Ruido Perlin de baja frecuencia
 */
class FastNoiseLiteWrapper {
public:
    fnl_state state; ///< Estado interno de FastNoiseLite (estructura C)

    /**
     * @brief Constructor - inicializa el estado con valores por defecto
     */
    FastNoiseLiteWrapper() {
        state = fnlCreateState();
    }

    /** @brief Establece el tipo de ruido (Perlin, OpenSimplex2, etc.) */
    void SetNoiseType(fnl_noise_type type) { state.noise_type = type; }

    /** @brief Establece la semilla del generador de ruido */
    void SetSeed(int seed) { state.seed = seed; }

    /** @brief Establece la frecuencia del ruido (menor = terreno más suave) */
    void SetFrequency(float freq) { state.frequency = freq; }

    /** @brief Establece el número de octavas para ruido fractal */
    void SetFractalOctaves(int octaves) { state.octaves = octaves; }

    /** @brief Establece la lacunaridad (cuánto cambia la frecuencia entre octavas) */
    void SetFractalLacunarity(float lac) { state.lacunarity = lac; }

    /** @brief Establece la ganancia (amplitud de cada octava adicional) */
    void SetFractalGain(float gain) { state.gain = gain; }

    /** @brief Establece el tipo de fractal (FBM, Billow, etc.) */
    void SetFractalType(fnl_fractal_type type) { state.fractal_type = type; }

    /** @brief Genera ruido 2D para terreno y biomas */
    float GetNoise(float x, float y) const {
        return fnlGetNoise2D(&state, x, y);
    }

    /** @brief Genera ruido 3D para cuevas y estructuras subterráneas */
    float GetNoise(float x, float y, float z) const {
        return fnlGetNoise3D(&state, x, y, z);
    }
};

/**
 * @brief Especialización de hash para ChunkPos (permite usar como clave en unordered_map)
 *
 * El hash combina los valores X y Z usando XOR y desplazamiento de bits.
 * - hash(x) ^ (hash(z) << 1) minimiza colisiones
 * - XOR es rápido y produce buena distribución
 * - << 1 dispersa los bits de Z para evitar patrones
 */
namespace std {
    template<>
    struct hash<ChunkPos> {
        size_t operator()(const ChunkPos& pos) const {
            return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.z) << 1);
        }
    };
}

/**
 * @class World
 * @brief Mundo procedural infinito con generación de terreno
 *
 * El World gestiona un conjunto infinito de chunks que se generan proceduralmente
 * usando ruido Perlin. Características principales:
 *
 * Generación procedural:
 * - Terreno variable con colinas y valles
 * - Biomas diferentes (nieve, desierto, pasto, etc.)
 * - Cuevas subterráneas con ruido 3D
 * - Altura del mundo: 256 bloques
 *
 * Gestión de chunks:
 * - Carga dinámica: se generan bajo demanda
 * - Descarga automática: se liberan chunks lejanos
 * - Almacenamiento en unordered_map para acceso O(1)
 *
 * Rendimiento:
 * - LOAD_RADIUS: chunks alrededor del jugador (9x9 = 81)
 * - UNLOAD_DISTANCE: distancia para liberar chunks
 * - Memoria típica: ~1.3 MB para 81 chunks
 */
class World {
public:
    /**
     * @brief Constructor del mundo
     * @param seed Semilla para generación procedural (0 = aleatoria)
     *
     * Inicializa los generadores de ruido y prepara el mundo vacío.
     * Inicia el thread de background para generación de chunks.
     */
    World(uint32_t seed = 0);

    /**
     * @brief Destructor - detiene el thread de background
     *
     * Libera recursos y espera a que el thread de generación termine.
     */
    ~World();

    /**
     * @brief Obtiene un bloque en coordenadas mundiales
     * @param x Coordenada X mundial (puede ser negativa)
     * @param y Coordenada Y (altura) [0, 255]
     * @param z Coordenada Z mundial (puede ser negativa)
     * @return Referencia al bloque (modificable)
     *
     * Convierte coordenadas mundiales a locales del chunk automáticamente.
     * Si el chunk no existe, devuelve un bloque inválido (AIRE).
     */
    Block& getBlock(int x, int y, int z);

    /** @brief Versión const de getBlock() */
    const Block& getBlock(int x, int y, int z) const;

    /**
     * @brief Establece un bloque en coordenadas mundiales
     * @param x, y, z Coordenadas mundiales
     * @param type Tipo de bloque a colocar
     *
     * Útil para modificación del terreno por el jugador.
     * Si el chunk no existe, la operación se ignora.
     */
    void setBlock(int x, int y, int z, BlockType type);

    /**
     * @brief Obtiene un chunk (si existe)
     * @param pos Posición del chunk
     * @return Puntero al chunk o nullptr si no existe
     *
     * NO genera el chunk si no existe. Para obtener/generar, usar getChunksAround().
     */
    inline Chunk* getChunk(ChunkPos pos) {
        auto it = m_chunks.find(pos);
        return it != m_chunks.end() ? it->second.get() : nullptr;
    }

    /** @brief Versión const de getChunk() */
    inline const Chunk* getChunk(ChunkPos pos) const {
        auto it = m_chunks.find(pos);
        return it != m_chunks.end() ? it->second.get() : nullptr;
    }

    /**
     * @brief Genera un chunk proceduralmente
     * @param pos Posición del chunk a generar
     *
     * Si el chunk ya existe, no hace nada.
     * Proceso de generación:
     * 1. Crea chunk vacío
     * 2. Genera terreno con ruido Perlin
     * 3. Aplica biomas según ruido de baja frecuencia
     * 4. Genera cuevas con ruido 3D
     * 5. Marca como generado y almacena
     */
    void generateChunk(ChunkPos pos);

    /**
     * @brief Obtiene todos los chunks alrededor de una posición
     * @param center ChunkPos central
     * @param radius Radio en chunks (ej: 3 = 7x7 = 49 chunks)
     * @return Vector con punteros a los chunks
     *
     * Genera chunks que no existen y devuelve solo los que están generados.
     * Se usa para obtener los chunks visibles desde una posición.
     */
    std::vector<Chunk*> getChunksAround(ChunkPos center, int radius);

    /**
     * @brief Descarga chunks lejanos para liberar memoria
     * @param center Posición central (jugador)
     * @param maxDistance Distancia máxima para mantener chunks
     * @return Cantidad de chunks descargados
     *
     * Usa distancia Manhattan (más rápido que euclidiana).
     * Libera chunks que están más lejos de maxDistance en cualquier eje.
     * unique_ptr libera la memoria automáticamente.
     */
    int unloadChunksFarFrom(ChunkPos center, int maxDistance);

    /**
     * @brief Obtiene cantidad de chunks cargados
     * @return Número de chunks en memoria
     */
    size_t getChunkCount() const { return m_chunks.size(); }

    /**
     * @brief Obtiene la altura del terreno en una posición
     * @param x Coordenada X mundial
     * @param z Coordenada Z mundial
     * @return Altura Y del bloque sólido más alto
     *
     * Útil para spawnear entities en la superficie.
     */
    int getTerrainHeight(int x, int z) const;

    /** @brief Obtiene la semilla del mundo */
    inline uint32_t getSeed() const { return m_seed; }

private:
    uint32_t m_seed;                                    ///< Semilla del mundo (determinista)
    std::mt19937 m_rng;                                 ///< Generador Mersenne Twister para aleatorios
    FastNoiseLiteWrapper m_noiseTerrain;                ///< Ruido para terreno (Perlin + FBM)
    FastNoiseLiteWrapper m_noiseCaves;                  ///< Ruido para cuevas (OpenSimplex2 3D)
    FastNoiseLiteWrapper m_noiseBiome;                  ///< Ruido para biomas (Perlin baja frecuencia)

    // OPTIMIZACIÓN: Cache de lookup para biomas (3-5% FPS)
    static constexpr int BIOME_CACHE_SIZE = 1024;
    mutable BlockType m_biomeCache[BIOME_CACHE_SIZE];   ///< Cache de biomas por valor de ruido
    mutable bool m_biomeCacheInitialized = false;       ///< Flag de inicialización

    // OPTIMIZACIÓN 6: Multithreaded Chunk Generation
    std::thread m_chunkGeneratorThread;                 ///< Thread de background para generación
    std::queue<ChunkPos> m_chunkGenerationQueue;        ///< Cola de chunks pendientes de generación
    std::mutex m_chunkQueueMutex;                       ///< Mutex para proteger la cola
    std::condition_variable m_chunkQueueCV;             ///< Variable de condición para wakeups
    std::atomic<bool> m_chunkGeneratorShouldStop;       ///< Flag para detener el thread

    /**
     * @brief Worker thread para generación de chunks en background
     *
     * Este método corre en un thread separado y:
     * 1. Espera a que haya chunks en la cola
     * 2. Genera chunks de la cola
     * 3. Los chunks generados se agregan a m_chunks
     *
     * Se detiene cuando m_chunkGeneratorShouldStop es true.
     */
    void chunkGenerationWorker();

    /**
     * @brief Solicita generación asíncrona de un chunk
     * @param pos Posición del chunk a generar
     *
     * Agrega el chunk a la cola de generación si no existe.
     * El thread de background lo generará eventualmente.
     */
    void requestChunkGeneration(ChunkPos pos);

    /**
     * @brief Mapa de chunks cargados
     * Key: ChunkPos, Value: unique_ptr<Chunk>
     * unordered_map da O(1) búsqueda/insertación/eliminación
     */
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> m_chunks;

    /**
     * @brief Object Pool de chunks inactivos para reutilización
     *
     * En lugar de crear/destruir chunks constantemente (allocations del heap),
     * los chunks descargados se devuelven a este pool para ser reutilizados.
     *
     * Beneficios:
     * - Elimina allocations/deallocations frecuentes
     * - Mejor localidad de cache
     * - 5-10% mejora general de rendimiento
     */
    std::vector<std::unique_ptr<Chunk>> m_chunkPool;

    /**
     * @brief Inicializa los generadores de ruido con la semilla
     *
     * Configura 3 generadores con parámetros diferentes:
     * - Terreno: Perlin, freq=0.01, 4 octavas, FBM
     * - Cuevas: OpenSimplex2, freq=0.05
     * - Biomas: Perlin, freq=0.002 (muy baja frecuencia = biomas muy grandes)
     */
    void initNoiseGenerators();

    /**
     * @brief Genera terreno para un chunk específico
     * @param chunk Chunk a rellenar (debe existir)
     *
     * Proceso:
     * 1. Para cada columna (x,z) del chunk:
     *    a. Calcular altura del terreno con ruido 2D
     *    b. Determinar bioma con ruido separado
     *    c. Llenar columna desde Y=0 hasta altura:
     *       - Y=0: bedrock (piedra)
     *       - Y < altura-4: piedra (con cuevas)
     *       - Y < altura: tierra
     *       - Y = altura: bioma (pasto, nieve, arena)
     *       - Y > altura: aire
     */
    BlockType getBiomeAt(int x, int z, int height) const;

private:
    /**
     * @brief Determina el bioma desde valor de ruido pre-calculado
     * @param biomeValue Valor de ruido de bioma [-1, 1]
     * @return Tipo de bloque para la superficie
     *
     * Versión optimizada interna que usa el valor de ruido ya calculado.
     */
    BlockType getBiomeFromNoise(float biomeValue) const;

    /**
     * @brief Genera el terreno para un chunk completo
     * @param chunk Chunk a generar (debe estar vacío)
     *
     * Implementa el algoritmo de generación procedural:
     * - Usa ruido Perlin 2D para altura del terreno
     * - Usa ruido Perlin 3D para cuevas subterráneas
     * - Selecciona bioma según ruido de bioma
     *
     * Estructura vertical de la columna (x, z):
     * - Y = 0: bedrock (indestructible)
     * - Y < altura-4: piedra (con cuevas)
     * - Y < altura: tierra
     * - Y = altura: bioma (pasto, nieve, arena)
     * - Y > altura: aire
     */
    void generateTerrain(Chunk* chunk);
};

