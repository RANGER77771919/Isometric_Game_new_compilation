# Módulo Pathfinding

Sistema de navegación para 1,000 entidades con pathfinding activo.

## 📦 Contenido

### Algoritmos:
```
pathfinding/
├── include/pathfinding/
│   ├── astar/             # Algoritmo A*
│   │   ├── AStar.hpp      # Implementación principal
│   │   └── Heuristic.hpp  # Heurísticas (Manhattan, Euclidean)
│   └── flowfields/        # Flow Fields
│       ├── FlowField.hpp  # Campo de vectores
│       └── Integration.hpp # Integración de calor
└── src/
```

## 🧭 Estrategias de Pathfinding

### A* (Individual)
- **Uso**: Entidades individuales o pequeños grupos
- **Características**:
  - Pathfinding exacto
  - Compartir cache entre entidades con mismo destino
  - Corre en background thread

### Flow Fields (Grupos)
- **Uso**: Muchas entidades con el mismo destino
- **Características**:
  - Pathfinding compartido
  - Bajo costo por entidad
  - 1,000+ entidades fácilmente

## 📋 Tareas

### Fase 1: A* Básico
- [ ] Implementar A* standard
- [ ] Heurística Manhattan
- [ ] Path smoothing

### Fase 2: Optimizaciones
- [ ] Path cache (compartir paths)
- [ ] Hierarchical pathfinding
- [ ] Background thread processing

### Fase 3: Flow Fields
- [ ] Implementar flow field integration
- [ ] Field cache
- [ ] Dynamic obstacles

## 🔗 Dependencias

- **módulo ecs** (para componentes Position, PathfindingAgent)
- **módulo core** (para acceso a World/Chunk)
- **ThreadPool** (para background processing)

## 📊 Rendimiento

### A*:
- **Costo por path**: ~1-5ms (depende de distancia)
- **Cache hit**: <0.1ms
- **Soporta**: ~100 paths/frame

### Flow Fields:
- **Costo por field**: ~10-20ms (one-time)
- **Costo por entidad**: ~0.01ms
- **Soporta**: 10,000+ entidades

## 📝 Notas

- Pathfinding asíncrono (no bloquea el main thread)
- Cache de paths para reutilización
- Recalcular solo cuando el terreno cambia
- Prioridad por distancia a cámara

## 🎯 Casos de Uso

1. **Villagers moviéndose**: A* individual
2. **Ejército atacando**: Flow field
3. **Animales pastando**: Random walk + avoidance
