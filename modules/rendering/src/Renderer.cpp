/**
 * @file Renderer.cpp
 * @brief Implementación del sistema de renderizado isométrico
 *
 * Implementa TextureManager y Renderer usando SDL2 y stb_image.
 */

#include "rendering/Renderer.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <array>

// ============================================================================
// TextureManager Implementation
// ============================================================================

/**
 * @brief Constructor del gestor de texturas
 * @param renderer Renderer SDL2 para crear las texturas
 */
TextureManager::TextureManager(SDL_Renderer* renderer)
    : m_renderer(renderer)
{
}

/**
 * @brief Destructor - libera todas las texturas cargadas
 *
 * Itera sobre el mapa y destruye cada textura con SDL_DestroyTexture().
 * También libera el sprite sheet y el sprite sheet de árboles si están cargados.
 */
TextureManager::~TextureManager() {
    // Liberar sprite sheet
    if (m_spriteSheet) {
        SDL_DestroyTexture(m_spriteSheet);
        m_spriteSheet = nullptr;
    }

    // Liberar sprite sheet de árboles
    if (m_treeSpriteSheet) {
        SDL_DestroyTexture(m_treeSpriteSheet);
        m_treeSpriteSheet = nullptr;
    }

    // Liberar todas las texturas del mapa
    for (auto& pair : m_textures) {
        if (pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    m_textures.clear();
}

/**
 * @brief Carga una textura desde archivo PNG
 * @param name Nombre identificador para la textura
 * @param path Ruta al archivo PNG
 * @return true si se cargó correctamente
 *
 * Proceso:
 * 1. stbi_load(): Carga PNG del disco (RGBA, 4 canales)
 * 2. SDL_CreateRGBSurfaceFrom(): Crea superficie desde los datos
 * 3. SDL_CreateTextureFromSurface(): Crea textura GPU desde la superficie
 * 4. SDL_FreeSurface() + stbi_image_free(): Libera memoria temporal
 * 5. Almacena en m_textures[name]
 */
bool TextureManager::loadTexture(const std::string& name, const std::string& path) {
    // Cargar imagen con stb_image (canales RGBA)
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        std::cerr << "Error al cargar textura '" << name << "' desde " << path << ": " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Crear superficie SDL desde los datos de imagen
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        data,
        width,
        height,
        32,  // bits per pixel (RGBA)
        width * 4,  // pitch (bytes por fila)
        0x000000FF,  // R mask
        0x0000FF00,  // G mask
        0x00FF0000,  // B mask
        0xFF000000   // A mask
    );

    if (!surface) {
        std::cerr << "Error al crear superficie para '" << name << "': " << SDL_GetError() << std::endl;
        stbi_image_free(data);
        return false;
    }

    // Crear textura SDL desde la superficie (memoria GPU)
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);
    stbi_image_free(data);

    if (!texture) {
        std::cerr << "Error al crear textura '" << name << "': " << SDL_GetError() << std::endl;
        return false;
    }

    // Cache textura con dimensiones
    TextureInfo info;
    info.texture = texture;
    info.width = width;
    info.height = height;
    info.scaledWidth = width;   // Inicialmente sin zoom (zoom=1.0)
    info.scaledHeight = height;
    m_textures[name] = info;
    return true;
}

/**
 * @brief Obtiene una textura por nombre
 * @param name Nombre identificador
 * @return Puntero a la textura o nullptr si no existe
 *
 * Si la textura no existe, imprime advertencia (una sola vez)
 * y devuelve nullptr.
 */
SDL_Texture* TextureManager::getTexture(const std::string& name) const {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second.texture;
    }

    // Advertencia solo una vez por textura faltante
    static std::string lastWarning;
    if (lastWarning != name) {
        std::cerr << "Advertencia: Textura '" << name << "' no encontrada" << std::endl;
        lastWarning = name;
    }

    return nullptr;
}

/**
 * @brief Carga todas las texturas del juego
 * @return true si todas se cargaron correctamente
 *
 * Ahora carga el sprite sheet que contiene todos los tiles:
 * - cubos_tiles_Sheet.png (9 tiles en una fila, cada uno de 32px)
 * - sprite_sheet_tres.png (60 árboles en 5 filas x 12 columnas, cada uno de 64px)
 * - player.png (textura separada del jugador)
 *
 * Los tiles del sprite sheet se mapean por índice (0-8) a BlockType:
 * 0=grass(PASTO), 1=blood_grass(HIERBA_SANGRE), 2=sand(ARENA),
 * 3=black_dirt(PIEDRA), 4=dirt(TIERRA), 5=dirt_alt(DIRT_ALT),
 * 6=full_blood_grass(PASTO_FULL), 7=snow(NIEVE), 8=water(AGUA)
 */
