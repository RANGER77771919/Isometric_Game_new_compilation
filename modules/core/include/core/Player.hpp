#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>

// Declaración anticipada de World para evitar dependencia circular
class World;

/**
 * @class Player
 * @brief Representa al jugador en el mundo isométrico 3D
 *
 * El jugador tiene una posición en coordenadas mundiales (x, y, z) donde:
 * - X: Coordenada horizontal este-oeste
 * - Y: Coordenada vertical (altura)
 * - Z: Coordenada horizontal norte-sur
 *
 * El jugador puede moverse, spawnear en la superficie del terreno y ser
 * renderizado usando un tile específico.
 */
class Player {
public:
    /**
     * @brief Constructor del jugador
     * @param x Coordenada X inicial (posición horizontal este-oeste)
     * @param y Coordenada Y inicial (altura/vertical)
     * @param z Coordenada Z inicial (posición horizontal norte-sur)
     *
     * Inicializa el jugador en la posición especificada del mundo.
     * Generalmente se crea en (0, 0, 0) y luego se reubica con spawnOnSurface().
     * Las coordenadas se convierten a enteras para alineación con grid.
     */
    Player(float x, float y, float z);

    /**
     * @brief Destructor por defecto
     *
     * No necesita liberar recursos adicionales ya que solo almacena
     * coordenadas flotantes básicas.
     */
    ~Player() = default;

    /**
     * @brief Obtiene la posición actual del jugador
     * @param x Variable de salida para la coordenada X
     * @param y Variable de salida para la coordenada Y (altura)
     * @param z Variable de salida para la coordenada Z
     *
     * Las coordenadas se pasan por referencia para ser modificadas.
     * Útil para obtener la posición actual del jugador para renderizado,
     * detección de colisiones, o actualización de la cámara.
     */
    void getPosition(float& x, float& y, float& z) const;

    /**
     * @brief Establece la posición del jugador
     * @param x Nueva coordenada X (posición horizontal este-oeste)
     * @param y Nueva coordenada Y (altura/vertical)
     * @param z Nueva coordenada Z (posición horizontal norte-sur)
     *
     * Teletransporta al jugador a la posición especificada.
     * Útil para reposicionar al jugador después de cargar un chunks,
     * respawn, o efectos de teletransporte.
     */
    void setPosition(float x, float y, float z);

    /**
     * @brief Muve al jugador relativamente desde su posición actual
     * @param dx Delta de movimiento en el eje X (positivo = este, negativo = oeste)
     * @param dy Delta de movimiento en el eje Y (positivo = arriba, negativo = abajo)
     * @param dz Delta de movimiento en el eje Z (positivo = sur, negativo = norte)
     * @param world Puntero al mundo para detección de colisiones
     *
     * Los deltas se suman a la posición actual. Se usa durante el manejo
     * de input del jugador para movimiento continuo basado en deltaTime.
     * Los valores típicos son pequeños (ej: 0.1 por frame).
     *
     * Verifica colisiones con bloques sólidos antes de moverse.
     */
    void move(float dx, float dy, float dz, World* world);

    /**
     * @brief Spawnea al jugador en la superficie del terreno
     * @param world Puntero al mundo donde buscará la superficie
     *
     * Este método busca el bloque sólido más alto en la posición (X, Z) del jugador
     * y lo coloca sobre ese bloque (Y = superficie + 1).
     *
     * Algoritmo:
     * 1. Obtiene el chunk en la posición actual del jugador
     * 2. Si el chunk no existe, lo genera
     * 3. Busca desde arriba (Y=255) hacia abajo el primer bloque sólido
     * 4. Posiciona al jugador una unidad sobre ese bloque
     *
     * Esto asegura que el jugador siempre aparezca sobre el terreno
     * y no dentro de la tierra o flotando en el aire.
     */
    void spawnOnSurface(World* world);

    /**
     * @brief Intenta mover al jugador en una dirección
     * @param dx Delta de movimiento en X (-1, 0, o 1)
     * @param dy Delta de movimiento en Y (-1, 0, o 1)
     * @param dz Delta de movimiento en Z (-1, 0, o 1)
     * @param world Puntero al mundo
     * @return true si el movimiento fue iniciado
     *
     * Sistema de movimiento discreto por grid:
     * - Solo se mueve si no hay colisión en la nueva posición
     * - Inicia una interpolación visual suave
     * - Respeta el cooldown entre movimientos
     */
    bool tryMove(int dx, int dy, int dz, World* world);

    /**
     * @brief Actualiza la lógica del jugador (input y gravedad)
     * @param deltaTime Tiempo transcurrido desde el último frame
     * @param world Puntero al mundo
     *
     * Maneja:
     * - Cooldown de movimiento
     * - Gravedad discreta (caer 1 bloque cada intervalo)
     * - Actualización de interpolación visual
     * - Animación de salto
     */
    void update(float deltaTime, World* world);

