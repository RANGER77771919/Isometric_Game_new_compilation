/**
 * @file Game.hpp
 * @brief Define la clase principal del juego
 *
 * Este archivo contiene la definición de Game, GameState y las constantes
 * de configuración del game loop, sistema de chunks y controles.
 */

#pragma once

#include "core/World.hpp"
#include "core/Camera.hpp"
#include "rendering/Renderer.hpp"
#include "core/Player.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <atomic>

/**
 * @struct GameState
 * @brief Estado del juego (input y flags de control)
 *
 * Contiene todas las banderas booleanas que controlan el estado
 * del juego y el input del usuario.
 */
struct GameState {
    bool running = true;   ///< true si el juego está ejecutándose, false para salir
    bool paused = false;   ///< true si el juego está pausado

    // Movimiento del jugador/cámara (WASD)
    bool moveUp = false;    ///< W: mover norte (Z negativo)
    bool moveDown = false;  ///< S: mover sur (Z positivo)
    bool moveLeft = false;  ///< A: mover oeste (X negativo)
    bool moveRight = false; ///< D: mover este (X positivo)

    // Control de zoom (+/-)
    bool zoomIn = false;    ///< +: acercar (aumentar zoom)
    bool zoomOut = false;   ///< -: alejar (disminuir zoom)
};

/**
 * @class Game
 * @brief Clase principal del juego
 *
 * Gestiona el game loop, inicialización, input, actualización y renderizado.
 *
 * Arquitectura:
 * - init(): Inicializa SDL2, ventana, renderer, mundo, cámara, jugador
 * - run(): Game loop principal (handleInput -> update -> render)
 * - handleInput(): Procesa eventos SDL (teclado, ventana)
 * - update(): Actualiza lógica (movimiento, chunks)
 * - render(): Renderiza mundo y jugador
 * - cleanup(): Libera recursos
 *
 * Sistema de chunks:
 * - RENDER_RADIUS: chunks visibles para renderizado
 * - LOAD_RADIUS: chunks cargados en memoria
 * - UNLOAD_DISTANCE: distancia para liberar chunks
 * - MOVEMENT_THRESHOLD: movimiento mínimo para recargar chunks
 *
 * Controles:
 * - WASD: movimiento horizontal (cardinal: N/S/E/W)
 * - +/-: zoom in/out
 * - P: pausar
 * - ESC: salir
 */
class Game {
public:
    /**
     * @brief Constructor - inicializa miembros
     *
     * Inicializa m_lastChunkPos en (0,0) y m_lastChunkUpdateTime en 0.
     */
    Game();

    /**
     * @brief Destructor
     *
     * Note: cleanup() debe llamarse explícitamente antes de la destrucción.
     */
    ~Game();

    /**
     * @brief Inicializa todos los subsistemas del juego
     * @return true si la inicialización fue exitosa
     *
     * Proceso:
     * 1. Inicializar SDL2
     * 2. Crear ventana (1280x720, redimensionable)
     * 3. Crear renderer
     * 4. Crear mundo con semilla aleatoria
     * 5. Crear cámara
     * 6. Generar chunks iniciales
     * 7. Crear jugador
     * 8. Spawnsar jugador en superficie
     * 9. Posicionar cámara en jugador
     */
    bool init();

    /**
     * @brief Ejecuta el game loop principal
     *
     * Loop principal:
     * while (running):
     *   - Calcular deltaTime
     *   - handleInput(): procesar eventos
     *   - if (!paused): update(deltaTime)
     *   - render()
     *   - Limitar FPS a 60
     */
    void run();

    /**
     * @brief Limpia todos los recursos del juego
     *
 * Libera renderer, mundo, cámara, jugador y ventana SDL2.
     * Debe llamarse explícitamente antes de salir.
     */
    void cleanup();

private:
    /**
     * @brief Procesa input del usuario
     *
     * Maneja eventos SDL:
     * - SDL_QUIT: cerrar ventana -> running = false
     * - SDL_KEYDOWN: teclas presionadas -> actualizar estado
     * - SDL_KEYUP: teclas liberadas -> actualizar estado
     * - SDL_WINDOWEVENT: redimensionar ventana -> actualizar centro de cámara
     *
     * Controles:
     * - ESC: salir
     * - WASD: movimiento horizontal
     * - +/-: zoom
     * - P: pausar
     */
    void handleInput();

    /**
     * @brief Actualiza la lógica del juego
     * @param deltaTime Tiempo transcurrido desde el último frame (segundos)
     *
     * Llama a:
     * - updateCamera(): movimiento del jugador y cámara
     * - updateChunks(): carga/descarga de chunks
     */
    void update(float deltaTime);

    /**
     * @brief Renderiza el juego
     *
     * Pipeline:
     * 1. renderer->clear(): limpiar pantalla
     * 2. Obtener chunks visibles (RENDER_RADIUS)
     * 3. renderer->renderWorld(): renderizar terreno
     * 4. renderer->renderPlayer(): renderizar jugador
     * 5. renderer->present(): mostrar en pantalla
     * 6. Mostrar FPS (cada 1 segundo)
     */
    void render();

