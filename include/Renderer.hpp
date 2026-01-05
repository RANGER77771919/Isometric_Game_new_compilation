/**
 * @file Renderer.hpp
 * @brief Define las clases de renderizado isométrico
 *
 * Este archivo contiene la definición de Renderer, TextureManager y
 * estructuras auxiliares para el renderizado del mundo isométrico 3D
 * en una superficie 2D usando SDL2.
 */

#pragma once

#include "Chunk.hpp"
#include "Camera.hpp"
#include <SDL2/SDL.h>
#include <stb_image.h>
#include <string>
#include <unordered_map>

/**
 * @struct RenderTile
 * @brief Información de un tile para renderizar
 *
 * Estructura ligera que contiene la información necesaria para
 * renderizar un tile en pantalla, usada para el sistema de
 * ordenación por profundidad (depth sorting).
 */
struct RenderTile {
    float x, y;       ///< Posición en pantalla (píxeles)
    BlockType type;   ///< Tipo de bloque (determina la textura)
    int worldY;       ///< Altura en el mundo (para orden de profundidad)
    int worldX, worldZ; ///< Coordenadas mundiales X y Z (para selección de árbol)
};

/**
 * @class TextureManager
 * @brief Gestor de texturas de tiles usando stb_image
 *
 * Carga y gestiona las texturas de los bloques desde archivos PNG.
 * Usa stb_image para cargar imágenes y SDL2 para crear texturas GPU.
 *
 * Características:
 * - Carga asíncrona de imágenes PNG
 * - Cache de texturas en unordered_map (búsqueda O(1))
 * - Gestión automática de memoria (SDL_DestroyTexture)
 * - Advertencias de texturas faltantes (solo una vez por textura)
 */
class TextureManager {
public:
    /**
     * @brief Información de textura con dimensiones cacheadas
     */
    struct TextureInfo {
        SDL_Texture* texture = nullptr;
        int width = 0;
        int height = 0;
        int scaledWidth = 0;    ///< Dimensión escalada por zoom (cache)
        int scaledHeight = 0;   ///< Dimensión escalada por zoom (cache)
    };

    /**
     * @brief Constructor del gestor de texturas
     * @param renderer Renderer SDL2 para crear las texturas
     *
     * Inicializa el mapa vacío y guarda el renderer.
     */
    TextureManager(SDL_Renderer* renderer);

    /**
     * @brief Destructor - libera todas las texturas
     *
     * Itera sobre todas las texturas y llama SDL_DestroyTexture()
     * para liberar la memoria de GPU.
     */
    ~TextureManager();

    /**
     * @brief Carga una textura desde archivo
     * @param name Nombre identificador de la textura
     * @param path Ruta al archivo PNG
     * @return true si se cargó correctamente, false en caso contrario
     *
     * Proceso:
     * 1. Cargar imagen con stbi_load() (RGB + alpha)
     * 2. Crear superficie SDL desde los datos
     * 3. Crear textura SDL desde la superficie
     * 4. Liberar datos temporales
     * 5. Almacenar en el mapa con 'name' como clave
     */
    bool loadTexture(const std::string& name, const std::string& path);

    /**
     * @brief Obtiene una textura por nombre
     * @param name Nombre identificador de la textura
     * @return Puntero a la textura o nullptr si no existe
     *
     * Si la textura no existe, imprime advertencia (solo una vez)
     * y devuelve nullptr. El renderizado debe manejar esto.
     */
    SDL_Texture* getTexture(const std::string& name) const;

    /**
     * @brief Carga todos los tiles del juego
     * @return true si todas se cargaron correctamente
     *
     * Carga las texturas de:
     * - grass, dirt, stone, sand, snow, water
     * - blood_grass, full_blood_grass
     * - player
     *
     * Todas deben estar en assets/tiles/ como PNG.
     */
    bool loadAllTextures();

    /**
     * @brief Obtiene textura de bloque directamente por BlockType
     * @param type Tipo de bloque
     * @return Puntero a TextureInfo con textura y dimensiones escaladas, o nullptr si no existe
     *
     * Acceso O(1) por índice, más rápido que buscar por nombre.
     * Devuelve TextureInfo para acceder directamente a scaledWidth/scaledHeight cacheados.
     */
    TextureInfo* getBlockTexture(BlockType type) const;

    /**
     * @brief Actualiza las dimensiones escaladas de todas las texturas
     * @param zoom Nivel de zoom actual
     *
     * Solo recalcula si el zoom cambió desde la última llamada (dirty flag).
     * Esto evita llamar SDL_QueryTexture repetidamente cuando el zoom no cambia.
     */
    void updateScaledDimensions(float zoom);

