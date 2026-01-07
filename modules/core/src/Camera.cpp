/**
 * @file Camera.cpp
 * @brief Implementación de la clase Camera para proyección isométrica
 *
 * Este archivo contiene la implementación de los métodos de la clase Camera,
 * que maneja la proyección isométrica de coordenadas del mundo 3D a pantalla 2D.
 */

#include "core/Camera.hpp"
#include <cmath>

/**
 * @brief Constructor de la cámara
 *
 * Inicializa todos los miembros de la cámara con valores por defecto:
 * - Posición: (0, 0, 0) - origen del mundo
 * - Zoom: 1.0 - sin ampliación ni reducción
 * - Centro: (400, 300) - centro de una ventana 800x600 típica
 *
 * La posición Y se establece en 0 (a nivel del suelo) en lugar de 100,
 * para evitar offsets de renderizado incorrectos.
 */
Camera::Camera()
    : m_posX(0.0f)
    , m_posY(0.0f)  // Cambiado de 100.0f a 0.0f para estar a nivel del suelo
    , m_posZ(0.0f)
    , m_zoom(1.0f)
    , m_centerX(400.0f)
    , m_centerY(300.0f)
    , m_cache{IsoConfig::TILE_WIDTH * 0.5f, IsoConfig::TILE_HEIGHT * 0.5f,
              IsoConfig::BLOCK_HEIGHT, 1.0f}  // Inicializar cache con zoom=1.0
{
}

/**
 * @brief Establece la posición de la cámara en el mundo
 * @param x Nueva coordenada X
 * @param y Nueva coordenada Y (altura)
 * @param z Nueva coordenada Z
 *
 * Actualiza la posición de la cámara. Se llama cada frame con la posición
 * del jugador para que la cámara lo siga.
 */
void Camera::setPosition(float x, float y, float z) {
    m_posX = x;
    m_posY = y;
    m_posZ = z;
}

/**
 * @brief Obtiene la posición actual de la cámara
 * @param x Variable de salida para la coordenada X
 * @param y Variable de salida para la coordenada Y (altura)
 * @param z Variable de salida para la coordenada Z
 *
 * Las coordenadas se obtienen pasándolas por referencia.
 * El método es const ya que no modifica el estado de la cámara.
 */
void Camera::getPosition(float& x, float& y, float& z) const {
    x = m_posX;
    y = m_posY;
    z = m_posZ;
}

/**
 * @brief Mueve la cámara relativamente
 * @param dx Delta de movimiento en X
 * @param dy Delta de movimiento en Y (altura)
 * @param dz Delta de movimiento en Z
 *
 * Los deltas se suman a la posición actual de la cámara.
 * Útil para movimiento libre de la cámara (no usado actualmente,
 * ya que la cámara sigue al jugador).
 */
void Camera::move(float dx, float dy, float dz) {
    m_posX += dx;
    m_posY += dy;
    m_posZ += dz;
}

/**
 * @brief Establece el nivel de zoom de la cámara
 * @param zoom Nuevo nivel de zoom
 *
 * El zoom se clampea al rango [0.1, 5.0] para evitar:
 * - zoom <= 0: invertiría la imagen
 * - zoom muy pequeño: veríamos muy poco
 * - zoom muy grande: pérdida de rendimiento y artefactos visuales
 */
void Camera::setZoom(float zoom) {
    m_zoom = zoom;
    if (m_zoom < 0.1f) m_zoom = 0.1f;
    if (m_zoom > 5.0f) m_zoom = 5.0f;
}

/**
 * @brief Convierte coordenadas del mundo 3D a coordenadas de pantalla 2D
 * @param worldX Coordenada X del mundo a convertir
 * @param worldY Coordenada Y del mundo (altura) a convertir
 * @param worldZ Coordenada Z del mundo a convertir
 * @param screenX Variable de salida para la coordenada X de pantalla (píxeles)
 * @param screenY Variable de salida para la coordenada Y de pantalla (píxeles)
 *
 * Algoritmo de proyección isométrica:
 *
 * 1. Calcular posición relativa (mundo -> cámara):
 *    - relX = worldX - m_posX
 *    - relZ = worldZ - m_posZ
 *    - NOTA: NO restamos m_posY porque la altura se maneja diferente
 *
 * 2. Aplicar proyección isométrica:
 *    - isoX = (relX - relZ) * TILE_WIDTH / 2
 *    - isoY = (relX + relZ) * TILE_HEIGHT / 2 + worldY * BLOCK_HEIGHT
 *
 *    La fórmula crea la vista diagonal característica del isométrico:
 *    - (relX - relZ) crea la diagonal horizontal
 *    - (relX + relZ) crea la profundidad
 *    - worldY * BLOCK_HEIGHT empuja hacia ABAJO los bloques más altos (signo +)
 *
 * 3. Aplicar zoom:
 *    - isoX *= m_zoom
 *    - isoY *= m_zoom
 *
 * 4. Ajustar el centro vertical según la altura de la cámara:
 *    - centerYAdjusted = m_centerY - (m_posY * BLOCK_HEIGHT * zoom)
 *
 *    Esto es CRÍTICO: cuando la cámara está en una altura Y (ej: 81),
 *    todos los bloques se empujan hacia abajo por la fórmula isométrica.
 *    Ajustamos el centro hacia arriba para compensar y mantener al jugador centrado.
 *
 * 5. Centrar en pantalla:
 *    - screenX = isoX + m_centerX
 *    - screenY = isoY + centerYAdjusted
 *
 * Ejemplo práctico:
 * - Jugador en (0, 81, 0), cámara en (0, 81, 0)
 * - relX = 0, relZ = 0
 * - isoX = 0, isoY = 0 + 81*32 = 2592
 * - centerYAdjusted = 360 - 81*32*1 = -2232
 * - screenX = 0 + 640 = 640
 * - screenY = 2592 + (-2232) = 360 ✓ (centrado!)
 */
