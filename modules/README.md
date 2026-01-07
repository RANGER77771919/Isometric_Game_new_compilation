# Módulos del Proyecto Isometrico2D

Este directorio contiene todos los módulos del juego organizados por funcionalidad.

## 📁 Estructura de Módulos

### **core/** 🔧
Sistemas centrales del juego.
- **Propósito**: Game loop, World, Chunk management
- **Estado**: Migración desde `src/` y `include/`
- **Dependencias**: Ninguna (solo STL)

### **rendering/** 🎨
Sistema de renderizado multi-backend.
- **Propósito**: Renderizado abstracto (SDL2, bgfx)
- **Backends**:
  - `backend/sdl2/` - SDL2 rendering (actual)
  - `backend/bgfx/` - bgfx rendering (futuro)
- **Subsistemas**:
  - `shaders/` - Gestión de shaders
  - `textures/` - Gestión de texturas
  - `batches/` - Batch rendering system

### **ecs/** 🧩
Entity Component System usando EnTT.
- **Propósito**: Arquitectura de entidades
- **Contenido**:
  - `components/` - Definición de componentes
  - `systems/` - Sistemas de procesamiento
- **Componentes planificados**:
  - Position, Velocity, Renderable
  - PathfindingAgent, ParticleEmitter

### **pathfinding/** 🧭
Sistema de navegación y pathfinding.
- **Propósito**: 1,000 entidades con pathfinding activo
- **Algoritmos**:
  - `astar/` - Algoritmo A* optimizado
  - `flowfields/` - Flow fields para grupos

### **particles/** ✨
Sistema de partículas con instanced rendering.
- **Propósito**: 10,000-50,000 partículas
- **Contenido**:
  - `emitters/` - Tipos de emitters
  - `updaters/` - Física de partículas

### **physics/** ⚡
Sistema de física (opcional/futuro).
- **Propósito**: Colisiones, gravedad
- **Estado**: No planeado inicialmente

### **ai/** 🤖
Inteligencia artificial.
- **Propósito**: Comportamiento de entidades
- **Estado**: Pendiente de definición

### **audio/** 🔊
Sistema de audio.
- **Propósito**: Música y efectos de sonido
- **Backend**: SDL_mixer o similar

### **input/** 🎮
Sistema de input unificado.
- **Propósito**: Manejo de input (teclado, mouse, gamepad)
- **Backend**: SDL2 events

### **ui/** 🖼️
Interfaz de usuario.
- **Propósito**: HUD, menús, debug UI
- **Estado**: Pendiente de definición

### **utils/** 🛠️
Utilidades compartidas.
- **Propósito**: Funciones helper, math extensions
- **Contenido**: ThreadPool, Profiler, Logger

## 🔄 Estrategia de Migración

### Fase 1: Core + Rendering
1. Mover `Game`, `World`, `Chunk` → `modules/core/`
2. Mover `Renderer` → `modules/rendering/`
3. Crear backends SDL2 y bgfx

### Fase 2: ECS + Pathfinding
1. Integrar EnTT en `modules/ecs/`
2. Implementar componentes básicos
3. Implementar A* en `modules/pathfinding/`

### Fase 3: Particles + Optimizations
1. Instanced rendering en `modules/rendering/`
2. Sistema de partículas en `modules/particles/`
3. Flow fields en `modules/pathfinding/`

## 📋 Convenciones

### Estructura de cada módulo:
```
modules/nombre_modulo/
├── include/           # Headers públicos
│   └── nombre_modulo/
│       └── *.hpp      # Interfaces
├── src/               # Implementaciones
│   └── *.cpp          # Código fuente
├── tests/             # Tests del módulo
│   ├── unit/          # Tests unitarios
│   └── integration/   # Tests de integración
└── README.md          # Este archivo
```

### Reglas:
1. **Cada módulo es autocontenido**
2. **Interfaces públicas en `include/`**
3. **Implementaciones privadas en `src/`**
4. **Tests en `tests/`**
5. **Sin dependencias circulares**

## 🚀 Agregando un Nuevo Módulo

1. Crear directorio bajo `modules/`
2. Seguir estructura estándar (include/, src/, tests/)
3. Agregar README.md explicando propósito
4. Actualizar este README principal
5. Agregar a CMakeLists.txt

---

**Última actualización**: 6 de enero de 2026