    /**
     * @brief Obtiene el sprite sheet completo
     * @return Puntero a la textura del sprite sheet o nullptr si no está cargado
     */
    SDL_Texture* getSpriteSheet() const { return m_spriteSheet; }

    /**
     * @brief Obtiene el rectángulo fuente para un tipo de bloque del sprite sheet
     * @param type Tipo de bloque
     * @return SDL_Rect con el área del sprite sheet correspondiente al bloque
     *
     * Los tipos de bloques se mapean a índices del sprite sheet (0-8):
     * PASTO=0, HIERBA_SANGRE=1, ARENA=2, PIEDRA=3, TIERRA=4, DIRT_ALT=5,
     * PASTO_FULL=6, NIEVE=7, AGUA=8
     */
    SDL_Rect getSpriteSheetRect(BlockType type) const;

    /**
     * @brief Obtiene el rectángulo fuente para un árbol del sprite sheet de árboles
     * @param type Tipo de árbol (ARBOL_SECO, ARBOL_GRASS, ARBOL_SANGRE)
     * @param worldX Coordenada X mundial (para selección aleatoria)
     * @param worldZ Coordenada Z mundial (para selección aleatoria)
     * @return SDL_Rect con el área del sprite sheet correspondiente al árbol
     *
     * Los árboles se distribuyen en:
     * - Fila 1 (0-11): Árboles secos para biomas secos
     * - Filas 2-3 (12-35): Árboles vivos para biomas grass
     * - Filas 4-5 (36-59): Árboles de sangre para biomas blood_grass
     */
    SDL_Rect getTreeSpriteRect(BlockType type, int worldX, int worldZ) const;

private:
    SDL_Renderer* m_renderer; ///< Renderer SDL2 para crear texturas
    std::unordered_map<std::string, TextureInfo> m_textures; ///< Mapa de texturas con dimensiones cacheadas
    std::array<TextureInfo*, 14> m_blockTextures{}; ///< Array de punteros a TextureInfo indexado por BlockType
    float m_lastZoom = -1.0f;  ///< Último zoom usado para dimensiones escaladas (dirty flag)

    // Sprite sheet configuration
    SDL_Texture* m_spriteSheet = nullptr;  ///< Sprite sheet de tiles
    static constexpr int SPRITE_TILE_SIZE = 32;  ///< Tamaño de cada tile en el sprite sheet

    // Tree sprite sheet configuration
    SDL_Texture* m_treeSpriteSheet = nullptr;  ///< Sprite sheet de árboles
    static constexpr int TREE_SPRITE_SIZE = 64;  ///< Tamaño de cada árbol en el sprite sheet (64x64)
    static constexpr int TREE_SPRITE_COLUMNS = 12;  ///< 12 columnas de árboles
};

/**
 * @class Renderer
 * @brief Renderizador isométrico usando SDL2
 *
 * Renderiza el mundo 3D en proyección isométrica sobre una superficie 2D.
 * Maneja texturas, cámara, profundidad y frustum culling.
 *
 * Pipeline de renderizado:
 * 1. clear(): Limpiar pantalla con color de fondo
 * 2. renderWorld(): Renderizar chunks visibles
 *    - Crear lista de tiles (con frustum culling)
 *    - Ordenar por profundidad (back-to-front)
 *    - Renderizar cada tile
 * 3. renderPlayer(): Renderizar jugador sobre el terreno
 * 4. present(): Voltear buffer y mostrar en pantalla
 *
 * Técnicas utilizadas:
 * - Frustum culling: solo renderiza tiles visibles
 * - Depth sorting: ordena tiles de atrás hacia adelante
 * - Batch rendering: agrupa todos los tiles antes de renderizar
 */
class Renderer {
public:
    /**
     * @brief Constructor del renderer
     * @param window Ventana SDL2 para crear el renderer
     *
     * Crea el renderer SDL2 con aceleración hardware y VSync.
     * Inicializa el TextureManager y carga todas las texturas.
     */
    Renderer(SDL_Window* window);

    /**
     * @brief Destructor - libera renderer SDL2
     */
    ~Renderer();

    /**
     * @brief Limpia la pantalla con el color de fondo
     *
     * Llena la pantalla con el color especificado (sky blue por defecto).
     * Debe llamarse al inicio de cada frame.
     */
    void clear();

