/**
 * @file Player.cpp
 * @brief Implementación de la clase Player
 *
 * Este archivo contiene la implementación de los métodos de la clase Player,
 * que representa al jugador en el mundo isométrico 3D.
 */

#include "core/Player.hpp"
#include "core/World.hpp"
#include "core/Block.hpp"
#include <iostream>
#include <cmath>  // Para std::floor

/**
 * @brief Constructor del jugador
 * @param x Coordenada X inicial (posición horizontal este-oeste)
 * @param y Coordenada Y inicial (altura/vertical)
 * @param z Coordenada Z inicial (posición horizontal norte-sur)
 *
 * Inicializa las coordenadas del jugador con los valores proporcionados.
 * Convierte las coordenadas float a enteras para el sistema grid.
 * Inicializa las coordenadas visuales iguales a las lógicas.
 */
Player::Player(float x, float y, float z)
    : m_posX(static_cast<int>(std::floor(x)))
    , m_posY(static_cast<int>(std::floor(y)))
    , m_posZ(static_cast<int>(std::floor(z)))
    , m_visualX(static_cast<float>(m_posX))
    , m_visualY(static_cast<float>(m_posY))
    , m_visualZ(static_cast<float>(m_posZ))
    , m_prevVisualX(m_visualX)
    , m_prevVisualY(m_visualY)
    , m_prevVisualZ(m_visualZ)
{
}

/**
 * @brief Obtiene la posición actual del jugador
 * @param x Variable de salida para la coordenada X
 * @param y Variable de salida para la coordenada Y (altura)
 * @param z Variable de salida para la coordenada Z
 *
 * Las coordenadas se pasan por referencia y son modificadas con los valores actuales.
 * Devuelve las coordenadas VISUALES (interpoladas) para renderizado suave.
 * El método es const ya que no modifica el estado del jugador.
 */
void Player::getPosition(float& x, float& y, float& z) const {
    x = m_visualX;
    y = m_visualY;
    z = m_visualZ;
}

/**
 * @brief Establece la posición del jugador
 * @param x Nueva coordenada X (posición horizontal este-oeste)
 * @param y Nueva coordenada Y (altura/vertical)
 * @param z Nueva coordenada Z (posición horizontal norte-sur)
 *
 * Teletransporta al jugador a la posición especificada.
 * Convierte a enteras para el sistema grid y sincroniza las visuales.
 * Útil para respawn inicial, teletransporte, o correcciones de posición.
 */
void Player::setPosition(float x, float y, float z) {
    m_posX = static_cast<int>(std::floor(x));
    m_posY = static_cast<int>(std::floor(y));
    m_posZ = static_cast<int>(std::floor(z));

    // Sincronizar posición visual instantáneamente (sin interpolación)
    m_visualX = static_cast<float>(m_posX);
    m_visualY = static_cast<float>(m_posY);
    m_visualZ = static_cast<float>(m_posZ);
    m_prevVisualX = m_visualX;
    m_prevVisualY = m_visualY;
    m_prevVisualZ = m_visualZ;
    m_lerpT = 1.0f;
    m_isMoving = false;
}

/**
 * @brief Mueve al jugador relativamente (MANTENIDO POR COMPATIBILIDAD)
 * @param dx Delta de movimiento en el eje X
 * @param dy Delta de movimiento en el eje Y
 * @param dz Delta de movimiento en el eje Z
 * @param world Puntero al mundo para detección de colisiones
 *
 * NOTA: Este método se mantiene por compatibilidad pero ya no se usa.
 * El nuevo sistema usa tryMove() para movimiento discreto por grid.
 */
void Player::move(float dx, float dy, float dz, World* world) {
    // Convertir movimiento float a discreto
    int moveX = (dx > 0.1f) ? 1 : ((dx < -0.1f) ? -1 : 0);
    int moveZ = (dz > 0.1f) ? 1 : ((dz < -0.1f) ? -1 : 0);

    if (moveX != 0 || moveZ != 0) {
        tryMove(moveX, 0, moveZ, world);
    }
}