bool TextureManager::loadAllTextures() {
    bool success = true;

    // Cargar el sprite sheet que contiene todos los tiles
    success &= loadTexture("sprite_sheet", "assets/tiles/cubos_tiles_Sheet.png");

    // Guardar puntero al sprite sheet para acceso rápido
    if (success) {
        m_spriteSheet = m_textures["sprite_sheet"].texture;
    }

    // Cargar el sprite sheet de árboles
    success &= loadTexture("tree_sprite_sheet", "assets/tiles/sprite_sheet_tres.png");

    // Guardar puntero al sprite sheet de árboles para acceso rápido
    if (m_textures.find("tree_sprite_sheet") != m_textures.end()) {
        m_treeSpriteSheet = m_textures["tree_sprite_sheet"].texture;
    }

    // Cargar textura del jugador (separada del sprite sheet)
    success &= loadTexture("player", "assets/tiles/player.png");

    // Configurar TextureInfo para cada BlockType apuntando al sprite sheet
    // IMPORTANTE: Sobrescribir las dimensiones para que sean de un tile individual (32x32)
    // en lugar de las dimensiones del sprite sheet completo (288x32)
    auto& spriteSheetInfo = m_textures["sprite_sheet"];
    spriteSheetInfo.width = SPRITE_TILE_SIZE;  // 32px en lugar de 288px
    spriteSheetInfo.height = SPRITE_TILE_SIZE; // 32px (ya era correcto)
    spriteSheetInfo.scaledWidth = SPRITE_TILE_SIZE;
    spriteSheetInfo.scaledHeight = SPRITE_TILE_SIZE;

    // Configurar TextureInfo para los árboles (64x64)
    TextureInfo* treeSpriteSheetInfoPtr = nullptr;
    if (m_textures.find("tree_sprite_sheet") != m_textures.end()) {
        auto& treeSpriteSheetInfo = m_textures["tree_sprite_sheet"];
        treeSpriteSheetInfo.width = TREE_SPRITE_SIZE;  // 64px
        treeSpriteSheetInfo.height = TREE_SPRITE_SIZE; // 64px
        treeSpriteSheetInfo.scaledWidth = TREE_SPRITE_SIZE;
        treeSpriteSheetInfo.scaledHeight = TREE_SPRITE_SIZE;
        treeSpriteSheetInfoPtr = &treeSpriteSheetInfo;

        // OPTIMIZACIÓN FASE 2: Pre-calcular sprite rects de árboles
        initializeTreeSpriteCache();
    }

    // Todos los bloques de terreno comparten el mismo TextureInfo del sprite sheet
    m_blockTextures[static_cast<int>(BlockType::PASTO)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::HIERBA_SANGRE)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::ARENA)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::PIEDRA)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::TIERRA)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::DIRT_ALT)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::PASTO_FULL)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::NIEVE)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::AGUA)] = &spriteSheetInfo;
    m_blockTextures[static_cast<int>(BlockType::AIRE)] = nullptr;  // No se renderiza

    // Los árboles usan el sprite sheet de árboles
    if (treeSpriteSheetInfoPtr) {
        m_blockTextures[static_cast<int>(BlockType::ARBOL_SECO)] = treeSpriteSheetInfoPtr;
        m_blockTextures[static_cast<int>(BlockType::ARBOL_GRASS)] = treeSpriteSheetInfoPtr;
        m_blockTextures[static_cast<int>(BlockType::ARBOL_SANGRE)] = treeSpriteSheetInfoPtr;
    }

    return success;
}

void TextureManager::updateScaledDimensions(float zoom) {
    // Dirty flag: solo recalcular si el zoom cambió
    if (zoom == m_lastZoom) {
        return;
    }

    m_lastZoom = zoom;

    // Recalcular dimensiones escaladas para todas las texturas
    for (auto& pair : m_textures) {
        TextureInfo& info = pair.second;
        if (info.texture) {
            info.scaledWidth = static_cast<int>(info.width * zoom);
            info.scaledHeight = static_cast<int>(info.height * zoom);
        }
    }
}

TextureManager::TextureInfo* TextureManager::getBlockTexture(BlockType type) const {
    int index = static_cast<int>(type);
    if (index >= 0 && index < 13) {  // 13 tipos de bloques (0-12)
        return m_blockTextures[index];
    }
    return nullptr;
}

/**
 * @brief Obtiene el rectángulo fuente para un tipo de bloque del sprite sheet
 * @param type Tipo de bloque
 * @return SDL_Rect con el área del sprite sheet correspondiente al bloque
 *
 * Mapea BlockType a índices del sprite sheet (0-8):
 * PASTO=0, HIERBA_SANGRE=1, ARENA=2, PIEDRA=3, TIERRA=4, DIRT_ALT=5,
 * PASTO_FULL=6, NIEVE=7, AGUA=8
 */
