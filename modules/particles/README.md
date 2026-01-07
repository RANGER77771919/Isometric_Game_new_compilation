# Módulo Particles

Sistema de partículas de alto rendimiento con instanced rendering.

## 📦 Contenido

### Arquitectura:
```
particles/
├── include/particles/
│   ├── emitters/          # Tipos de emitters
│   │   ├── PointEmitter.hpp
│   │   ├── AreaEmitter.hpp
│   │   └── ConeEmitter.hpp
│   └── updaters/          # Física de partículas
│       ├── PhysicsUpdater.hpp
│       ├── ColorUpdater.hpp
│       └── LifetimeUpdater.hpp
└── src/
```

## ✨ Características

### Instanced Rendering
- **10,000+ partículas** en 1 draw call
- GPU-accelerado (bgfx)
- Actualización en compute shader (futuro)

### Tipos de Efectos
- **Fuego**: Partículas ascendentes con fade
- **Humo**: Partículas lentas con expansión
- **Explosiones**: Burst radial
- **Lluvia**: Partículas con gravedad
- **Magia**: Partículas con trail

## 📋 Tareas

### Fase 1: Sistema Básico
- [ ] ParticlePool (object pooling)
- [ ] PointEmitter simple
- [ ] PhysicsUpdater básico

### Fase 2: Instanced Rendering
- [ ] Vertex shader para instancing
- [ ] Instance data buffer
- [ ] 10,000 partículas en 1 draw call

### Fase 3: Efectos Avanzados
- [ ] Múltiples emitters
- [ ] Collisions con terreno
- [ ] Compute shader update

## 🔗 Dependencias

- **módulo rendering** (para renderizado)
- **módulo ecs** (opcional, para particle entities)
- **bgfx** (para instanced rendering)

## 📊 Rendimiento

### SDL2 (Actual):
- Máximo: ~500-1,000 partículas
- Draw calls: 500-1,000
- CPU usage: Alto

### bgfx (Futuro):
- Máximo: 30,000-100,000 partículas
- Draw calls: 1-10
- CPU usage: Bajo
- GPU update: Opcional

## 📝 Notas

- Object pooling obligatorio (sin allocations en runtime)
- Partículas son entidades ECS o datos crudos
- Actualización en paralelo cuando sea posible
- Culling por distancia a cámara

## 🎨 Ejemplo de Uso

```cpp
// Crear emitter
auto emitter = new PointEmitter();
emitter->position = {0, 10, 0};
emitter->rate = 100; // partículas/segundo
emitter->lifetime = 2.0f;

// Update (en game loop)
emitter->update(dt);
particleSystem->update(dt);

// Render (automático con rendering module)
```

## 💡 Optimizaciones

1. **Object Pooling**: Reusar partículas muertas
2. **SIMD**: Actualizar 8 partículas a la vez
3. **GPU Compute**: Actualizar en shader (futuro)
4. **Culling**: No actualizar partículas off-screen