    /**
     * @brief Actualiza la posición del jugador y cámara
     * @param deltaTime Tiempo transcurrido (segundos)
     *
     * Proceso:
     * 1. Calcular movimiento según input (WASD)
     * 2. Aplicar movimiento al jugador (no a la cámara)
     * 3. Hacer que la cámara siga al jugador
     * 4. Aplicar zoom si está activo
     *
     * Movimiento cardinales:
     * - W (arriba): Z-- (norte)
     * - S (abajo): Z++ (sur)
     * - A (izquierda): X-- (oeste)
     * - D (derecha): X++ (este)
     */
    void updateCamera(float deltaTime);

    /**
     * @brief Carga/descarga chunks dinámicamente
     *
     * Optimización: solo procesa si el jugador se movió suficiente
     * (MOVEMENT_THRESHOLD chunks desde la última posición).
     *
     * Proceso:
     * 1. Obtener posición actual de la cámara en coordenadas de chunk
     * 2. Calcular distancia desde la última posición
     * 3. Si distancia >= MOVEMENT_THRESHOLD:
     *    a. Descargar chunks lejanos (unloadChunksFarFrom)
     *    b. Cargar nuevos chunks necesarios (getChunksAround)
     *    c. Actualizar última posición
     *    d. Mostrar estadísticas (cada 1 segundo)
     *
     * Esto evita procesamiento innecesario cuando el jugador está quieto.
     */
    void updateChunks();

    /**
     * @brief Obtiene ChunkPos desde coordenadas de cámara con cache
     * @param camX, camY, camZ Coordenadas de la cámara
     * @return ChunkPos correspondiente (con cache O(1))
     *
     * OPTIMIZACIÓN FASE 1: Simplificada para mejor inline efectivo.
     * Cachea la conversión para evitar std::floor repetidas cuando la cámara no se ha movido.
     */
    inline ChunkPos getCameraChunkPos(float camX, float camY, float camZ) {
        // Fast path: cache hit (sin conversión)
        if (m_cachedCamX == camX && m_cachedCamY == camY && m_cachedCamZ == camZ) {
            return m_cachedChunkPos;
        }

        // Slow path: actualizar cache
        m_cachedCamX = camX;
        m_cachedCamY = camY;
        m_cachedCamZ = camZ;

        // Conversión directa a ChunkPos (evita BlockPos temporal)
        int chunkX = (camX >= 0.0f) ? static_cast<int>(camX) / BlockConfig::CHUNK_SIZE
                                    : (static_cast<int>(camX) + 1) / BlockConfig::CHUNK_SIZE - 1;
        int chunkZ = (camZ >= 0.0f) ? static_cast<int>(camZ) / BlockConfig::CHUNK_SIZE
                                    : (static_cast<int>(camZ) + 1) / BlockConfig::CHUNK_SIZE - 1;

        m_cachedChunkPos = ChunkPos(chunkX, chunkZ);
        return m_cachedChunkPos;
    }

    // Miembros del juego
    SDL_Window* m_window;                    ///< Ventana SDL2
    std::unique_ptr<Renderer> m_renderer;    ///< Renderer isométrico
    std::unique_ptr<World> m_world;          ///< Mundo procedural infinito
    std::unique_ptr<Camera> m_camera;        ///< Cámara isométrica
    std::unique_ptr<Player> m_player;        ///< Jugador

    GameState m_state;                       ///< Estado del juego

    Uint64 m_lastFrameTime;                  ///< Tiempo del último frame (ms)
    const int FPS = 60;                      ///< Objetivo de FPS
    const int FRAME_DELAY = 1000 / FPS;       ///< Delay por frame (ms) para 60 FPS

    // Configuración de carga de chunks optimizada
    static constexpr int RENDER_RADIUS = 5;       ///< Radio de chunks a renderizar (11x11 = 121)
    static constexpr int LOAD_RADIUS = 6;         ///< Radio de chunks en memoria (13x13 = 169)
    static constexpr int UNLOAD_DISTANCE = 9;     ///< Distancia para descartar chunks lejanos
    static constexpr int MOVEMENT_THRESHOLD = 2;  ///< Movimiento mínimo para recargar chunks
    static constexpr float ZOOM_SPEED = 2.0f;     ///< Segundos para zoom min->max

    // Seguimiento de movimiento para optimizar carga/descarga
    ChunkPos m_lastChunkPos;         ///< Última posición de la cámara (en coordenadas de chunk)
    Uint64 m_lastChunkUpdateTime;   ///< Última vez que se mostraron estadísticas de chunks

    // OPTIMIZACIÓN FASE 3: Object pool para evitar allocations en render()
    std::vector<Chunk*> m_visibleChunksCache;  ///< Cache reutilizable de chunks visibles

    // OPTIMIZACIÓN FASE 4: Cache de ChunkPos para evitar conversiones repetidas
    float m_cachedCamX = 0.0f, m_cachedCamY = 0.0f, m_cachedCamZ = 0.0f;
    ChunkPos m_cachedChunkPos;  ///< Última ChunkPos calculada (cache)
};