    /**
     * @brief Presenta el renderizado (voltea el buffer)
     *
     * Intercambia los buffers de renderizado (double buffering).
     * Debe llamarse al final de cada frame.
     */
    void present();

    /**
     * @brief Renderiza un chunk completo
     * @param chunk Chunk a renderizar
     * @param camera Cámara para proyección isométrica
     *
     * Crea lista de tiles, ordena por profundidad y renderiza.
     * Actualmente no usado directamente (se usa renderWorld).
     */
    void renderChunk(const Chunk* chunk, const Camera& camera);

    /**
     * @brief Renderiza un bloque individual
     * @param screenX Posición X en pantalla
     * @param screenY Posición Y en pantalla
     * @param type Tipo de bloque (selecciona textura)
     * @param worldY Altura en el mundo (no usado directamente)
     *
     * Si type == AIRE, no renderiza.
     * Obtiene textura del TextureManager y la dibuja centrada.
     */
    void renderBlock(float screenX, float screenY, BlockType type, int worldY, float zoom = 1.0f);

    /**
     * @brief Renderiza el mundo (todos los chunks visibles)
     * @param chunks Vector de chunks a renderizar
     * @param camera Cámara para proyección
     *
     * Pipeline completo:
     * 1. Recopilar todos los tiles de todos los chunks
     * 2. Aplicar frustum culling (descartar tiles fuera de pantalla)
     * 3. Ordenar por profundidad (Y de mundo)
     * 4. Renderizar cada tile ordenado
     *
     * Optimizaciones:
     * - Solo renderiza bloques sólidos
     * - Frustum culling con margen de 100px
     * - Sort por altura Y (no por screenX + screenY)
     */
    void renderWorld(const std::vector<Chunk*>& chunks, const Camera& camera);

    /**
     * @brief Renderiza el jugador
     * @param camera Cámara para proyección
     * @param playerX, playerY, playerZ Posición del jugador
     * @param tileName Nombre de la textura ("player")
     *
     * Convierte posición del jugador a pantalla y dibuja la textura.
     * Se renderiza DESPUÉS del mundo para aparecer encima.
     */
    void renderPlayer(const Camera& camera, float playerX, float playerY, float playerZ, const char* tileName);

    /**
     * @brief Obtiene el renderer SDL2 subyacente
     * @return Puntero al renderer SDL2
     */
    SDL_Renderer* getSDLRenderer() { return m_renderer; }

    /**
     * @brief Obtiene el gestor de texturas
     * @return Puntero al TextureManager
     */
    TextureManager* getTextureManager() { return &m_textureManager; }

    /**
     * @brief Configura el color de fondo
     * @param r, g, b Componentes RGB [0, 255]
     *
     * El color se usa en clear().
     * Por defecto: (135, 206, 235) = sky blue
     */
    void setClearColor(Uint8 r, Uint8 g, Uint8 b);

private:
    SDL_Window* m_window;             ///< Ventana SDL2
    SDL_Renderer* m_renderer;         ///< Renderer SDL2
    TextureManager m_textureManager;  ///< Gestor de texturas
    Uint8 m_clearR, m_clearG, m_clearB; ///< Color de fondo
    std::vector<RenderTile> m_tileCache; ///< Cache reutilizable de tiles

    /**
     * @brief Verifica si un chunk es visible en pantalla
     * @param chunk Chunk a verificar
     * @param camera Cámara para proyección
     * @param screenWidth Ancho de pantalla
     * @param screenHeight Alto de pantalla
     * @return true si el chunk puede estar visible
     *
     * Usa bounding box para descartar chunks completos antes de iterar bloques.
     */
    bool isChunkVisible(const Chunk* chunk, const Camera& camera, int screenWidth, int screenHeight);

    /**
     * @brief Crea lista de tiles para renderizar
     * @param chunk Chunk a procesar
     * @param camera Cámara para proyección
     * @return Vector de RenderTile ordenado
     *
     * Convierte cada bloque sólido a RenderTile con coordenadas de pantalla.
     */
    std::vector<RenderTile> createRenderList(const Chunk* chunk, const Camera& camera);

    /**
     * @brief Ordena tiles por profundidad (back-to-front)
     * @param tiles Vector de tiles a ordenar (se modifica in-place)
     *
     * Ordena usando fórmula de profundidad isométrica:
     * depth = x + y + worldY*2
     *
     * Los tiles con menor depth se renderizan primero (están "atrás").
     * Los tiles con mayor depth se renderizan después (están "adelante").
     */
    void sortTilesByDepth(std::vector<RenderTile>& tiles);
};
