# Módulo ECS

Entity Component System usando EnTT.

## 📦 Contenido

### Arquitectura:
```
ecs/
├── include/ecs/
│   ├── components/        # Definiciones de componentes
│   │   ├── Position.hpp
│   │   ├── Velocity.hpp
│   │   ├── Renderable.hpp
│   │   ├── PathfindingAgent.hpp
│   │   └── ParticleEmitter.hpp
│   └── systems/           # Sistemas de procesamiento
│       ├── MovementSystem.hpp
│       ├── RenderSystem.hpp
│       ├── PathfindingSystem.hpp
│       └── ParticleSystem.hpp
└── src/
```

## 🧩 Componentes Planificados

### Core Components:
- **Position**: `vec3 position`
- **Velocity**: `vec3 velocity`
- **Renderable**: `texture, color, shader`
- **Transform**: `scale, rotation`

### Gameplay Components:
- **PathfindingAgent**: `target, path, speed`
- **ParticleEmitter**: `type, rate, lifetime`
- **Health**: `current, max`
- **AI**: `behavior_tree, state`

## ⚙️ Sistemas Planificados

### Movement System
- Actualiza posiciones basado en velocidad
- Delta time integration
- Collision detection (opcional)

### Render System
- Prepara datos para rendering backend
- Frustum culling
- Batch preparation

### Pathfinding System
- Corre en background thread
- A* para entidades individuales
- Flow fields para grupos

### Particle System
- Actualiza vida de partículas
- Física simple
- Instanced rendering preparation

## 📋 Tareas

### Fase 1: Setup
- [ ] Integrar biblioteca EnTT
- [ ] Crear componentes básicos
- [ ] Crear Registry global

### Fase 2: Sistemas Básicos
- [ ] MovementSystem
- [ ] RenderSystem (básico)

### Fase 3: Integración
- [ ] Migrar Player a ECS
- [ ] Migrar entidades del juego a ECS

## 🔗 Dependencias

- **EnTT** (biblioteca ECS)
- **glm** (matemáticas 3D) - opcional
- **Futuro**: módulo pathfinding, módulo rendering

## 📝 Notas

- EnTT es header-only, fácil de integrar
- Diseñado para 1,000+ entidades
- Systems corren en paralelo cuando sea posible
- Componentes POD para cache efficiency

## 🚀 Instalación de EnTT

```bash
git clone https://github.com/skypjack/entt.git libs/entt
```

Luego en CMakeLists.txt:
```cmake
add_library(entt INTERFACE)
target_include_directories(entl INTERFACE ${PROJECT_SOURCE_DIR}/libs/entt/src)
```

## 📊 Rendimiento Esperado

- **1,000 entidades**: 60 FPS fácil
- **10,000 entidades**: Posible con optimizaciones
- **Update time**: <1ms por frame para sistemas simples