SDL_Rect TextureManager::getSpriteSheetRect(BlockType type) const {
    SDL_Rect rect;
    rect.w = SPRITE_TILE_SIZE;
    rect.h = SPRITE_TILE_SIZE;

    // Los BlockType están ordenados para coincidir con el sprite sheet
    // AIRE = 0, PASTO = 1, ..., AGUA = 9
    // Necesitamos restar 1 para obtener el índice del sprite sheet (0-8)
    int spriteIndex = static_cast<int>(type) - 1;

    if (spriteIndex >= 0 && spriteIndex < 9) {
        rect.x = spriteIndex * SPRITE_TILE_SIZE;
        rect.y = 0;
    } else {
        // Para AIRE o tipos inválidos, retornar rectángulo vacío
        rect.x = 0;
        rect.y = 0;
        rect.w = 0;
        rect.h = 0;
    }

    return rect;
}

/**
 * @brief Obtiene el rectángulo fuente para un árbol del sprite sheet de árboles
 * @param type Tipo de árbol (ARBOL_SECO, ARBOL_GRASS, ARBOL_SANGRE)
 * @param worldX Coordenada X mundial (para selección aleatoria)
 * @param worldZ Coordenada Z mundial (para selección aleatoria)
 * @return SDL_Rect con el área del sprite sheet correspondiente al árbol
 *
 * OPTIMIZACIÓN FASE 2: Ahora usa cache pre-calculado en lugar de calcular
 * hash, módulo y división en cada frame. Solo calcula el índice del árbol
 * y hace lookup O(1) en el array cacheado.
 *
 * Los árboles se distribuyen en:
 * - Fila 1 (0-11): Árboles secos para biomas secos (ARBOL_SECO)
 * - Filas 2-3 (12-35): Árboles vivos para biomas grass (ARBOL_GRASS)
 * - Filas 4-5 (36-59): Árboles de sangre para biomas blood_grass (ARBOL_SANGRE)
 */
SDL_Rect TextureManager::getTreeSpriteRect(BlockType type, int worldX, int worldZ) const {
    // Si el cache no está inicializado, fallback al método original
    if (!m_treeSpriteCache.initialized) {
        SDL_Rect rect;
        rect.w = TREE_SPRITE_SIZE;
        rect.h = TREE_SPRITE_SIZE;

        // Determinar el rango de índices según el tipo de árbol
        int startIndex, endIndex;

        switch (type) {
            case BlockType::ARBOL_SECO:
                startIndex = 0; endIndex = 11; break;
            case BlockType::ARBOL_GRASS:
                startIndex = 12; endIndex = 35; break;
            case BlockType::ARBOL_SANGRE:
                startIndex = 36; endIndex = 59; break;
            default:
                rect.x = 0; rect.y = 0; rect.w = 0; rect.h = 0;
                return rect;
        }

        // Hash consistente para selección aleatoria
        int treeCount = endIndex - startIndex + 1;
        int hash = (worldX * 374761393 + worldZ * 668265263) % 1000000007;
        int treeOffset = abs(hash) % treeCount;
        int treeIndex = startIndex + treeOffset;

        int column = treeIndex % TREE_SPRITE_COLUMNS;
        int row = treeIndex / TREE_SPRITE_COLUMNS;
        rect.x = column * TREE_SPRITE_SIZE;
        rect.y = row * TREE_SPRITE_SIZE;
        return rect;
    }

    // OPTIMIZACIÓN: Usar cache pre-calculado (O(1) lookup)
    int treeTypeIndex;
    int startIndex, endIndex;

    // Determinar tipo y rango de índices
    switch (type) {
        case BlockType::ARBOL_SECO:
            treeTypeIndex = 0;
            startIndex = 0; endIndex = 11;  // 12 árboles
            break;
        case BlockType::ARBOL_GRASS:
            treeTypeIndex = 1;
            startIndex = 12; endIndex = 35;  // 24 árboles
            break;
        case BlockType::ARBOL_SANGRE:
            treeTypeIndex = 2;
            startIndex = 36; endIndex = 59;  // 24 árboles
            break;
        default:
            SDL_Rect empty{0, 0, 0, 0};
            return empty;
    }

    // Calcular índice específico del árbol (hash consistente)
    int treeCount = endIndex - startIndex + 1;
    int hash = (worldX * 374761393 + worldZ * 668265263) % 1000000007;
    int treeOffset = abs(hash) % treeCount;
    int treeIndex = startIndex + treeOffset;

    // Lookup directo en cache (SIN calcular columna/fila)
    return m_treeSpriteCache.rects[treeTypeIndex][treeIndex];
}

/**
 * @brief Inicializa el cache de sprite rects de árboles
 *
 * Pre-calcula todos los rects posibles para cada tipo de árbol basado en spriteIndex.
 * Esto permite lookup O(1) en renderWorld() en lugar de calcular hash, módulo y división.
 */
