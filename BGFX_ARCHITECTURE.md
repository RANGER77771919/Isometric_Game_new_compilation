# Arquitectura: Migración a bgfx + SDL2 + EnTT

## 🎯 Objetivo del Proyecto

- **1,000 entidades** con pathfinding activo
- **Mundo procedural** infinito
- **Shaders modernos** (agua, iluminación, efectos)
- **10,000-50,000 partículas**
- **Isométrico moderno** (no se vea viejo)

---

## 📐 Arquitectura General

```
┌─────────────────────────────────────────────────────────────┐
│  CAPAS DEL SISTEMA                                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  CAPA DE RENDER (bgfx)                              │   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  - Vertex buffers (tiles, partículas)              │   │
│  │  - Index buffers                                     │   │
│  │  - Shaders (GLSL → SPIR-V / HLSL / GL)             │   │
│  │  - Textures (sprite sheets, partículas)             │   │
│  │  - Instanced rendering                              │   │
│  └─────────────────────────────────────────────────────┘   │
│                          ↓                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  CAPA DE ENTIDADES (EnTT ECS)                      │   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  Componentes:                                      │   │
│  │  - Position (vec3)                                  │   │
│  │  - Velocity (vec3)                                  │   │
│  │  - Renderable (sprite, color, shader)              │   │
│  │  - PathfindingAgent (destino, ruta)                │   │
│  │  - ParticleEmitter (tipo, vida, velocidad)          │   │
│  │                                                     │   │
│  │  Sistemas:                                          │   │
│  │  - MovementSystem (actualiza posiciones)            │   │
│  │  - RenderSystem (prepara vertex buffers)            │   │
│  │  - PathfindingSystem (A*, flow fields)             │   │
│  │  - ParticleSystem (actualiza partículas)           │   │
│  └─────────────────────────────────────────────────────┘   │
│                          ↓                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  CAPA DE LÓGICA (Game, World, Chunk)               │   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  - Game (main loop)                                  │   │
│  │  - World (procedural)                               │   │
│  │  - Chunk (terrain sections)                         │   │
│  │  - Pathfinding (A*, flow fields, navmesh)           │   │
│  └─────────────────────────────────────────────────────┘   │
│                          ↓                                  │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  CAPA DE PLATAFORMA (SDL2)                         │   │
│  ├─────────────────────────────────────────────────────┤   │
│  │  - Ventanas (SDL_Window)                            │   │
│  │  - Input (SDL_Event)                                │   │
│  │  - Audio (SDL_mixer - opcional)                     │   │
│  │  - Game controllers                                 │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔄 Pipeline de Renderizado (bgfx)

### **1. Inicialización**
```cpp
// Inicializar bgfx con SDL2
bgfx::Init init;
init.type = bgfx::RendererType::Direct3D11; // Windows
// init.type = bgfx::RendererType::OpenGL;   // Linux/macOS
init.resolution.width = 1280;
init.resolution.height = 720;
init.resolution.reset = BGFX_RESET_FLAGS_VSYNC;
bgfx::init(init);

// Crear shader program
bgfx::ProgramHandle program = loadShader("vs_isometric", "fs_isometric");
```

### **2. Cada Frame**
```cpp
void renderFrame() {
    // 1. Begin frame
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
    bgfx::touch(0);

    // 2. Configurar shaders
    bgfx::setProgram(program);

    // 3. Configurar vertex/index buffers
    bgfx::setVertexBuffer(0, vertexBufferHandle);
    bgfx::setIndexBuffer(indexBufferHandle);

    // 4. Set textures
    bgfx::setTexture(0, uniformTexColorHandle, textureHandle);

    // 5. Set model-view-projection matrix
    bgfx::setUniform(u_modelViewProjHandle, mat4);

    // 6. Submit draw calls
    bgfx::submit(0);

    // 7. End frame
    bgfx::frame();
}
```

---

## 🎨 Sistema de Shaders

### **Vertex Shader (vs_isometric.sc)**
```glsl
// Entradas del vertex buffer
$input a_position, a_texcoord0, a_color0

// Salidas al fragment shader
$output v_texcoord0, v_color0, v_worldPos

#include "bgfx_shader.sh"

void main()
{
    // Transformar posición
    gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

    // Pasar al fragment shader
    v_texcoord0 = a_texcoord0;
    v_color0 = a_color0;
    v_worldPos = a_position;
}
```

### **Fragment Shader (fs_isometric.sc)**
```glsl
// Entradas desde vertex shader
$input v_texcoord0, v_color0, v_worldPos