/**
 * @brief Spawnea al jugador en la superficie del terreno
 * @param world Puntero al mundo donde se buscará la superficie
 *
 * Este método implementa el algoritmo para encontrar la superficie del terreno:
 *
 * 1. Validación: Verifica que el puntero al mundo no sea nulo
 * 2. Obtener chunk: Convierte la posición del jugador a coordenadas de chunk
 * 3. Generar si es necesario: Si el chunk no existe, lo genera
 * 4. Calcular coordenadas locales: Convierte coordenadas mundiales a locales del chunk
 * 5. Buscar superficie: Recorre desde Y=255 hacia abajo buscando el primer bloque sólido
 * 6. Posicionar jugador: Coloca al jugador en Y = superficie + 1 (encima del bloque)
 *
 * El algoritmo de búsqueda de superficie:
 * - Comienza desde el tope del mundo (Y=255)
 * - Recorre hacia abajo (Y decrementando)
 * - El primer bloque donde esSolido() es true es la superficie
 * - El jugador se coloca una unidad arriba de ese bloque
 *
 * Esto asegura que el jugador siempre aparezca sobre el terreno y no
 * dentro de bloques sólidos o flotando en el aire.
 */
void Player::spawnOnSurface(World* world) {
    // Validar que el puntero al mundo no sea nulo
    if (!world) {
        std::cerr << "Error: Mundo es nulo en spawnOnSurface" << std::endl;
        return;
    }

    // Convertir la posición del jugador a coordenadas de bloque
    // Nota: La coordenada Y se establece en 0 ya que nos interesa la columna (X, Z)
    BlockPos blockPos(static_cast<int>(m_posX), 0, static_cast<int>(m_posZ));
    ChunkPos chunkPos = blockPos.toChunkPos();

    // Obtener el chunk en la posición del jugador
    const Chunk* chunk = world->getChunk(chunkPos);

    // Si el chunk no existe, generarlo
    if (!chunk) {
        world->generateChunk(chunkPos);
        chunk = world->getChunk(chunkPos);

        // Verificar que se generó correctamente
        if (!chunk) {
            std::cerr << "Error: No se pudo generar el chunk" << std::endl;
            return;
        }
    }

    // Calcular coordenadas locales dentro del chunk
    // CHUNK_SIZE es 8, así que localX y localZ estarán en rango [0, 7]
    int localX = blockPos.x % BlockConfig::CHUNK_SIZE;
    int localZ = blockPos.z % BlockConfig::CHUNK_SIZE;

    // Manejar coordenadas negativas (el operador % en C++ puede dar negativos)
    // Si localX es -3, agregamos 8 para obtener 5 (coordenada válida)
    if (localX < 0) localX += BlockConfig::CHUNK_SIZE;
    if (localZ < 0) localZ += BlockConfig::CHUNK_SIZE;

    // Buscar desde arriba hacia abajo el primer bloque sólido
    // WORLD_HEIGHT es 256, así que buscamos desde Y=255 hasta Y=0
    int surfaceY = BlockConfig::WORLD_HEIGHT - 1;
    for (int y = BlockConfig::WORLD_HEIGHT - 1; y >= 0; y--) {
        const Block& block = chunk->getBlock(localX, y, localZ);
        if (block.esSolido()) {
            surfaceY = y;
            break; // Encontramos el primer bloque sólido desde arriba
        }
    }

    // Posicionar al jugador en la superficie + 1
    // +1 para que el jugador esté ENCIMA del bloque, no dentro de él
    m_posY = surfaceY + 1;

    // Sincronizar posición visual
    m_visualX = static_cast<float>(m_posX);
    m_visualY = static_cast<float>(m_posY);
    m_visualZ = static_cast<float>(m_posZ);
    m_prevVisualX = m_visualX;
    m_prevVisualY = m_visualY;
    m_prevVisualZ = m_visualZ;
    m_lerpT = 1.0f;
}

/**
 * @brief Actualiza la lógica del jugador (input y gravedad)
 * @param deltaTime Tiempo transcurrido desde el último frame
 * @param world Puntero al mundo
 *
 * Sistema grid-based con interpolación:
 * 1. Actualizar cooldown de movimiento
 * 2. Aplicar gravedad discreta (caer 1 bloque)
 * 3. Actualizar interpolación visual
 */