void TextureManager::initializeTreeSpriteCache() {
    // Pre-calcular rects para cada tipo de árbol y cada sprite index (0-59)
    for (int spriteIndex = 0; spriteIndex < TREE_SPRITE_COUNT; spriteIndex++) {
        // Calcular posición en el sprite sheet
        int column = spriteIndex % TREE_SPRITE_COLUMNS;
        int row = spriteIndex / TREE_SPRITE_COLUMNS;

        SDL_Rect rect;
        rect.x = column * TREE_SPRITE_SIZE;
        rect.y = row * TREE_SPRITE_SIZE;
        rect.w = TREE_SPRITE_SIZE;
        rect.h = TREE_SPRITE_SIZE;

        // Almacenar el mismo rect para los 3 tipos de árbol
        // La selección del tipo específico (ARBOL_SECO, ARBOL_GRASS, ARBOL_SANGRE)
        // se basa en el rango de índices, no en el rect en sí
        m_treeSpriteCache.rects[0][spriteIndex] = rect;  // ARBOL_SECO
        m_treeSpriteCache.rects[1][spriteIndex] = rect;  // ARBOL_GRASS
        m_treeSpriteCache.rects[2][spriteIndex] = rect;  // ARBOL_SANGRE
    }

    m_treeSpriteCache.initialized = true;
}

// ============================================================================
// Renderer Implementation
// ============================================================================

/**
 * @brief Constructor del renderer
 * @param window Ventana SDL2
 *
 * Crea renderer SDL2 con aceleración hardware y VSync.
 * Inicializa TextureManager y carga todas las texturas.
 */
Renderer::Renderer(SDL_Window* window)
    : m_window(window)
    , m_renderer(nullptr)
    , m_textureManager(nullptr)
    , m_clearR(135)
    , m_clearG(206)
    , m_clearB(235)  // Sky blue
{
    // Pre-asignar cache de tiles (capacidad para ~200k tiles)
    m_tileCache.reserve(200000);

    // Crear renderer con aceleración hardware y VSync
    m_renderer = SDL_CreateRenderer(
        m_window,
        -1,  // Driver por defecto
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "Error al crear renderer: " << SDL_GetError() << std::endl;
        return;
    }

    // Inicializar texture manager
    m_textureManager = TextureManager(m_renderer);

    // Cargar todas las texturas
    if (!m_textureManager.loadAllTextures()) {
        std::cerr << "Advertencia: Algunas texturas no pudieron cargarse" << std::endl;
    }
}

/**
 * @brief Destructor - libera renderer SDL2
 */
Renderer::~Renderer() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
    }
}

/**
 * @brief Limpia la pantalla con el color de fondo
 */
void Renderer::clear() {
    SDL_SetRenderDrawColor(m_renderer, m_clearR, m_clearG, m_clearB, 255);
    SDL_RenderClear(m_renderer);
}

/**
 * @brief Presenta el renderizado (flip buffer)
 */
void Renderer::present() {
    SDL_RenderPresent(m_renderer);
}

/**
 * @brief Renderiza un bloque individual en pantalla
 * @param screenX, screenY Posición en pantalla
 * @param type Tipo de bloque (selecciona textura)
 * @param worldY Altura en el mundo (no usado directamente)
 *
 * Si type == AIRE, no renderiza.
 * Obtiene textura y la dibuja centrada en la posición.
 */
void Renderer::renderBlock(float screenX, float screenY, BlockType type, int worldY, float zoom) {
    // No renderizar aire
    if (type == BlockType::AIRE) {
        return;
    }

    // OPTIMIZACIÓN FASE 1: Usar TextureInfo cacheado en lugar de SDL_QueryTexture
    TextureManager::TextureInfo* texInfo = m_textureManager.getBlockTexture(type);
    if (!texInfo || !texInfo->texture) {
        return;  // Textura no válida
    }

    // Usar dimensiones cacheadas (ya escaladas por zoom si se llamó updateScaledDimensions)
    // Si no se ha actualizado el zoom, calcular al vuelo
    int texWidth = texInfo->scaledWidth;
    int texHeight = texInfo->scaledHeight;

    // Si el zoom es diferente al cacheado en TextureManager, recalcular
    if (texWidth == static_cast<int>(texInfo->width)) {
        // No se ha aplicado zoom, aplicarlo ahora
        texWidth = static_cast<int>(texInfo->width * zoom);
        texHeight = static_cast<int>(texInfo->height * zoom);
    }

    // Crear rectángulo de destino (centrado en screenX, screenY)
    SDL_Rect destRect;
    destRect.x = static_cast<int>(std::round(screenX - texWidth / 2.0f));
    destRect.y = static_cast<int>(std::round(screenY - texHeight));
    destRect.w = texWidth;
    destRect.h = texHeight;

    // Obtener el rectángulo fuente (sprite sheet)
    SDL_Rect srcRect = m_textureManager.getSpriteSheetRect(type);
    if (srcRect.w == 0 || srcRect.h == 0) {
        return;  // Sprite inválido
    }

    // Renderizar textura
    SDL_RenderCopy(m_renderer, texInfo->texture, &srcRect, &destRect);
}

/**
 * @brief Renderiza un chunk completo
 * @param chunk Chunk a renderizar
 * @param camera Cámara para proyección isométrica
 *
 * Crea lista de tiles, ordena por profundidad y renderiza.
 */
