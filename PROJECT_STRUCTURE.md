# Estructura del Proyecto Isometrico2D

Estructura completa de directorios del proyecto con migración a arquitectura modular.

## 📂 Árbol de Directorios

```
Isometric-Game-main/
│
├── 📁 assets/                          # Assets del juego
│   └── tiles/                          # Texturas de tiles
│
├── 📁 build/                           # Binarios compilados
│   ├── Isometrico2D.exe                # Ejecutable principal
│   └── test_bgfx_sdl2.exe              # Test de bgfx
│
├── 📁 docs/                            # 📖 Documentación
│   ├── architecture/                   # Diagramas de arquitectura
│   ├── api/                            # Documentación de APIs
│   ├── tutorials/                      # Tutoriales y guías
│   ├── design/                         # Game design docs
│   └── README.md                       # Guía de documentación
│
├── 📁 examples/                        # 💡 Ejemplos de código
│   ├── hello_world/                    # bgfx + SDL2 mínimo
│   ├── ecs_demo/                       # Demo de ECS
│   ├── particle_demo/                  # Demo de partículas
│   └── README.md
│
├── 📁 include/                         # 📄 Headers LEGACY (código actual)
│   ├── Block.hpp                       # Tipos de bloques
│   ├── Camera.hpp                      # Cámara isométrica
│   ├── Chunk.hpp                       # Sección del mundo
│   ├── Game.hpp                        # Game loop
│   ├── Player.hpp                      # Jugador
│   ├── Renderer.hpp                    # Renderer SDL2 (actual)
│   └── World.hpp                       # Mundo procedural
│
├── 📁 src/                             # 💻 Source LEGACY (código actual)
│   ├── Camera.cpp
│   ├── Chunk.cpp
│   ├── Game.cpp
│   ├── main.cpp
│   ├── Player.cpp
│   ├── Renderer.cpp                    # Renderer SDL2 (actual)
│   └── World.cpp
│
├── 📁 libs/                            # 📚 Bibliotecas externas
│   ├── bgfx/                           # Render engine
│   │   ├── .build/win64_mingw-gcc/bin/
│   │   │   └── libbgfxRelease.a        # ✅ Compilado
│   │   └── include/
│   ├── bx/                             # Math utilities
│   │   ├── .build/win64_mingw-gcc/bin/
│   │   │   └── libbxRelease.a          # ✅ Compilado
│   │   └── include/
│   ├── bimg/                           # Image processing
│   │   ├── .build/win64_mingw-gcc/bin/
│   │   │   └── libbimgRelease.a        # ✅ Compilado
│   │   └── include/
│   ├── FastNoiseLite/                  # Generación de terreno
│   └── entt/                           # ⏳ ECS (por agregar)
│
├── 📁 modules/                         # 🧩 NUEVA ESTRUCTURA MODULAR
│   │
│   ├── 📁 core/                        # ⚙️ Sistemas centrales
│   │   ├── include/core/               #    Headers públicos
│   │   │   ├── Game.hpp               #    (migrar desde include/)
│   │   │   ├── World.hpp              #    (migrar desde include/)
│   │   │   └── Chunk.hpp              #    (migrar desde include/)
│   │   ├── src/                        #    Implementaciones
│   │   │   ├── Game.cpp               #    (migrar desde src/)
│   │   │   ├── World.cpp              #    (migrar desde src/)
│   │   │   └── Chunk.cpp              #    (migrar desde src/)
│   │   ├── tests/                      #    Tests del módulo
│   │   └── README.md                   #    📖 Documentación
│   │
│   ├── 📁 rendering/                   # 🎨 Sistema de renderizado
│   │   ├── include/rendering/
│   │   │   ├── backend/               #    Backends abstractos
│   │   │   │   ├── SDL2.hpp          #       Backend SDL2 (actual)
│   │   │   │   └── Bgfx.hpp          #       Backend bgfx (futuro)
│   │   │   ├── shaders/               #    Gestión de shaders
│   │   │   ├── textures/              #    Gestión de texturas
│   │   │   └── batches/               #    Batch rendering
│   │   ├── src/                        #    Implementación
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 ecs/                         # 🧩 Entity Component System
│   │   ├── include/ecs/
│   │   │   ├── components/            #    Definiciones de componentes
│   │   │   │   ├── Position.hpp
│   │   │   │   ├── Velocity.hpp
│   │   │   │   ├── Renderable.hpp
│   │   │   │   ├── PathfindingAgent.hpp
│   │   │   │   └── ParticleEmitter.hpp
│   │   │   └── systems/               #    Sistemas de procesamiento
│   │   │       ├── MovementSystem.hpp
│   │   │       ├── RenderSystem.hpp
│   │   │       ├── PathfindingSystem.hpp
│   │   │       └── ParticleSystem.hpp
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 pathfinding/                 # 🧭 Navegación de entidades
│   │   ├── include/pathfinding/
│   │   │   ├── astar/                 #    Algoritmo A*
│   │   │   │   ├── AStar.hpp
│   │   │   │   └── Heuristic.hpp
│   │   │   └── flowfields/            #    Flow Fields
│   │   │       ├── FlowField.hpp
│   │   │       └── Integration.hpp
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 particles/                   # ✨ Sistema de partículas
│   │   ├── include/particles/
│   │   │   ├── emitters/              #    Tipos de emitters
│   │   │   │   ├── PointEmitter.hpp
│   │   │   │   ├── AreaEmitter.hpp
│   │   │   │   └── ConeEmitter.hpp
│   │   │   └── updaters/              #    Física de partículas
│   │   │       ├── PhysicsUpdater.hpp
│   │   │       ├── ColorUpdater.hpp
│   │   │       └── LifetimeUpdater.hpp
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 physics/                     # ⚡ Sistema de física
│   │   ├── include/physics/
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 ai/                          # 🤖 Inteligencia artificial
│   │   ├── include/ai/
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 audio/                       # 🔊 Sistema de audio
│   │   ├── include/audio/
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 input/                       # 🎮 Sistema de input
│   │   ├── include/input/
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   ├── 📁 ui/                          # 🖼️ Interfaz de usuario
│   │   ├── include/ui/
│   │   ├── src/
│   │   ├── tests/
│   │   └── README.md
│   │
│   └── 📁 utils/                       # 🛠️ Utilidades compartidas
│       ├── include/utils/
│       │   ├── ThreadPool.hpp
│       │   ├── Profiler.hpp
│       │   └── Logger.hpp
│       ├── src/
│       ├── tests/
│       └── README.md
│
├── 📁 shaders/                         # 🎨 Shaders de bgfx
│   ├── vs_isometric.sc                 #    Vertex shader isométrico
│   ├── fs_isometric.sc                 #    Fragment shader texturing
│   ├── varying.sc                      #    Varyings definition
│   └── bgfx_shader.sh                  #    Common shader header
│
├── 📁 tests/                           # 🧪 Suite de tests
│   ├── unit/                           #    Tests unitarios
│   ├── integration/                    #    Tests de integración
│   ├── benchmark/                      #    Benchmarks
│   └── README.md
│
├── 📁 tools/                           # 🔧 Herramientas de desarrollo
│   ├── shader_compiler/                #    Compilador de shaders
│   ├── asset_packer/                   #    Empaquetador de assets
│   ├── profiler/                       #    Profiler de rendimiento
│   └── README.md
│
├── 📄 CMakeLists.txt                   # ⚙️ Build configuration
├── 📄 BGFX_ARCHITECTURE.md             # 📖 Arquitectura de migración
├── 📄 BGFX_MIGRATION_STATUS.md         # 📊 Estado de migración
├── 📄 PROJECT_STRUCTURE.md             # 📂 Este archivo
└── 📄 README.md                        # 📖 README principal
```

