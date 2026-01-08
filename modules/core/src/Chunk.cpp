/**
 * @file Chunk.cpp
 * @brief Implementación de la clase Chunk
 *
 * Este archivo contiene la implementación de los métodos de la clase Chunk,
 * que gestiona secciones del mundo de 8x8x256 bloques.
 */

#include "core/Chunk.hpp"
#include <algorithm>

/**
 * @brief Constructor del chunk
 * @param position Posición del chunk en el mundo (coordenadas de chunk)
 *
 * Inicializa el chunk con estructura sparse de datos.
 *
 * OPTIMIZACIÓN 8: SPARSE DATA STRUCTURES + FASE 2: WORLD_HEIGHT=32
 * - Ya no reserva 2,048 bloques (solo los sólidos)
 * - Memoria inicial: 0 bloques
 * - Memoria típica: ~600-800 bloques (~70% menos que denso)
 *
 * Proceso de inicialización:
 * 1. Guardar la posición del chunk
 * 2. Marcar como "no generado" (m_generated = false)
 * 3. El mapa sparse está vacío inicialmente
 */
Chunk::Chunk(ChunkPos position)
    : m_position(position)
    , m_generated(false)
{
    // OPTIMIZACIÓN 8 + FASE 2: No reservar memoria upfront
    // Los bloques se agregan solo cuando son sólidos
    // Capacity inicial del unordered_map para WORLD_HEIGHT=32
    m_blocks.reserve(800);  // ~40% de 2,048 bloques son sólidos (ajustado)
}

/**
 * @brief Obtiene un bloque en coordenadas locales (versión modificable)
 * @param x Coordenada X local [0, CHUNK_SIZE-1]
 * @param y Coordenada Y (altura) [0, WORLD_HEIGHT-1]
 * @param z Coordenada Z local [0, CHUNK_SIZE-1]
 * @return Referencia al bloque (permite modificación)
 *
 * Valida las coordenadas antes de acceder al array.
 * Si están fuera de rango, devuelve un bloque estático inválido.
 *
 * Validación:
 * - x debe estar en [0, CHUNK_SIZE-1] = [0, 7]
 * - y debe estar en [0, WORLD_HEIGHT-1] = [0, 255]
 * - z debe estar en [0, CHUNK_SIZE-1] = [0, 7]
 *
 * Bloque inválido:
 * - Es una variable estática local (persiste entre llamadas)
 * - Siempre es AIRE
 * - Evita crashes por accesos fuera de rango
 *
 * Nota de diseño:
 * - Se podría lanzar una excepción en lugar de devolver un inválido
 * - Pero en un motor de juego es preferible fallar silenciosamente
 * - para evitar interrupciones durante el gameplay
 */
Block& Chunk::getBlock(int x, int y, int z) {
    // Validar coordenadas locales
    if (x < 0 || x >= BlockConfig::CHUNK_SIZE ||
        z < 0 || z >= BlockConfig::CHUNK_SIZE ||
        y < 0 || y >= BlockConfig::WORLD_HEIGHT) {
        // Devolver bloque estático inválido (evita crash)
        // thread_local asegura que cada thread tenga su propia instancia (thread-safe)
        thread_local Block invalidBlock(BlockType::AIRE);
        return invalidBlock;
    }

    // OPTIMIZACIÓN 8: Buscar en mapa sparse O(1)
    size_t index = getIndex(x, y, z);
    auto it = m_blocks.find(index);

    if (it != m_blocks.end()) {
        return it->second;
    }

    // Bloque no existe = AIRE (por defecto)
    // thread_local asegura que cada thread tenga su propia instancia (thread-safe)
    thread_local Block airBlock(BlockType::AIRE);
    return airBlock;
}

/**
 * @brief Obtiene un bloque en coordenadas locales (versión constante)
 * @param x Coordenada X local [0, CHUNK_SIZE-1]
 * @param y Coordenada Y (altura) [0, WORLD_HEIGHT-1]
 * @param z Coordenada Z local [0, CHUNK_SIZE-1]
 * @return Referencia constante al bloque (no permite modificación)
 *
 * Versión const de getBlock() para chunks constantes.
 * Tiene la misma lógica de validación y acceso.
 */