void Renderer::renderChunk(const Chunk* chunk, const Camera& camera) {
    if (!chunk) {
        return;
    }

    // Crear lista de tiles a renderizar
    std::vector<RenderTile> renderList = createRenderList(chunk, camera);

    // Ordenar por profundidad (back-to-front)
    sortTilesByDepth(renderList);

    // Renderizar cada tile usando renderBlock (tiene el switch correcto)
    float zoom = camera.getZoom();
    for (const auto& tile : renderList) {
        renderBlock(tile.x, tile.y, tile.type, tile.worldY, zoom);
    }
}

/**
 * @brief Crea lista de tiles para renderizar desde un chunk
 * @return Vector de RenderTile con coordenadas de pantalla
 *
 * Itera sobre todos los bloques del chunk, convierte los sólidos
 * a RenderTile usando worldToScreen() de la cámara.
 */
std::vector<RenderTile> Renderer::createRenderList(const Chunk* chunk, const Camera& camera) {
    std::vector<RenderTile> tiles;

    ChunkPos chunkPos = chunk->getPosition();
    int worldXStart = chunkPos.x * BlockConfig::CHUNK_SIZE;
    int worldZStart = chunkPos.z * BlockConfig::CHUNK_SIZE;

    // Recorrer todos los bloques del chunk
    for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
        for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
            for (int y = 0; y < BlockConfig::WORLD_HEIGHT; y++) {
                const Block& block = chunk->getBlockUnsafe(x, y, z);

                // Solo renderizar bloques sólidos
                if (!block.esSolido()) {
                    continue;
                }

                // Obtener coordenadas mundiales
                float worldX = worldXStart + x;
                float worldZ = worldZStart + z;

                // Convertir a coordenadas de pantalla
                float screenX, screenY;
                camera.worldToScreen(worldX, static_cast<float>(y), worldZ, screenX, screenY);

                // Agregar a la lista de renderizado
                RenderTile tile;
                tile.x = screenX;
                tile.y = screenY;
                tile.type = block.type;
                tile.worldY = y;
                tiles.push_back(tile);
            }
        }
    }

    return tiles;
}

/**
 * @brief Ordena tiles por profundidad (back-to-front)
 * @param tiles Vector de tiles (se modifica in-place)
 *
 * Ordena usando fórmula de profundidad isométrica:
 * depth = screenX + screenY + worldY*2
 *
 * Los tiles con menor depth se renderizan primero.
 */
void Renderer::sortTilesByDepth(std::vector<RenderTile>& tiles) {
    std::sort(tiles.begin(), tiles.end(), [](const RenderTile& a, const RenderTile& b) {
        // Calcular profundidad isométrica correcta
        float depthA = a.x + a.y + a.worldY * 2.0f;
        float depthB = b.x + b.y + b.worldY * 2.0f;

        return depthA < depthB;
    });
}

/**
 * @brief Radix Sort O(n) para tiles por profundidad isométrica
 * @param tiles Vector de tiles (se modifica in-place)
 *
 * OPTIMIZACIÓN FASE 2: Implementación LSD radix sort para 32-bit integers.
 * Procesa 8 bits a la vez (4 pasadas para 32 bits).
 * Maneja signed integers invirtiendo el bit de signo.
 *
 * Complejidad: O(n) vs O(n log n) de std::sort
 * Memoria: O(n) para buffer temporal
 */
void Renderer::radixSortTilesByDepth(std::vector<RenderTile>& tiles) {
    if (tiles.size() <= 1) return;

    // Buffer temporal para swapping
    std::vector<RenderTile> temp(tiles.size());

    // 4 pasadas de 8 bits cada una (32 bits total)
    for (int shift = 0; shift < 32; shift += 8) {
        // Inicializar buckets para 256 valores posibles (8 bits)
        size_t count[256] = {0};

        // Contar frecuencia de cada valor de bucket
        for (size_t i = 0; i < tiles.size(); i++) {
            // Calcular profundidad como entero
            int depth = tiles[i].worldX + tiles[i].worldZ + tiles[i].worldY * 2;

            // XOR con bit de signo para manejar signed integers correctamente
            // Esto convierte -1 a 0x7FFFFFFF, manteniendo orden correcto
            uint32_t signedDepth = static_cast<uint32_t>(depth) ^ 0x80000000;

            // Extraer 8 bits actuales
            int bucket = (signedDepth >> shift) & 0xFF;
            count[bucket]++;
        }

        // Convertir conteos a posiciones acumuladas
        size_t total = 0;
        for (int i = 0; i < 256; i++) {
            size_t oldCount = count[i];
            count[i] = total;
            total += oldCount;
        }

        // Distribuir elementos en buffer temporal
        for (size_t i = 0; i < tiles.size(); i++) {
            int depth = tiles[i].worldX + tiles[i].worldZ + tiles[i].worldY * 2;
            uint32_t signedDepth = static_cast<uint32_t>(depth) ^ 0x80000000;
            int bucket = (signedDepth >> shift) & 0xFF;

            temp[count[bucket]++] = tiles[i];
        }

        // Swap arrays para siguiente pasada
        std::swap(tiles, temp);
    }
}