void Player::update(float deltaTime, World* world) {
    // Actualizar cooldown
    if (m_moveCooldown > 0.0f) {
        m_moveCooldown -= deltaTime;
        if (m_moveCooldown < 0.0f) {
            m_moveCooldown = 0.0f;
        }
    }

    // Aplicar gravedad discreta
    m_gravityTimer += deltaTime;
    if (m_gravityTimer >= GRAVITY_INTERVAL) {
        m_gravityTimer = 0.0f;
        applyDiscreteGravity(world);
    }

    // Actualizar interpolación visual
    updateVisuals(deltaTime);
}

/**
 * @brief Intenta mover al jugador en una dirección
 * @param dx Delta de movimiento en X (-1, 0, o 1)
 * @param dy Delta de movimiento en Y (-1, 0, o 1)
 * @param dz Delta de movimiento en Z (-1, 0, o 1)
 * @param world Puntero al mundo
 * @return true si el movimiento fue iniciado
 *
 * OPTIMIZADO: Reduce verificaciones duplicadas de bloques.
 *
 * Sistema de movimiento discreto por grid CON AUTO-JUMP:
 * - Primero intenta moverse normalmente
 * - Si hay bloque pero puede saltar, hace auto-jump + movimiento lateral
 * - Verifica colisiones y aplica cooldown
 */
bool Player::tryMove(int dx, int dy, int dz, World* world) {
    // Verificar cooldown (early return)
    if (m_moveCooldown > 0.0f || m_isMoving) {
        return false;
    }

    // OPTIMIZACIÓN: Solo calcular si hay movimiento
    if (dx == 0 && dy == 0 && dz == 0) {
        return false;
    }

    // Calcular nueva posición
    int newX = m_posX + dx;
    int newY = m_posY + dy;
    int newZ = m_posZ + dz;

    // OPTIMIZACIÓN: Verificar colisión UNA vez en lugar de múltiples
    bool canMoveNormally = false;
    bool needsAutoJump = false;

    if (world) {
        // Verificar movimiento normal
        const Block& feetBlock = world->getBlock(newX, newY, newZ);
        const Block& headBlock = world->getBlock(newX, newY + 1, newZ);

        // El jugador NO puede caminar donde hay un árbol (bloquea cualquier movimiento)
        bool isTree = (feetBlock.type == BlockType::ARBOL_SECO ||
                      feetBlock.type == BlockType::ARBOL_GRASS ||
                      feetBlock.type == BlockType::ARBOL_SANGRE);

        if (isTree) {
            // Hay un árbol, no se puede mover aquí ni siquiera con auto-jump
            canMoveNormally = false;
            needsAutoJump = false;
        } else {
            canMoveNormally = !feetBlock.esSolido() && !headBlock.esSolido();
        }

        // Si no se puede mover normalmente, verificar auto-jump
        if (!canMoveNormally && (dx != 0 || dz != 0)) {
            // Verificar condiciones de auto-jump
            const Block& groundBlock = world->getBlock(newX, m_posY, newZ);
            const Block& aboveBlock = world->getBlock(newX, m_posY + 1, newZ);
            const Block& headBlock2 = world->getBlock(newX, m_posY + 2, newZ);

            // NO permitir auto-jump sobre árboles
            bool isGroundTree = (groundBlock.type == BlockType::ARBOL_SECO ||
                              groundBlock.type == BlockType::ARBOL_GRASS ||
                              groundBlock.type == BlockType::ARBOL_SANGRE);

            // No permitir auto-jump si el bloque de encima es un árbol
            bool isAboveTree = (aboveBlock.type == BlockType::ARBOL_SECO ||
                              aboveBlock.type == BlockType::ARBOL_GRASS ||
                              aboveBlock.type == BlockType::ARBOL_SANGRE);

            // Bloque al nivel del suelo, espacio arriba, pero NO sobre árboles
            bool hasBlockToJump = groundBlock.esSolido() && !aboveBlock.esSolido() && !headBlock2.esSolido() && !isAboveTree && !isGroundTree;

            // Podemos saltar desde nuestra posición
            const Block& currentAbove1 = world->getBlock(m_posX, m_posY + 1, m_posZ);
            const Block& currentAbove2 = world->getBlock(m_posX, m_posY + 2, m_posZ);
            bool canJumpFromHere = !currentAbove1.esSolido() && !currentAbove2.esSolido();

            needsAutoJump = hasBlockToJump && canJumpFromHere && !m_isJumping;
        }
    } else {
        // Sin mundo = sin colisiones
        canMoveNormally = true;
    }

    // Ejecutar movimiento normal o auto-jump
    if (canMoveNormally) {
        // Movimiento normal
        m_prevVisualX = m_visualX;
        m_prevVisualY = m_visualY;
        m_prevVisualZ = m_visualZ;

        m_posX = newX;
        m_posY = newY;
        m_posZ = newZ;

        m_lerpT = 0.0f;
        m_isMoving = true;
        m_moveCooldown = MOVE_COOLDOWN;

        return true;
    }

    if (needsAutoJump) {
        // Auto-jump: saltar Y moverse lateralmente en UNA sola acción
        m_prevVisualX = m_visualX;
        m_prevVisualY = m_visualY;
        m_prevVisualZ = m_visualZ;

        // Moverse lateralmente Y saltar (simultáneo)
        m_posX = newX;
        m_posY = m_posY + JUMP_HEIGHT;  // Saltar 1 bloque
        m_posZ = newZ;

        // Iniciar animación de salto
        m_jumpStartY = m_prevVisualY;
        m_jumpTargetY = m_posY;

        m_jumpTimer = 0.0f;
        m_isJumping = true;
        m_lerpT = 0.0f;
        m_isMoving = true;
        m_moveCooldown = JUMP_COOLDOWN;

        return true;
    }

    return false;
}

