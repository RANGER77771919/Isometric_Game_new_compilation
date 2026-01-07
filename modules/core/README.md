# Módulo Core

Sistemas centrales del juego: Game loop, World management, Chunk system.

## 📦 Contenido

### Responsabilidades:
- **Game**: Main loop, inicialización, cleanup
- **World**: Generación procedural, gestión de chunks
- **Chunk**: Almacenamiento y acceso a bloques

## 🔄 Migración desde código legacy

### Desde `src/` y `include/`:
- `src/Game.cpp` → `modules/core/src/Game.cpp`
- `src/World.cpp` → `modules/core/src/World.cpp`
- `src/Chunk.cpp` → `modules/core/src/Chunk.cpp`
- `include/Game.hpp` → `modules/core/include/core/Game.hpp`
- `include/World.hpp` → `modules/core/include/core/World.hpp`
- `include/Chunk.hpp` → `modules/core/include/core/Chunk.hpp`

## 📋 Tareas Pendientes

- [ ] Mover Game.cpp/hpp a `modules/core/`
- [ ] Mover World.cpp/hpp a `modules/core/`
- [ ] Mover Chunk.cpp/hpp a `modules/core/`
- [ ] Actualizar includes en el código
- [ ] Agregar tests unitarios

## 🔗 Dependencias

- **STL** (standard library)
- **FastNoiseLite** (generación de terreno)
- **Futuro**: módulo ECS para entidades

## 📝 Notas

- Mantiene la lógica actual de generación procedural
- No tiene dependencias de gráficos (rendering)
- Diseñado para ser thread-safe en el futuro
