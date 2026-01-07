/**
 * @file Camera.hpp
 * @brief Define la clase Camera para proyección isométrica
 *
 * Este archivo contiene la definición de la cámara isométrica y las
 * constantes de configuración para la proyección de mundo a pantalla.
 */

#pragma once

#include "Block.hpp"
#include <SDL2/SDL.h>

/**
 * @struct IsoConfig
 * @brief Configuración de la vista isométrica
 *
 * Define las constantes utilizadas para la proyección isométrica:
 * - TILE_WIDTH: Ancho de cada tile en píxeles (horizontal)
 * - TILE_HEIGHT: Alto de cada tile en píxeles (profundidad)
 * - BLOCK_HEIGHT: Altura vertical de cada bloque en píxeles
 *
 * La proyección isométrica usa estas constantes para convertir coordenadas
 * del mundo 3D (x, y, z) a coordenadas de pantalla 2D (screenX, screenY).
 *
 * Fórmulas de proyección:
 * - screenX = (worldX - worldZ) * TILE_WIDTH / 2
 * - screenY = (worldX + worldZ) * TILE_HEIGHT / 2 - worldY * BLOCK_HEIGHT
 */
struct IsoConfig {
    static constexpr float TILE_WIDTH = 32.0f;      ///< Ancho del tile en pantalla (píxeles)
    static constexpr float TILE_HEIGHT = 16.0f;     ///< Alto del tile en pantalla (píxeles)
    static constexpr float BLOCK_HEIGHT = 16.0f;    ///< Altura vertical de cada bloque (píxeles)
};

/**
 * @class Camera
 * @brief Cámara para vista isométrica con zoom y paneo
 *
 * La cámara representa el punto de vista del jugador en el mundo isométrico.
 * Mantiene una posición en el mundo 3D y proyecta las coordenadas a pantalla 2D.
 *
 * Funcionalidades:
 * - Posicionamiento en coordenadas mundiales (x, y, z)
 * - Zoom con rango limitado [0.1, 5.0]
 * - Conversión bidireccional mundo <-> pantalla
 * - Ajuste del centro de la pantalla para redimensionamiento de ventana
 *
 * La cámara sigue al jugador durante el juego, actualizando su posición
 * para mantener al jugador centrado en pantalla.
 */
class Camera {
public:
    /**
     * @brief Constructor de la cámara
     *
     * Inicializa la cámara en el origen del mundo (0, 0, 0) con:
     * - Zoom: 1.0x (sin zoom)
     * - Centro: (400, 300) - resolución típica 800x600
     * - Posición: (0, 0, 0) - origen del mundo
     */
    Camera();

    /**
     * @brief Destructor por defecto
     *
     * No requiere limpieza de recursos adicionales.
     */
    ~Camera() = default;

    /**
     * @brief Establece la posición de la cámara en el mundo
     * @param x Coordenada X de la cámara (posición horizontal este-oeste)
     * @param y Coordenada Y de la cámara (altura/vertical)
     * @param z Coordenada Z de la cámara (posición horizontal norte-sur)
     *
     * La cámara apuntará a esta posición del mundo. Cuando la cámara sigue
     * al jugador, este método se llama cada frame con la posición del jugador.
     */
    void setPosition(float x, float y, float z);

    /**
     * @brief Obtiene la posición actual de la cámara en el mundo
     * @param x Variable de salida para la coordenada X
     * @param y Variable de salida para la coordenada Y (altura)
     * @param z Variable de salida para la coordenada Z
     *
     * Las coordenadas se pasan por referencia y son modificadas con los
     * valores actuales de la cámara.
     */
    void getPosition(float& x, float& y, float& z) const;

    /**
     * @brief Mueve la cámara relativamente desde su posición actual
     * @param dx Delta de movimiento en el eje X
     * @param dy Delta de movimiento en el eje Y (altura)
     * @param dz Delta de movimiento en el eje Z
     *
     * Los deltas se suman a la posición actual. Útil para movimiento
     * continuo de la cámara basado en input del usuario.
     */
    void move(float dx, float dy, float dz);

    /**
     * @brief Establece el nivel de zoom de la cámara
     * @param zoom Nuevo nivel de zoom (clampeado a [0.1, 5.0])
     *
     * El zoom se aplica a toda la proyección isométrica.
     * - zoom < 1.0: aleja la vista (reduce todo)
     * - zoom = 1.0: zoom normal (1:1)
     * - zoom > 1.0: acerca la vista (amplía todo)
     *
     * Los valores fuera de rango se clampean a los límites.
     */
    void setZoom(float zoom);