/**
 * @brief Verifica si el jugador puede moverse a una posición
 * @param x Coordenada X de destino (entera)
 * @param y Coordenada Y de destino (entera)
 * @param z Coordenada Z de destino (entera)
 * @param world Puntero al mundo
 * @return true si no hay colisión en la nueva posición
 *
 * OPTIMIZADO: Versión simplificada inline-friendly.
 * Sistema simplificado de colisión para grid:
 * - Verifica solo pies (nivel Y) y cabeza (nivel Y + 1)
 * - Acepta coordenadas enteras
 */
bool Player::canMoveTo(int x, int y, int z, World* world) const {
    // Early return si no hay mundo
    if (!world) {
        return true;
    }

    // OPTIMIZACIÓN: Verificar pies primero (más probable que colisione)
    const Block& feetBlock = world->getBlock(x, y, z);

    // El jugador NO puede caminar donde hay un árbol
    bool isTree = (feetBlock.type == BlockType::ARBOL_SECO ||
                  feetBlock.type == BlockType::ARBOL_GRASS ||
                  feetBlock.type == BlockType::ARBOL_SANGRE);

    if (feetBlock.esSolido() && !isTree) {
        return false;  // Early return - bloque sólido normal
    }

    if (isTree) {
        return false;  // Es un árbol, no se puede mover aquí
    }

    // Verificar cabeza (jugador tiene altura ~2 bloques)
    const Block& headBlock = world->getBlock(x, y + 1, z);
    return !headBlock.esSolido();
}

/**
 * @brief Aplica gravedad discreta (caer 1 bloque)
 * @param world Puntero al mundo
 * @return true si cayó un bloque
 */
bool Player::applyDiscreteGravity(World* world) {
    if (!world) {
        return false;
    }

    // Verificar si podemos caer 1 bloque
    if (canMoveTo(m_posX, m_posY - 1, m_posZ, world)) {
        // Guardar posición visual actual
        m_prevVisualX = m_visualX;
        m_prevVisualY = m_visualY;
        m_prevVisualZ = m_visualZ;

        // Caer un bloque
        m_posY -= 1;

        // Iniciar interpolación
        m_lerpT = 0.0f;
        m_isMoving = true;

        return true;
    }

    return false;
}