const Block& Chunk::getBlock(int x, int y, int z) const {
    // Validar coordenadas locales
    if (x < 0 || x >= BlockConfig::CHUNK_SIZE ||
        z < 0 || z >= BlockConfig::CHUNK_SIZE ||
        y < 0 || y >= BlockConfig::WORLD_HEIGHT) {
        // Devolver bloque estático inválido (evita crash)
        // thread_local asegura que cada thread tenga su propia instancia (thread-safe)
        thread_local Block invalidBlock(BlockType::AIRE);
        return invalidBlock;
    }

    // OPTIMIZACIÓN 8: Buscar en mapa sparse O(1)
    size_t index = getIndex(x, y, z);
    auto it = m_blocks.find(index);

    if (it != m_blocks.end()) {
        return it->second;
    }

    // Bloque no existe = AIRE (por defecto)
    // thread_local asegura que cada thread tenga su propia instancia (thread-safe)
    thread_local Block airBlock(BlockType::AIRE);
    return airBlock;
}

/**
 * @brief Establece un bloque en coordenadas locales
 * @param x Coordenada X local [0, CHUNK_SIZE-1]
 * @param y Coordenada Y (altura) [0, WORLD_HEIGHT-1]
 * @param z Coordenada Z local [0, CHUNK_SIZE-1]
 * @param type Tipo de bloque a establecer
 *
 * OPTIMIZACIÓN 8: SPARSE STORAGE
 * - Si type == AIRE, elimina del mapa (libera memoria)
 * - Si type != AIRE, inserta/actualiza en el mapa
 * - No desperdicia memoria almacenando bloques aire
 *
 * Usos típicos:
 * - Generación procedural de terreno
 * - Modificación del terreno por el jugador (colocar bloques)
 * - Actualizaciones del mundo (cavar, construir)
 */
void Chunk::setBlock(int x, int y, int z, BlockType type) {
    // Validar coordenadas locales
    if (x < 0 || x >= BlockConfig::CHUNK_SIZE ||
        z < 0 || z >= BlockConfig::CHUNK_SIZE ||
        y < 0 || y >= BlockConfig::WORLD_HEIGHT) {
        // Coordenadas inválidas, ignorar operación
        return;
    }

    size_t index = getIndex(x, y, z);

    // OPTIMIZACIÓN 8: No almacenar bloques aire
    if (type == BlockType::AIRE) {
        m_blocks.erase(index);  // Eliminar si existe
    } else {
        m_blocks[index] = Block(type);  // Insertar o actualizar
    }
}

/**
 * @brief Calcula el índice en el array 1D para coordenadas 3D
 * @param x Coordenada X local
 * @param y Coordenada Y (altura)
 * @param z Coordenada Z local
 * @return Índice en el array m_blocks [0, 2047]
 *
 * OPTIMIZACIÓN FASE 2: WORLD_HEIGHT reducido de 256 a 32
 *
 * Fórmula de conversión:
 * index = x + (z * CHUNK_SIZE) + (y * CHUNK_SIZE * CHUNK_SIZE)
 *
 * Desglose de la fórmula:
 * - x: desplazamiento dentro de una fila
 * - z * CHUNK_SIZE: salta a la fila correcta
 * - y * CHUNK_SIZE * CHUNK_SIZE: salta a la capa de altura correcta
 *
 * Layout de memoria (con CHUNK_SIZE=8, WORLD_HEIGHT=32):
 * - Índices 0-7: fila Z=0, capa Y=0
 * - Índices 8-15: fila Z=1, capa Y=0
 * - Índices 56-63: última fila Z=7, capa Y=0
 * - Índices 64-71: fila Z=0, capa Y=1
 * - Índices 1984-2047: última fila Z=7, última capa Y=31
 *
 * Razonamiento del orden (x -> z -> y):
 * - Mejor cache locality cuando se accede en orden X
 * - Los bloques adyacentes en X tienen índices adyacentes
 * - Los bloques en la misma columna vertical (X, Z fijos) están
 *   separados por CHUNK_SIZE * CHUNK_SIZE = 64 posiciones
 *
 * Optimización:
 * - Las multiplicaciones pueden precalcularse como constantes:
 *   CHUNK_SIZE = 8
 *   CHUNK_SIZE_SQUARED = 64
 * - El compilador probablemente ya hace esta optimización
 */
int Chunk::getIndex(int x, int y, int z) const {
    // Índice en array 1D para coordenadas 3D
    // Orden: x -> z -> y (para mejor cache locality)
    return x + (z * BlockConfig::CHUNK_SIZE) + (y * BlockConfig::CHUNK_SIZE * BlockConfig::CHUNK_SIZE);
}
