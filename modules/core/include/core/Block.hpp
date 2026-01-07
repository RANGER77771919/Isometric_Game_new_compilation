/**
 * @file Block.hpp
 * @brief Define los tipos de bloques y la estructura Block del mundo
 *
 * Este archivo contiene la enumeración de tipos de bloques, la estructura Block
 * que representa un bloque individual, y las constantes de configuración global
 * para el sistema de bloques del mundo.
 */

#pragma once

#include <cstdint>
#include <array>

/**
 * @enum BlockType
 * @brief Tipos de bloques disponibles en el juego
 *
 * Enumeración fuerte (enum class) que define todos los tipos posibles de bloques
 * que pueden existir en el mundo. Cada tipo tiene una textura asociada y
 * propiedades específicas (sólido/transparente).
 *
 * Tipos de bloques:
 * - AIRE: Bloque vacío, no se renderiza, usado para rellenar espacios vacíos
 * - PASTO: Bloque de pasto verde, aparece en la superficie del terreno
 * - TIERRA: Bloque de tierra marrón, aparece debajo del pasto
 * - PIEDRA: Bloque de piedra gris, aparece en profundidades mayores
 * - ARENA: Bloque de arena amarilla, aparece en biomas de desierto
 * - NIEVE: Bloque de nieve blanca, aparece en biomas fríos o altura
 * - AGUA: Bloque de agua azul, semitransparente, aparece en lagos y ríos
 * - HIERBA_SANGRE: Pasto con hierba roja, variante decorativa
 * - PASTO_FULL: Pasto denso, variante decorativa
 * - TOTAL_TIPOS: Marcador final, NO es un tipo de bloque válido
 *
 * Cada tipo de bloque tiene una textura correspondiente en assets/tiles/.
 */
enum class BlockType : uint8_t {
    AIRE = 0,        ///< Bloque vacío (no se renderiza)
    PASTO,           ///< Pasto en la superficie
    HIERBA_SANGRE,   ///< Pasto con hierba roja
    ARENA,           ///< Arena
    PIEDRA,          ///< Piedra (usa textura de black dirt)
    TIERRA,          ///< Tierra debajo del pasto
    DIRT_ALT,        ///< Variante de tierra
    PASTO_FULL,      ///< Pasto denso
    NIEVE,           ///< Nieve
    AGUA,            ///< Agua
    ARBOL_SECO,      ///< Árbol seco para biomas secos (dirt/sand)
    ARBOL_GRASS,     ///< Árbol vivo para biomas de grass
    ARBOL_SANGRE,    ///< Árbol de sangre para biomas de blood_grass
    TOTAL_TIPOS      ///< Total de tipos de bloques (marcador, no usado)
};

/**
 * @struct Block
 * @brief Información de un bloque en el mundo
 *
 * Estructura ligera que representa un bloque individual en el mundo.
 * Cada bloque tiene un tipo y métodos para consultar sus propiedades.
 *
 * Uso de memoria:
 * - sizeof(Block) = 1 byte (solo el tipo, uint8_t)
 * - Un chunk completo (8x8x256) ocupa 8*8*256*1 = 16,384 bytes (~16 KB)
 *
 * Nota sobre optimización:
 * El struct es deliberadamente simple (POD - Plain Old Data) para permitir
 * almacenamiento eficiente en memoria contigua. Los chunks almacenan miles
 * de estos bloques, por lo que el tamaño mínimo es crítico.
 */
struct Block {
    BlockType type = BlockType::AIRE; ///< Tipo del bloque, por defecto AIRE

    /**
     * @brief Constructor por defecto
     *
     * Inicializa el bloque como AIRE usando el inicializador de miembro por defecto.
     * No realiza operaciones adicionales para maximizar rendimiento.
     */
    Block() = default;

    /**
     * @brief Constructor con tipo específico
     * @param t Tipo de bloque a crear
     *
     * Permite crear bloques directamente con un tipo específico:
     * @code
     * Block bloquePasto(BlockType::PASTO);
     * Block bloqueAire; // Ya es AIRE por defecto
     * @endcode
     */
    Block(BlockType t) : type(t) {}

    /**
     * @brief Verifica si el bloque es sólido
     * @return true si el bloque NO es aire, false en caso contrario
     *
     * OPTIMIZACIÓN: Inline y constexpr para mejor rendimiento
     * Un bloque sólido es aquel que ocupa espacio y puede colisionar.
     * El único bloque no sólido es el AIRE.
     *
     * Usos típicos:
     * - Renderizado: solo se renderizan bloques sólidos
     * - Colisiones: el jugador puede atravesar bloques no sólidos
     * - Generación de terreno: encontrar la superficie (primer bloque sólido desde arriba)
     */
    inline bool esSolido() const {
        return type != BlockType::AIRE;
    }

    /**
     * @brief Verifica si el bloque es transparente
     * @return true si el bloque es aire o agua, false en caso contrario
     *
     * OPTIMIZACIÓN: Inline para mejor rendimiento
     * Un bloque transparente permite ver a través de él.
     * Actualmente solo el AIRE y el AGUA son transparentes.
     *
     * Usos típicos:
     * - Renderizado: los bloques transparentes no ocultan los que están detrás
     * - Optimización: no renderizar bloques ocultos detrás de sólidos
     * - Iluminación: la luz pasa a través de bloques transparentes
     *
     * Nota: En una implementación más avanzada, podrías distinguir entre:
     * - Transparente: deja pasar luz (agua, cristal)
     * - No sólido: se puede atravesar (aire, agua, hierba)
     */
    inline bool esTransparente() const {
        return type == BlockType::AIRE || type == BlockType::AGUA;
    }
};

/**
 * @namespace BlockConfig
 * @brief Configuración global de los bloques
 *
 * Contiene constantes que definen las dimensiones y límites del sistema
 * de bloques del mundo. Estas constantes afectan la generación de terreno,
 * renderizado, y gestión de memoria.
 */
namespace BlockConfig {
    constexpr int TILE_SIZE = 32;    ///< Tamaño de cada tile en píxeles (ancho de textura)
    constexpr int CHUNK_SIZE = 8;    ///< Tamaño del chunk en bloques (8x8x32 bloques)
    constexpr int WORLD_HEIGHT = 32;  ///< OPTIMIZACIÓN FASE 2: Altura máxima reducida de 256 a 32 bloques (de Y=0 a Y=31)

    /**
     * @brief Cálculo de memoria del mundo
     *
     * OPTIMIZACIÓN FASE 2: WORLD_HEIGHT reducido de 256 a 32 (8× menos altura)
     *
     * Un chunk completo contiene:
     * - CHUNK_SIZE * CHUNK_SIZE * WORLD_HEIGHT bloques
     * - 8 * 8 * 32 = 2,048 bloques (antes: 16,384)
     * - 2,048 bytes (~2 KB) por chunk (antes: ~16 KB)
     *
     * Con LOAD_RADIUS = 6 (13x13 = 169 chunks en memoria):
     * - 169 chunks * 2 KB = ~338 KB (con sparse storage: ~200-250 KB real)
     *
     * **Reducción de memoria: ~87.5%** (de 2.7 MB a ~338 KB)
     *
     * CHUNK_SIZE=8 es óptimo para balance entre overhead de gestión
     * y costo de generar/renderizar cada chunk individual.
     */
}