/**
 * @brief Intenta saltar
 * @param world Puntero al mundo
 * @return true si el salto fue iniciado
 *
 * OPTIMIZADO: Versión simplificada para salto manual.
 * El jugador salta 1 bloque hacia arriba si hay espacio.
 */
bool Player::tryJump(World* world) {
    // Early returns para optimizar
    if (m_isJumping || m_moveCooldown > 0.0f) {
        return false;
    }

    if (!world) {
        return false;
    }

    // OPTIMIZACIÓN: Verificar bloques arriba (inline)
    const Block& block1 = world->getBlock(m_posX, m_posY + 1, m_posZ);
    if (block1.esSolido()) {
        return false;
    }

    const Block& block2 = world->getBlock(m_posX, m_posY + 2, m_posZ);
    if (block2.esSolido()) {
        return false;
    }

    // Guardar posición visual actual
    m_prevVisualX = m_visualX;
    m_prevVisualY = m_visualY;
    m_prevVisualZ = m_visualZ;

    // Iniciar salto
    m_jumpStartY = m_posY;
    m_jumpTargetY = m_posY + JUMP_HEIGHT;
    m_posY = m_jumpTargetY;

    m_jumpTimer = 0.0f;
    m_isJumping = true;
    m_lerpT = 0.0f;
    m_isMoving = true;
    m_moveCooldown = JUMP_COOLDOWN;

    return true;
}

/**
 * @brief Verifica si el jugador puede saltar
 * @param world Puntero al mundo
 * @return true si hay espacio para saltar
 *
 * OPTIMIZADO: Early returns para máxima eficiencia.
 * El jugador puede saltar si hay 2 bloques libres arriba.
 */
bool Player::canJump(World* world) const {
    // OPTIMIZACIÓN: Early returns (más probables primero)
    if (m_isJumping) {
        return false;
    }

    if (m_moveCooldown > 0.0f) {
        return false;
    }

    if (!world) {
        return false;
    }

    // Verificar que hay 2 bloques libres arriba
    const Block& block1 = world->getBlock(m_posX, m_posY + 1, m_posZ);
    if (block1.esSolido()) {
        return false;
    }

    const Block& block2 = world->getBlock(m_posX, m_posY + 2, m_posZ);
    return !block2.esSolido();
}

/**
 * @brief Actualiza la posición visual con interpolación
 * @param deltaTime Tiempo transcurrido
 *
 * Versión OPTIMIZADA:
 * - Early return si no hay movimiento
 * - Pre-calcular arco parabólico solo una vez
 * - Reducir divisiones
 */
void Player::updateVisuals(float deltaTime) {
    // OPTIMIZACIÓN: Early return si no hay movimiento
    if (m_lerpT >= 1.0f) {
        // Sin interpolación activa, mantener sincronizadas
        m_visualX = static_cast<float>(m_posX);
        m_visualY = static_cast<float>(m_posY);
        m_visualZ = static_cast<float>(m_posZ);
        return;
    }

    // Determinar duración según si es salto o movimiento normal
    float duration = m_isJumping ? JUMP_DURATION : MOVE_DURATION;

    // Avanzar interpolación
    float delta = deltaTime / duration;
    m_lerpT += delta;

    // OPTIMIZACIÓN: Clamp una sola vez
    if (m_lerpT > 1.0f) {
        m_lerpT = 1.0f;
    }

    // Calcular posición visual base interpolada
    m_visualX = lerp(m_prevVisualX, static_cast<float>(m_posX), m_lerpT);
    m_visualZ = lerp(m_prevVisualZ, static_cast<float>(m_posZ), m_lerpT);

    // Si está saltando, aplicar curva parabólica en Y
    if (m_isJumping) {
        // OPTIMIZACIÓN: Pre-calcular arco parabólico una sola vez
        // Fórmula: arc = 4 * t * (1-t)
        float arc = 4.0f * m_lerpT * (1.0f - m_lerpT);

        // Interpolación lineal base
        float baseY = lerp(m_prevVisualY, static_cast<float>(m_posY), m_lerpT);

        // Combinar
        m_visualY = baseY + arc * 0.3f;
    } else {
        // Movimiento normal, interpolación lineal
        m_visualY = lerp(m_prevVisualY, static_cast<float>(m_posY), m_lerpT);
    }

    // Verificar si terminó la interpolación
    if (m_lerpT >= 1.0f) {
        m_isMoving = false;
        m_isJumping = false;
        // Asegurar que la visual esté exactamente en la lógica
        m_visualX = static_cast<float>(m_posX);
        m_visualY = static_cast<float>(m_posY);
        m_visualZ = static_cast<float>(m_posZ);
    }
}