void Camera::worldToScreen(float worldX, float worldY, float worldZ,
                            float& screenX, float& screenY) const {
    // OPTIMIZACIÓN: Actualizar cache si el zoom cambió (dirty flag)
    if (m_cache.cachedZoom != m_zoom) {
        m_cache.tileWidthHalf = IsoConfig::TILE_WIDTH * 0.5f * m_zoom;
        m_cache.tileHeightHalf = IsoConfig::TILE_HEIGHT * 0.5f * m_zoom;
        m_cache.blockHeight = IsoConfig::BLOCK_HEIGHT * m_zoom;
        m_cache.cachedZoom = m_zoom;
    }

    // Proyección isométrica
    // x_screen = (x - z) * tile_width / 2
    // y_screen = (x + z) * tile_height / 2 - y * block_height

    // Calcular posición relativa (mundo -> camera)
    float relX = worldX - m_posX;
    float relZ = worldZ - m_posZ;
    float relY = worldY - m_posY;  // Altura relativa a la cámara

    // OPTIMIZACIÓN: Usar valores cacheados en lugar de recalcular
    float isoX = (relX - relZ) * m_cache.tileWidthHalf;
    float isoY = (relX + relZ) * m_cache.tileHeightHalf - relY * m_cache.blockHeight;

    // Centrar en pantalla (el zoom ya está aplicado en los valores cacheados)
    screenX = isoX + m_centerX;
    screenY = isoY + m_centerY;
}

/**
 * @brief Convierte coordenadas de pantalla 2D a coordenadas del mundo 3D
 * @param screenX Coordenada X de pantalla (píxeles) a convertir
 * @param screenY Coordenada Y de pantalla (píxeles) a convertir
 * @param worldX Variable de salida para la coordenada X del mundo
 * @param worldZ Variable de salida para la coordenada Z del mundo
 * @param worldY Altura asumida para la conversión (por defecto 0)
 *
 * Operación inversa de worldToScreen(). Útil para detectar clics del ratón.
 *
 * Algoritmo:
 *
 * 1. Ajustar el centro vertical según la altura de la cámara:
 *    - centerYAdjusted = m_centerY + (m_posY * BLOCK_HEIGHT * zoom)
 *
 * 2. Quitar centro de pantalla:
 *    - isoX = screenX - m_centerX
 *    - isoY = screenY - centerYAdjusted
 *
 * 3. Deshacer zoom:
 *    - isoX /= m_zoom
 *    - isoY /= m_zoom
 *
 * 4. Aplicar ajuste de altura:
 *    - isoY -= worldY * BLOCK_HEIGHT
 *
 * 5. Resolver sistema de ecuaciones isométrico:
 *    Dado:
 *      isoX = (x - z) * tile_width / 2  ... (1)
 *      isoY = (x + z) * tile_height / 2  ... (2)
 *
 *    Despejamos:
 *      x = (isoX / tile_width + isoY / tile_height) / 2
 *      z = (isoY / tile_height - isoX / tile_width) / 2
 *
 * 6. Sumar posición de la cámara:
 *    - worldX = m_posX + relX
 *    - worldZ = m_posZ + relZ
 *
 * Nota sobre worldY:
 * La proyección isométrica no es reversible sin conocer worldY porque
 * múltiples posiciones (x, y, z) pueden proyectarse al mismo punto de pantalla.
 * Por eso el parámetro worldY es obligatorio para esta operación.
 */
void Camera::screenToWorld(float screenX, float screenY,
                            float& worldX, float& worldZ, float worldY) const {
    // Quitar centro de pantalla
    float isoX = screenX - m_centerX;
    float isoY = screenY - m_centerY;

    // Deshacer zoom
    isoX /= m_zoom;
    isoY /= m_zoom;

    // Deshacer proyección isométrica
    // Sistema de ecuaciones:
    // isoX = (x - z) * tile_width / 2
    // isoY = (x + z) * tile_height / 2 - (y - cameraY) * block_height

    float yAdjustment = (worldY - m_posY) * IsoConfig::BLOCK_HEIGHT;
    isoY += yAdjustment;

    float tileWidthHalf = IsoConfig::TILE_WIDTH * 0.5f;
    float tileHeightHalf = IsoConfig::TILE_HEIGHT * 0.5f;

    // Resolver sistema:
    // x = (isoX / tile_width + isoY / tile_height) / 2
    // z = (isoY / tile_height - isoX / tile_width) / 2

    float relX = (isoX / tileWidthHalf + isoY / tileHeightHalf) * 0.5f;
    float relZ = (isoY / tileHeightHalf - isoX / tileWidthHalf) * 0.5f;

    // Invertir la posición relativa para obtener coordenadas mundiales
    worldX = m_posX + relX;
    worldZ = m_posZ + relZ;
}

/**
 * @brief Actualiza el centro de la cámara
 * @param centerX Nueva coordenada X del centro (píxeles)
 * @param centerY Nueva coordenada Y del centro (píxeles)
 *
 * Se llama cuando la ventana se redimensiona para mantener el contenido
 * centrado. La función SDL_GetWindowSize() obtiene el nuevo tamaño y
 * se divide por 2 para obtener el centro.
 */
void Camera::setCenter(float centerX, float centerY) {
    m_centerX = centerX;
    m_centerY = centerY;
}