/**
 * @brief Renderiza el mundo completo con optimizaciones
 * @param chunks Vector de chunks visibles
 * @param camera Cámara para proyección
 *
 * Pipeline:
 * 1. Recopilar todos los tiles de todos los chunks
 * 2. Aplicar frustum culling (descartar fuera de pantalla)
 * 3. Ordenar por profundidad (altura Y)
 * 4. Renderizar tiles ordenados
 *
 * Optimizaciones:
 * - Solo bloques sólidos
 * - Frustum culling con margen 100px
 * - Sort por Y (más simple que screenX+screenY)
 */
bool Renderer::isChunkVisible(const Chunk* chunk, const Camera& camera, int screenWidth, int screenHeight) {
    ChunkPos pos = chunk->getPosition();

    // OPTIMIZACIÓN FASE 2: Proyectar solo el centro del chunk + radio aproximado
    // OPTIMIZACIÓN FASE 2: centerY ajustado para WORLD_HEIGHT=32 (promedio de 3-28)
    float centerX = pos.x * BlockConfig::CHUNK_SIZE + BlockConfig::CHUNK_SIZE * 0.5f;
    float centerZ = pos.z * BlockConfig::CHUNK_SIZE + BlockConfig::CHUNK_SIZE * 0.5f;
    float centerY = 15.5f;  // Altura promedio del terreno (mitad de 3-28)

    // Proyectar solo el centro del chunk (1 proyección en lugar de 8)
    float screenX, screenY;
    camera.worldToScreen(centerX, centerY, centerZ, screenX, screenY);

    // Radio aproximado en pantalla (chunk_size × tile_size × zoom / 2)
    // Estimación conservadora para chunk de 8×8 en altura máxima
    float zoom = camera.getZoom();
    const float approximateRadius = BlockConfig::CHUNK_SIZE * 64.0f * zoom;  // 64 = TILE_WIDTH + TILE_HEIGHT aprox

    const int margin = 100;
    return !(screenX + approximateRadius < -margin ||
             screenX - approximateRadius > screenWidth + margin ||
             screenY + approximateRadius < -margin ||
             screenY - approximateRadius > screenHeight + margin);
}