/**
 * @brief Verifica si hay colisión en una posición
 * @param x Posición X a verificar
 * @param y Posición Y a verificar
 * @param z Posición Z a verificar
 * @param world Puntero al mundo
 * @return true si hay colisión con un bloque sólido
 *
 * Verifica los bloques que ocuparía el jugador en la posición especificada.
 *
 * El jugador se modela como una caja con dimensiones:
 * - PLAYER_WIDTH x PLAYER_WIDTH (base)
 * - PLAYER_HEIGHT (altura)
 *
 * Se verifican los bloques en:
 * 1. Los pies del jugador (nivel Y)
 * 2. La cabeza del jugador (nivel Y + PLAYER_HEIGHT - 1)
 *
 * Para cada nivel, se verifican las 4 esquinas de la base del jugador.
 */
bool Player::checkCollision(float x, float y, float z, World* world) const {
    if (!world) {
        return false;
    }

    // Mitad del ancho del jugador para calcular las esquinas
    float halfWidth = PLAYER_WIDTH / 2.0f;

    // Coordenadas de las esquinas de la base del jugador
    float minX = x - halfWidth;
    float maxX = x + halfWidth;
    float minZ = z - halfWidth;
    float maxZ = z + halfWidth;

    // Niveles de Y a verificar (pies y cabeza)
    int feetLevel = static_cast<int>(std::floor(y));
    int headLevel = static_cast<int>(std::floor(y + PLAYER_HEIGHT - 0.01f)); // -0.01 para evitar verificar el bloque de arriba

    // Verificar los bloques en los pies del jugador
    const Block& blockFeet1 = world->getBlock(
        static_cast<int>(std::floor(minX)),
        feetLevel,
        static_cast<int>(std::floor(minZ))
    );
    if (blockFeet1.esSolido()) return true;

    const Block& blockFeet2 = world->getBlock(
        static_cast<int>(std::floor(maxX)),
        feetLevel,
        static_cast<int>(std::floor(minZ))
    );
    if (blockFeet2.esSolido()) return true;

    const Block& blockFeet3 = world->getBlock(
        static_cast<int>(std::floor(minX)),
        feetLevel,
        static_cast<int>(std::floor(maxZ))
    );
    if (blockFeet3.esSolido()) return true;

    const Block& blockFeet4 = world->getBlock(
        static_cast<int>(std::floor(maxX)),
        feetLevel,
        static_cast<int>(std::floor(maxZ))
    );
    if (blockFeet4.esSolido()) return true;

    // Verificar los bloques en la cabeza del jugador (solo si está por encima de los pies)
    if (headLevel > feetLevel) {
        const Block& blockHead1 = world->getBlock(
            static_cast<int>(std::floor(minX)),
            headLevel,
            static_cast<int>(std::floor(minZ))
        );
        if (blockHead1.esSolido()) return true;

        const Block& blockHead2 = world->getBlock(
            static_cast<int>(std::floor(maxX)),
            headLevel,
            static_cast<int>(std::floor(minZ))
        );
        if (blockHead2.esSolido()) return true;

        const Block& blockHead3 = world->getBlock(
            static_cast<int>(std::floor(minX)),
            headLevel,
            static_cast<int>(std::floor(maxZ))
        );
        if (blockHead3.esSolido()) return true;

        const Block& blockHead4 = world->getBlock(
            static_cast<int>(std::floor(maxX)),
            headLevel,
            static_cast<int>(std::floor(maxZ))
        );
        if (blockHead4.esSolido()) return true;
    }

    return false;
}
