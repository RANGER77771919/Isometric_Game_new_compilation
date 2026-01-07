# Módulo Rendering

Sistema de renderizado abstracto con múltiples backends.

## 📦 Contenido

### Arquitectura:
```
rendering/
├── include/rendering/
│   ├── Renderer.hpp        # Interfaz abstracta
│   ├── backend/
│   │   ├── SDL2.hpp       # Backend SDL2 (actual)
│   │   └── Bgfx.hpp       # Backend bgfx (futuro)
│   ├── shaders/           # Gestión de shaders
│   ├── textures/          # Gestión de texturas
│   └── batches/           # Batch rendering
└── src/
```

## 🎨 Backends

### SDL2 Backend (Actual)
- **Ubicación**: `src/Renderer.cpp`
- **Características**:
  - CPU-bound
  - Sin shaders
  - Simple pero lento para muchos objetos

### bgfx Backend (Futuro)
- **Características**:
  - GPU-accelerated
  - Shaders modernos (GLSL)
  - Instanced rendering
  - Multi-API (DX11, OpenGL, Vulkan)

## 📋 Tareas de Migración

### Fase 1: Abstracción
- [ ] Crear interfaz `RendererBase` abstracta
- [ ] Mover `Renderer.cpp` a `backend/sdl2/`
- [ ] Crear stub de `backend/bgfx/`

### Fase 2: bgfx Implementation
- [ ] Implementar bgfx backend
- [ ] Sistema de texturas bgfx
- [ ] Compilar shaders (vs_isometric, fs_isometric)

### Fase 3: Optimizaciones
- [ ] Batch rendering system
- [ ] Instanced rendering para partículas
- [ ] Frustum culling mejorado

## 🔗 Dependencias

- **SDL2** (ventanas + input)
- **bgfx/bx/bimg** (renderizado moderno)
- **STB_IMAGE** (carga de texturas)
- **Futuro**: módulo ECS para componentes renderizables

## 📊 Rendimiento Esperado

| Métrica | SDL2 | bgfx |
|---------|------|------|
| Draw calls | ~10,000 | ~100-200 |
| FPS (1000 entidades) | 2-5 | 50-80 |
| Partículas | ~1,000 | ~50,000 |

## 📝 Notas

- Diseñado para cambiar backend en runtime
- Código legacy en `src/Renderer.cpp` se migrará aquí
- Los shaders en `/shaders/` se usarán para bgfx