void Renderer::renderWorld(const std::vector<Chunk*>& chunks, const Camera& camera) {
    // OPTIMIZACIÓN: Usar resize(0) en lugar de clear() para mantener capacidad sin reallocation
    m_tileCache.resize(0);

    // Obtener tamaño de pantalla UNA vez (no en el loop)
    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(m_renderer, &screenWidth, &screenHeight);

    int solidBlocks = 0;
    int culledBlocks = 0;
    int addedBlocks = 0;

    // Obtener posición del jugador para cálculos de distancia LOD
    float camX, camY, camZ;
    camera.getPosition(camX, camY, camZ);

    for (const Chunk* chunk : chunks) {
        if (!chunk) continue;

        // Bounding box culling - descartar chunk completo si está fuera de pantalla
        if (!isChunkVisible(chunk, camera, screenWidth, screenHeight)) {
            continue;
        }

        ChunkPos chunkPos = chunk->getPosition();
        int worldXStart = chunkPos.x * BlockConfig::CHUNK_SIZE;
        int worldZStart = chunkPos.z * BlockConfig::CHUNK_SIZE;

        // OPTIMIZACIÓN 9: LOD SYSTEM - Calcular distancia del chunk al jugador
        float chunkCenterX = worldXStart + BlockConfig::CHUNK_SIZE * 0.5f;
        float chunkCenterZ = worldZStart + BlockConfig::CHUNK_SIZE * 0.5f;
        float distX = chunkCenterX - camX;
        float distZ = chunkCenterZ - camZ;
        float distanceSquared = distX * distX + distZ * distZ;

        // Determinar nivel de detalle según distancia
        // LOD 0: 0-8 chunks (completo)
        // LOD 1: 8-16 chunks (medio)
        // LOD 2: 16+ chunks (mínimo)
        int lodLevel = 0;
        const float lod1Distance = BlockConfig::CHUNK_SIZE * 8.0f;  // ~64 bloques
        const float lod2Distance = BlockConfig::CHUNK_SIZE * 16.0f; // ~128 bloques

        if (distanceSquared > lod2Distance * lod2Distance) {
            lodLevel = 2;  // Muy lejos - mínimo detalle
        } else if (distanceSquared > lod1Distance * lod1Distance) {
            lodLevel = 1;  // Medio lejos - detalle medio
        }
        // lodLevel 0: Cerca - detalle completo

        // Recorrer todos los bloques del chunk (OCCLUSION CULLING con heightmap)
        for (int x = 0; x < BlockConfig::CHUNK_SIZE; x++) {
            for (int z = 0; z < BlockConfig::CHUNK_SIZE; z++) {
                // OPTIMIZACIÓN: Solo iterar hasta la altura máxima de esta columna
                int maxY = chunk->getMaxY(x, z);

                for (int y = 0; y <= maxY; y++) {
                    const Block& block = chunk->getBlockUnsafe(x, y, z);

                    // Solo renderizar bloques sólidos
                    if (!block.esSolido()) {
                        continue;
                    }

                    solidBlocks++;

                    // OPTIMIZACIÓN 9: LOD - Skip bloques underground según nivel de detalle
                    if (lodLevel >= 2) {
                        // LOD 2: Solo renderizar bloques superficiales (últimos 5 de la columna)
                        int surfaceY = maxY;
                        if (y < surfaceY - 5) {
                            continue;  // Skip bloques profundos
                        }
                    } else if (lodLevel == 1) {
                        // LOD 1: Solo renderizar bloques superficiales (últimos 15 de la columna)
                        int surfaceY = maxY;
                        if (y < surfaceY - 15) {
                            continue;  // Skip bloques profundos
                        }
                    }
                    // lodLevel 0: Renderizar todos los bloques (completo)

                    // OPTIMIZACIÓN 7: FACE CULLING - No renderizar bloques completamente ocultos
                    // Un bloque es visible si AL MENOS UNA de sus 6 caras está expuesta (tiene vecino aire)
                    // En LOD 1 y 2, skip face culling para mejorar rendimiento
                    bool hasExposedFace = true;  // Default para LOD >= 1

                    if (lodLevel == 0) {
                        // LOD 0: Face culling completo
                        hasExposedFace = false;

                        // Verificar cara SUPERIOR (y+1) - la más importante en isométrico
                        if (y < BlockConfig::WORLD_HEIGHT - 1) {
                            const Block& above = chunk->getBlockUnsafe(x, y + 1, z);
                            // Si el bloque encima es un árbol, consideramos que la cara superior está expuesta
                            // (queremos ver el grass debajo del árbol)
                            bool isAboveTree = (above.type == BlockType::ARBOL_SECO ||
                                              above.type == BlockType::ARBOL_GRASS ||
                                              above.type == BlockType::ARBOL_SANGRE);
                            if (!above.esSolido() || isAboveTree) {
                                hasExposedFace = true;
                            }
                        } else {
                            hasExposedFace = true;  // Borde superior del mundo
                        }

                        // Verificar caras LATERALES si la superior no está expuesta
                        if (!hasExposedFace) {
                            // Cara X+ (este)
                            if (x < BlockConfig::CHUNK_SIZE - 1) {
                                const Block& east = chunk->getBlockUnsafe(x + 1, y, z);
                                if (!east.esSolido()) hasExposedFace = true;
                            } else {
                                hasExposedFace = true;  // Borde del chunk
                            }

                            // Cara X- (oeste)
                            if (!hasExposedFace && x > 0) {
                                const Block& west = chunk->getBlockUnsafe(x - 1, y, z);
                                if (!west.esSolido()) hasExposedFace = true;
                            } else if (!hasExposedFace && x == 0) {
                                hasExposedFace = true;  // Borde del chunk
                            }

                            // Cara Z+ (sur)
                            if (!hasExposedFace && z < BlockConfig::CHUNK_SIZE - 1) {
                                const Block& south = chunk->getBlockUnsafe(x, y, z + 1);
                                if (!south.esSolido()) hasExposedFace = true;
                            } else if (!hasExposedFace && z == BlockConfig::CHUNK_SIZE - 1) {
                                hasExposedFace = true;  // Borde del chunk
                            }

                            // Cara Z- (norte)
                            if (!hasExposedFace && z > 0) {
                                const Block& north = chunk->getBlockUnsafe(x, y, z - 1);
                                if (!north.esSolido()) hasExposedFace = true;
                            } else if (!hasExposedFace && z == 0) {
                                hasExposedFace = true;  // Borde del chunk
                            }
                        }
                    }

                    // Si ninguna cara está expuesta, el bloque está completamente oculto
                    if (!hasExposedFace) {
                        continue;  // Skip - occlusion culling
                    }

                    // Frustum culling básico
                    float worldX = static_cast<float>(worldXStart + x);
                    float worldZ = static_cast<float>(worldZStart + z);

                    float screenX, screenY;
                    camera.worldToScreen(worldX, static_cast<float>(y), worldZ, screenX, screenY);

                    // Verificar si está dentro de la pantalla (con margen)
                    const int margin = 100;

                    if (screenX < -margin || screenX > screenWidth + margin ||
                        screenY < -margin || screenY > screenHeight + margin) {
                        culledBlocks++;
                        continue;  // Fuera de pantalla
                    }

                    // Agregar a la lista
                    RenderTile tile;
                    tile.x = screenX;
                    tile.y = screenY;
                    tile.type = block.type;
                    tile.worldY = y;
                    tile.worldX = worldXStart + x;
                    tile.worldZ = worldZStart + z;

                    m_tileCache.push_back(tile);
                    addedBlocks++;
                }
            }
        }
    }

    // OPTIMIZACIÓN FASE 2: Radix Sort O(n) en lugar de std::sort O(n log n)
    // Ordenar por profundidad isométrica correcta (back-to-front)
    // Fórmula: depth = X + Z + Y*2 (Y tiene doble peso en isométrico)
    radixSortTilesByDepth(m_tileCache);

    // NOTA: Radix sort es estable para claves únicas, pero si hay empates
    // usamos un criterio secundario de desempate. Como radix sort ya está
    // implementado para la profundidad completa, los empates son raros.

    // OPTIMIZACIÓN 1: Pre-calcular dimensiones escaladas UNA vez por frame
    float zoom = camera.getZoom();
    m_textureManager.updateScaledDimensions(zoom);

    // OPTIMIZACIÓN 2: INSTANCED RENDERING con orden de profundidad preservado
    // Iterar en orden de profundidad y hacer batch de tiles contiguos del mismo tipo
    BlockType currentType = BlockType::AIRE;
    TextureManager::TextureInfo* currentTexInfo = nullptr;
    SDL_Rect currentSrcRect{};
    int currentScaledWidth = 0;
    int currentScaledHeight = 0;
    bool textureValid = false;

    for (const RenderTile& tile : m_tileCache) {
        // Si el tipo cambió, necesitamos obtener nueva info de textura
        if (tile.type != currentType) {
            currentType = tile.type;
            currentTexInfo = m_textureManager.getBlockTexture(currentType);
            textureValid = false;

            if (currentTexInfo && currentTexInfo->texture) {
                currentScaledWidth = currentTexInfo->scaledWidth;
                currentScaledHeight = currentTexInfo->scaledHeight;
                textureValid = true;
            }
        }

        // Renderizar si la textura es válida
        if (textureValid) {
            // Verificar si es un árbol para obtener el sprite correcto
            bool isTree = (currentType == BlockType::ARBOL_SECO ||
                          currentType == BlockType::ARBOL_GRASS ||
                          currentType == BlockType::ARBOL_SANGRE);

            // Obtener el rectángulo fuente (diferente para cada árbol)
            if (isTree) {
                // Cada árbol tiene su propio sprite basado en posición
                currentSrcRect = m_textureManager.getTreeSpriteRect(currentType, tile.worldX, tile.worldZ);
            } else {
                // Los bloques normales comparten el mismo sprite
                currentSrcRect = m_textureManager.getSpriteSheetRect(currentType);
            }

            // Si el sprite no es válido, skip
            if (currentSrcRect.w == 0 || currentSrcRect.h == 0) {
                continue;
            }

            SDL_Rect destRect;
            // Centrar el bloque/árbol en la posición
            destRect.x = static_cast<int>(tile.x - currentScaledWidth / 2.0f + 0.5f);
            destRect.y = static_cast<int>(tile.y - currentScaledHeight + 0.5f);
            destRect.w = currentScaledWidth;
            destRect.h = currentScaledHeight;

            SDL_RenderCopy(m_renderer, currentTexInfo->texture, &currentSrcRect, &destRect);
        }
    }
}