    /**
     * @brief Obtiene el nivel de zoom actual
     * @return Nivel de zoom actual (rango [0.1, 5.0])
     */
    float getZoom() const { return m_zoom; }

    /**
     * @brief Convierte coordenadas del mundo 3D a coordenadas de pantalla 2D
     * @param worldX Coordenada X del mundo
     * @param worldY Coordenada Y del mundo (altura)
     * @param worldZ Coordenada Z del mundo
     * @param screenX Variable de salida para la coordenada X de pantalla
     * @param screenY Variable de salida para la coordenada Y de pantalla
     *
     * Implementa la proyección isométrica:
     * 1. Resta la posición de la cámara (solo X y Z, Y se maneja diferente)
     * 2. Aplica fórmula isométrica:
     *    - isoX = (relX - relZ) * TILE_WIDTH / 2
     *    - isoY = (relX + relZ) * TILE_HEIGHT / 2 - worldY * BLOCK_HEIGHT
     * 3. Aplica zoom
     * 4. Ajusta el centro vertical según la altura de la cámara
     * 5. Centra en pantalla
     *
     * Esta es la operación fundamental para renderizar el mundo isométrico.
     */
    void worldToScreen(float worldX, float worldY, float worldZ,
                       float& screenX, float& screenY) const;

    /**
     * @brief Convierte coordenadas de pantalla 2D a coordenadas del mundo 3D
     * @param screenX Coordenada X de pantalla (píxeles)
     * @param screenY Coordenada Y de pantalla (píxeles)
     * @param worldX Variable de salida para la coordenada X del mundo
     * @param worldZ Variable de salida para la coordenada Z del mundo
     * @param worldY Altura asumida para la conversión (por defecto 0)
     *
     * Operación inversa de worldToScreen():
     * 1. Quita el centro de pantalla (ajustado por altura de cámara)
     * 2. Deshace el zoom
     * 3. Aplica el ajuste de altura (worldY * BLOCK_HEIGHT)
     * 4. Resuelve el sistema de ecuaciones isométrico
     * 5. Suma la posición de la cámara
     *
     * Útil para detectar en qué bloque hizo clic el usuario.
     * Nota: La coordenada Y debe especificarse ya que múltiples alturas
     * pueden proyectarse al mismo punto de pantalla.
     */
    void screenToWorld(float screenX, float screenY,
                       float& worldX, float& worldZ, float worldY = 0) const;

    /**
     * @brief Obtiene la coordenada X del centro de pantalla
     * @return Coordenada X del centro en píxeles
     */
    float getCenterX() const { return m_centerX; }

    /**
     * @brief Obtiene la coordenada Y del centro de pantalla
     * @return Coordenada Y del centro en píxeles
     */
    float getCenterY() const { return m_centerY; }

    /**
     * @brief Actualiza el centro de la cámara
     * @param centerX Nueva coordenada X del centro
     * @param centerY Nueva coordenada Y del centro
     *
     * Se llama cuando la ventana se redimensiona para mantener el contenido
     * centrado. Por ejemplo, si la ventana pasa de 800x600 a 1280x720,
     * el centro cambia de (400, 300) a (640, 360).
     */
    void setCenter(float centerX, float centerY);

private:
    float m_posX;      ///< Coordenada X de la cámara en el mundo
    float m_posY;      ///< Coordenada Y de la cámara (altura)
    float m_posZ;      ///< Coordenada Z de la cámara en el mundo
    float m_zoom;      ///< Nivel de zoom actual (rango [0.1, 5.0])
    float m_centerX;   ///< Coordenada X del centro de pantalla (píxeles)
    float m_centerY;   ///< Coordenada Y del centro de pantalla (píxeles)

    // OPTIMIZACIÓN: Cache de constantes de proyección (5-10% FPS)
    struct CameraCache {
        float tileWidthHalf;   ///< TILE_WIDTH * 0.5 * zoom (cache)
        float tileHeightHalf;  ///< TILE_HEIGHT * 0.5 * zoom (cache)
        float blockHeight;     ///< BLOCK_HEIGHT * zoom (cache)
        float cachedZoom;      ///< Zoom usado para el cache (dirty flag)
    };
    mutable CameraCache m_cache;  ///< Cache de valores pre-calculados
};