    /**
     * @brief Intenta saltar
     * @param world Puntero al mundo
     * @return true si el salto fue iniciado
     *
     * El jugador salta si:
     * - No está saltando actualmente
     * - Hay espacio arriba (2 bloques)
     * - No está en cooldown
     */
    bool tryJump(World* world);

    /**
     * @brief Obtiene el nombre del tile usado para renderizar al jugador
     * @return Cadena con el nombre de la textura ("player")
     *
     * El renderer usa este nombre para buscar la textura correspondiente
     * en el TextureManager. La textura debe estar cargada en assets/tiles/player.png
     */
    const char* getTileName() const { return "player"; }

private:
    // Coordenadas lógicas (grid) - SIEMPRE enteras, alineadas con bloques
    int m_posX; ///< Coordenada X lógica (bloques este-oeste)
    int m_posY; ///< Coordenada Y lógica (altura en bloques)
    int m_posZ; ///< Coordenada Z lógica (bloques norte-sur)

    // Coordenadas visuales (float) - Para renderizado suave con interpolación
    float m_visualX; ///< Coordenada X visual (interpolada)
    float m_visualY; ///< Coordenada Y visual (interpolada)
    float m_visualZ; ///< Coordenada Z visual (interpolada)

    // Posiciones anteriores para interpolación
    float m_prevVisualX; ///< Posición X visual anterior
    float m_prevVisualY; ///< Posición Y visual anterior
    float m_prevVisualZ; ///< Posición Z visual anterior

    // Sistema de movimiento
    float m_lerpT = 0.0f; ///< Progreso de interpolación [0.0 - 1.0]
    float m_moveCooldown = 0.0f; ///< Tiempo restante de cooldown
    bool m_isMoving = false; ///< true si está en movimiento

    // Sistema de gravedad discreto
    float m_gravityTimer = 0.0f; ///< Timer para gravedad

    // Sistema de salto
    float m_jumpTimer = 0.0f; ///< Timer para animación de salto
    bool m_isJumping = false; ///< true si está saltando
    int m_jumpStartY = 0; ///< Posición Y donde empezó el salto
    int m_jumpTargetY = 0; ///< Posición Y objetivo del salto

    // Constantes de salto
    static constexpr int JUMP_HEIGHT = 1; ///< Altura del salto en bloques
    static constexpr float JUMP_DURATION = 0.2f; ///< Duración del salto (segundos)
    static constexpr float JUMP_COOLDOWN = 0.1f; ///< Cooldown después de saltar

    // Constantes de movimiento
    static constexpr float MOVE_DURATION = 0.15f; ///< Duración de la animación de movimiento (segundos)
    static constexpr float MOVE_COOLDOWN = 0.05f; ///< Cooldown entre movimientos (segundos)
    static constexpr float GRAVITY_INTERVAL = 0.1f; ///< Intervalo de caída (segundos)

    // Dimensiones del jugador para colisiones
    static constexpr float PLAYER_HEIGHT = 1.8f; ///< Altura del jugador en bloques
    static constexpr float PLAYER_WIDTH = 0.6f; ///< Ancho del jugador en bloques

    /**
     * @brief Verifica si hay colisión en una posición
     * @param x Posición X a verificar
     * @param y Posición Y a verificar
     * @param z Posición Z a verificar
     * @param world Puntero al mundo
     * @return true si hay colisión con un bloque sólido
     *
     * Verifica los bloques que ocuparía el jugador en la posición especificada.
     */
    bool checkCollision(float x, float y, float z, World* world) const;

    /**
     * @brief Verifica si el jugador puede moverse a una posición
     * @param x Coordenada X de destino (entera)
     * @param y Coordenada Y de destino (entera)
     * @param z Coordenada Z de destino (entera)
     * @param world Puntero al mundo
     * @return true si no hay colisión en la nueva posición
     *
     * Sistema simplificado de colisión para grid:
     * - Verifica solo pies y cabeza (2 bloques)
     * - Acepta coordenadas enteras
     */
    bool canMoveTo(int x, int y, int z, World* world) const;

    /**
     * @brief Función de interpolación lineal
     * @param a Valor inicial
     * @param b Valor final
     * @param t Progreso [0.0 - 1.0]
     * @return Valor interpolado
     *
     * Fórmula: a + (b - a) * t
     */
    static float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }

    /**
     * @brief Actualiza la posición visual con interpolación
     * @param deltaTime Tiempo transcurrido
     *
     * Interpola suavemente entre la posición visual anterior y la lógica actual.
     */
    void updateVisuals(float deltaTime);

    /**
     * @brief Aplica gravedad discreta (caer 1 bloque)
     * @param world Puntero al mundo
     * @return true si cayó un bloque
     */
    bool applyDiscreteGravity(World* world);

    /**
     * @brief Verifica si el jugador puede saltar
     * @param world Puntero al mundo
     * @return true si hay espacio para saltar
     *
     * OPTIMIZADO: El jugador puede saltar si hay 2 bloques libres arriba.
     */
    bool canJump(World* world) const;
};

#endif // PLAYER_HPP
