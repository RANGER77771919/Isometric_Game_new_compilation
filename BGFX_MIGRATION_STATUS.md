# Estado de Migración a bgfx

## ✅ Completado

### 1. Compilación de Bibliotecas bgfx
- ✅ **libbxRelease.a** (4.3MB) - Base math utilities
- ✅ **libbimgRelease.a** - Image processing library
- ✅ **libbgfxRelease.a** (15MB) - Rendering library

Ubicación:
- `libs/bx/.build/win64_mingw-gcc/bin/libbxRelease.a`
- `libs/bimg/.build/win64_mingw-gcc/bin/libbimgRelease.a`
- `libs/bgfx/.build/win64_mingw-gcc/bin/libbgfxRelease.a`

### 2. Configuración de CMakeLists.txt
- ✅ Include directories configurados para bgfx, bx, bimg
- ✅ Bibliotecas linkeadas correctamente
- ✅ Definiciones de compilación agregadas (BX_CONFIG_DEBUG, etc.)
- ✅ Dependencias de Windows agregadas (d3d11, dxgi, gdiplus, etc.)

### 3. Hello World bgfx + SDL2
- ✅ Archivo `test_bgfx_sdl2.cpp` creado
- ✅ Compila exitosamente
- ✅ Integra:
  - SDL2 para creación de ventana
  - SDL2 para manejo de eventos (input)
  - bgfx para renderizado gráfico
  - Vertex e index buffers
  - Transform matrices (view, projection, model)

**Ejecutable**: `build/test_bgfx_sdl2.exe` (20MB)

## 📋 Próximos Pasos

### Fase 1: Migración del Sistema de Texturas
**Objetivo**: Reemplazar SDL2 textures con bgfx textures

1. Crear TextureManager para bgfx:
   ```cpp
   class TextureManager {
       bgfx::TextureHandle loadTexture(const char* path);
       void destroyTexture(bgfx::TextureHandle handle);
   };
   ```

2. Modificar `src/Renderer.cpp`:
   - Reemplazar `SDL_Texture*` con `bgfx::TextureHandle`
   - Implementar carga de texturas con STB_IMAGE → bgfx
   - Crear texture sampler uniforms

**Archivos a modificar**:
- `include/Renderer.hpp`
- `src/Renderer.cpp`

### Fase 2: Migración del Loop de Renderizado
**Objetivo**: Reemplazar SDL_RenderCopy con bgfx draw calls

1. **Vertex Format para Tiles**:
   ```cpp
   struct IsometricVertex {
       float x, y, z;        // Posición
       float u, v;          // UV coordinates
       uint32_t abgr;       // Color tint
   };
   ```

2. **Batch Rendering System**:
   ```cpp
   struct RenderBatch {
       std::vector<IsometricVertex> vertices;
       std::vector<uint16_t> indices;
       bgfx::TextureHandle texture;
   };
   ```

3. **Modificaciones en Renderer.cpp**:
   - Reemplazar `SDL_RenderCopy()` con bgfx vertex/index buffers
   - Implementar depth sorting (usando Radix Sort ya existente)
   - Crear vertex e index buffers dinámicos
   - Implementar shader program (usar `vs_isometric.sc` y `fs_isometric.sc`)

4. **Isometric Projection Shader**:
   - El vertex shader ya existe: `shaders/vs_isometric.sc`
   - El fragment shader ya existe: `shaders/fs_isometric.sc`
   - Compilar shaders con shaderc (herramienta de bgfx)

**Archivos a modificar**:
- `src/Renderer.cpp` (migrar render loop)
- `src/Renderer.cpp` (implementar batch rendering)
- `shaders/` (compilar shaders a binario)

### Fase 3: Optimizaciones y ECS
**Objetivo**: Implementar ECS (EnTT) y optimizaciones para 1000 entidades

1. **Instalar EnTT**:
   ```bash
   git clone https://github.com/skypjack/entt.git libs/entt
   ```

2. **Crear Componentes**:
   ```cpp
   struct Position { vec3 pos; };
   struct Velocity { vec3 vel; };
   struct Renderable { bgfx::TextureHandle texture; };
   ```

3. **Pathfinding System** (background thread):
   - Implementar A* para 1000 entidades
   - Considerar Flow Fields para grupos

4. **Particle System** (instanced rendering):
   - 10,000-50,000 partículas con instanced rendering
   - 1 draw call para todas las partículas

## 🔧 Comandos Útiles

### Compilar proyecto principal:
```bash
cd build
mingw32-make
```

### Compilar test bgfx:
```bash
cd build
mingw32-make test_bgfx_sdl2
```

### Ejecutar test:
```bash
cd build
./test_bgfx_sdl2.exe
```

### Compilar shaders (cuando se implementen):
```bash
# Requiere shaderc de bgfx
./libs/bgfx/tools/bin/windows/shaderc.exe -f vs_isometric.sc -o vs_isometric.bin
./libs/bgfx/tools/bin/windows/shaderc.exe -f fs_isometric.sc -o fs_isometric.bin
```

## 📊 Rendimiento Esperado

| Aspecto | Antes (SDL2) | Después (bgfx) |
|---------|--------------|----------------|
| FPS idle | 87-104 | 250-400 |
| FPS (1000 entidades) | 2-5 ❌ | 50-80 ✅ |
| Partículas | 500-1,000 | 30,000-100,000 |
| Draw calls | ~10,000 | ~100-200 |
| CPU libre | 25-40% | 70-85% |
| Shaders | ❌ No | ✅ Sí |

## 📝 Notas Técnicas

### Compilador y Build System
- **Compilador**: MinGW-w64 UCRT64 (gcc 14.x)
- **Build System**: CMake + MinGW Makefiles
- **bgfx Build**: genie (herramienta de build de bgfx) + mingw32-make

### Plataforma
- **Windows 10/11**
- **Renderer Type**: bgfx::RendererType::Count (auto-detect)
- **Backend preferido**: Direct3D 11 (por performance en Windows)

### Dependencias
- SDL2 (ventanas + input)
- bgfx (renderizado)
- bx (math utilities)
- bimg (procesamiento de imágenes)
- STB_IMAGE (carga de imágenes)
- FastNoiseLite (generación procedural)

## ⚠️ Issues Conocidos

1. **Shaders sin compilar**: Los archivos `.sc` en `shaders/` necesitan compilarse con shaderc
2. **Sin ECS aún**: EnTT no está integrado
3. **Sin pathfinding**: El sistema de pathfinding no está implementado
4. **Sin partículas**: El sistema de partículas no está implementado

## 🚀 Siguiente Inmediato

**Recomendación**: Empezar con **Fase 1** (Migración del Sistema de Texturas)

Esto permitirá:
- Cargar texturas existentes con bgfx
- Preparar el terreno para el render loop
- Mantener compatibilidad con el resto del código

---

**Fecha**: 6 de enero de 2026
**Estado**: Configuración base completada ✅
**Tiempo estimado para migración completa**: 20-30 horas
