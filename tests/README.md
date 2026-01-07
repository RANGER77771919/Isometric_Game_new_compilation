# Tests

Suite de tests unitarios, de integración y benchmarks.

## 📁 Estructura

### **unit/**
Tests unitarios de componentes individuales.
- Testea funciones/clases en aislamiento
- Rápido de ejecutar
- Ejemplo: `test_a_star_pathfinding.cpp`

### **integration/**
Tests de integración entre módulos.
- Testea interacción entre componentes
- Ejemplo: `test_pathfinding_with_world.cpp`

### **benchmark/**
Benchmarks de performance.
- Mide FPS, memory usage, etc.
- Ejemplo: `bench_1000_entities.cpp`

## 📋 Framework

Recomendado: **Catch2** o **Google Test**

```cpp
// Ejemplo con Catch2
#include <catch2/catch_test_macros.hpp>

TEST_CASE("A* finds path", "[pathfinding]") {
    auto path = findPath({0,0}, {5,5});
    REQUIRE(path.size() > 0);
}
```

## 🚀 Ejecutar Tests

```bash
cd build
./tests/unit_tests
./tests/integration_tests
./tests/benchmarks
```

## 📋 Tests Planificados

- [ ] Chunk sparse storage
- [ ] Radix sort correctness
- [ ] A* pathfinding
- [ ] bgfx initialization
- [ ] EnTT entity creation
- [ ] 1000 entities benchmark