#include "bgfx_shader.sh"

void main()
{
    // Muestrear textura
    vec4 texColor = texture2D(s_texColor, v_texcoord0);

    // Aplicar color
    gl_FragColor = texColor * v_color0;
}
```

---

## 🔧 Vertex Format

```cpp
struct Vertex
{
    float x, y, z;      // Posición
    float u, v;        // Coordenadas de textura
    uint32_t abgr;     // Color (0xAABBGGRR)
    float padding_;    // Padding para alineación
};

static bgfx::VertexLayout s_vertexLayout;
s_vertexLayout
    .begin()
    .add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
    .end();
```

---

## 📊 Batch Rendering

```cpp
struct RenderBatch
{
    bgfx::VertexBufferHandle vertexBuffer;
    bgfx::IndexBufferHandle indexBuffer;
    uint32_t vertexCount;
    uint32_t indexCount;
    bgfx::ProgramHandle program;
    bgfx::TextureHandle texture;
};

// En lugar de renderizar tile por tile:
for (tile : tiles) {
    bgfx::setTexture(...);
    bgfx::submit(0);  // ❌ 10,000 draw calls
}

// Renderizamos por batches:
for (batch : batches) {
    bgfx::setVertexBuffer(0, batch.vertexBuffer);
    bgfx::setIndexBuffer(batch.indexBuffer);
    bgfx::setTexture(0, batch.texture);
    bgfx::submit(0);  // ✅ ~100-200 draw calls
}
```

---

## 🚀 Instanced Rendering (Partículas)

```cpp
// 1 instanced draw call = 10,000 partículas
struct ParticleInstanceData
{
    float x, y, z;      // Posición
    float r, g, b, a;   // Color
    float scale;       // Tamaño
    float life;        // Vida de la partícula
};

// Vertex shader para instancing
$input a_position, a_texcoord0
$input i_data0  // Instance data

void main()
{
    vec4 worldPos = mul(u_model, vec4(a_position, 1.0));
    worldPos.xyz += i_data0.xyz;  // Desplazar por instancia
    gl_Position = mul(u_viewProj, worldPos);
}

// Render
bgfx::setVertexBuffer(0, particleVertexBuffer);
bgfx::setInstanceDataBuffer(0, instanceDataBuffer);
bgfx::setInstanceCount(particleCount);
bgfx::submit(0);  // 1 llamada para todas las partículas
```

---

## 🧭 Pathfinding para 1,000 Entidades

### **A* Optimizado**
```cpp
// PathfindingSystem (corre en background thread)

struct PathfindingRequest
{
    int entityId;
    int startX, startZ;
    int endX, endZ;
    std::vector<glm::ivec2> path;
};

void PathfindingSystem::update()
{
    // Procesar solicitudes de pathfinding
    while (!pathfindingQueue.empty())
    {
        auto request = pathfindingQueue.pop();

        // A* con heurística Manhattan
        request.path = findPathAStar(
            request.startX, request.startZ,
            request.endX, request.endZ
        );

        // Guardar resultado en componente
        registry.get<PathComponent>(request.entityId).path = request.path;
    }
}
```

### **Flow Fields (para grupos)**
```cpp
// Para muchos agentes con el mismo destino
struct FlowField
{
    int targetX, targetZ;
    std::vector<glm::vec2> vectors;  // Campo de vectores
};

void updateFlowField(FlowField& field)
{
    // Integración de calor desde objetivo
    // Calcula vectores de dirección para cada celda
    // 1000 entidades siguen el mismo campo
}
```

---

## ⚡ Rendimiento Esperado

| Aspecto | **Antes (SDL2)** | **Después (bgfx)** |
|---------|-----------------|-------------------|
| **FPS idle** | 87-104 | 250-400 |
| **FPS (1000 entidades)** | 2-5 ❌ | 50-80 ✅ |
| **Partículas** | 500-1,000 | 30,000-100,000 |
| **Draw calls** | ~10,000 | ~100-200 |
| **CPU libre** | 25-40% | 70-85% |
| **Shaders** | ❌ No | ✅ Sí |

---

## 📝 Próximos Pasos

1. ✅ Compilar bx/bimg/bgfx
2. ⏳ CMakeLists.txt para linkear
3. ⏳ Hello World bgfx + SDL2
4. ⏳ Migrar texturas a bgfx
5. ⏳ Migrar loop de renderizado
6. ⏳ Optimizaciones (instanced rendering)

---