## 📊 Estado de Migración

### ✅ Completado
- [x] Estructura de directorios creada
- [x] Documentación de módulos creada
- [x] bgfx compilado (libbgfxRelease.a)
- [x] CMakeLists.txt configurado
- [x] Hello world bgfx + SDL2 funcionando

### 🔄 En Progreso
- [ ] Migrar código actual a `modules/`
- [ ] Instalar EnTT
- [ ] Crear componentes ECS básicos

### ⏳ Pendiente
- [ ] Implementar backend bgfx
- [ ] Sistema de texturas bgfx
- [ ] Pathfinding system
- [ ] Particle system

## 🎯 Roadmap de Migración

### Fase 1: Core + Rendering (2-3 semanas)
1. Mover `Game`, `World`, `Chunk` → `modules/core/`
2. Mover `Renderer` → `modules/rendering/backend/sdl2/`
3. Crear interfaz abstracta de Renderer
4. Implementar backend bgfx básico

### Fase 2: ECS + Entidades (1-2 semanas)
1. Instalar EnTT
2. Crear componentes básicos
3. Implementar sistemas simples
4. Migrar Player a ECS

### Fase 3: Pathfinding (2-3 semanas)
1. Implementar A*
2. Path cache
3. Background thread processing

### Fase 4: Particles (1-2 semanas)
1. Object pooling
2. Instanced rendering
3. Diferentes tipos de efectos

### Fase 5: Optimizaciones (continuo)
1. Batch rendering
2. Frustum culling mejorado
3. SIMD donde sea posible

## 🚀 Agregando un Nuevo Módulo

1. **Crear estructura**:
   ```bash
   mkdir -p modules/nuevo_modulo/{include/nuevo_modulo,src,tests}
   ```

2. **Crear README**:
   ```bash
   touch modules/nuevo_modulo/README.md
   ```

3. **Agregar a CMakeLists.txt**:
   ```cmake
   add_subdirectory(modules/nuevo_modulo)
   ```

4. **Documentar**:
   - Agregar descripción en `modules/README.md`
   - Actualizar este documento

## 📝 Convenciones de Código

### Nombres de Archivos
- **Headers**: `PascalCase.hpp` (ej: `World.hpp`)
- **Source**: `PascalCase.cpp` (ej: `World.cpp`)
- **Tests**: `test_PascalCase.cpp` (ej: `test_World.cpp`)

### Namespaces
- Cada módulo tiene su namespace
- Ejemplo: `core::World`, `rendering::Renderer`

### Includes
- Usar rutas relativas al proyecto: `#include "core/World.hpp"`
- No usar `../..` en includes

## 🔗 Dependencias Entre Módulos

```
┌─────────────┐
│    core     │  (Sin dependencias)
└─────────────┘
       ↓
┌─────────────┐     ┌──────────────┐
│  rendering  │ ←→ │     ecs      │
└─────────────┘     └──────────────┘
       ↓                    ↓
┌─────────────┐     ┌──────────────┐
│  particles  │ ←→ │ pathfinding  │
└─────────────┘     └──────────────┘
       ↓                    ↓
┌──────────────────────────────────┐
│           utils                  │
└──────────────────────────────────┘
```

## 📈 Métricas del Proyecto

- **Líneas de código**: ~3,000 (actual)
- **Módulos**: 10 (planificados)
- **Bibliotecas externas**: 4
- **Tests**: 0 (por crear)
- **Documentación**: En progreso

---

**Última actualización**: 6 de enero de 2026
**Autor**: Claude Code
**Versión**: 0.1.0