/**
 * @brief Renderiza el jugador
 * @param camera Cámara para proyección
 * @param playerX, playerY, playerZ Posición del jugador
 * @param tileName Nombre de la textura ("player")
 *
 * Convierte posición del jugador a pantalla y dibuja la textura centrada.
 * Se renderiza DESPUÉS del mundo para aparecer encima del terreno.
 */
void Renderer::renderPlayer(const Camera& camera, float playerX, float playerY, float playerZ, const char* tileName) {
    // Convertir posición del jugador a coordenadas de pantalla
    float screenX, screenY;
    camera.worldToScreen(playerX, playerY, playerZ, screenX, screenY);

    // Obtener textura del player
    SDL_Texture* texture = m_textureManager.getTexture(tileName);
    if (!texture) {
        return;
    }

    // Obtener dimensiones de la textura
    int texWidth, texHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &texWidth, &texHeight);

    // Aplicar zoom al tamaño del player
    float zoom = camera.getZoom();
    texWidth = static_cast<int>(texWidth * zoom);
    texHeight = static_cast<int>(texHeight * zoom);

    // Crear rectángulo de destino (centrado)
    SDL_Rect destRect;
    destRect.x = static_cast<int>(std::round(screenX - texWidth / 2.0f));
    destRect.y = static_cast<int>(std::round(screenY - texHeight));
    destRect.w = texWidth;
    destRect.h = texHeight;

    // Renderizar textura del player
    SDL_RenderCopy(m_renderer, texture, nullptr, &destRect);
}

/**
 * @brief Configura el color de fondo
 * @param r, g, b Componentes RGB [0, 255]
 */
void Renderer::setClearColor(Uint8 r, Uint8 g, Uint8 b) {
    m_clearR = r;
    m_clearG = g;
    m_clearB = b;
}
