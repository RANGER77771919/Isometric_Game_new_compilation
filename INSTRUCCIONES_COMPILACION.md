# CÓMO COMPILAR EL JUEGO ISOMÉTRICO 2D

## OPCIÓN 1: Visual Studio Community 2022 (RECOMENDADO)

### Paso 1: Instalar Visual Studio
1. Descargar Visual Studio Community 2022 (gratis): https://visualstudio.microsoft.com/es/downloads/
2. Durante la instalación, seleccionar **"Carga de trabajo: Desarrollo de escritorio con C++"**
3. Esperar a que se complete la instalación

### Paso 2: Descargar SDL2
1. Ir a: https://github.com/libsdl-org/SDL/releases
2. Descargar el archivo más reciente que diga **`SDL2-devel-2.XX.X-VC.zip`** (versión Visual C++)
3. Extraer el ZIP en **`C:/SDL2/`**
   - Deberías tener: `C:/SDL2/include/SDL2.h`
   - Y: `C:/SDL2/lib/x64/SDL2.lib`

### Paso 3: Abrir el proyecto en Visual Studio
1. Abrir **"x64 Native Tools Command Prompt for VS 2022"** (buscar en menú inicio)
2. Navegar al proyecto:
   ```cmd
   cd C:\Users\RanDeko\Desktop\Proyect\Isometric-Game-main
   ```
3. Crear carpeta build y configurar:
   ```cmd
   mkdir build
   cd build
   cmake ..
   ```
4. Si todo está bien, verás: "SDL2 encontrado" o "Usando SDL2 manual"

### Paso 4: Compilar
```cmd
cmake --build . --config Release
```

### Paso 5: Ejecutar
```cmd
cd Release
Isometrico2D.exe
```

---

## OPCIÓN 2: Sin Visual Studio (solo CMake)

Si NO quieres instalar Visual Studio, puedes intentar:

### Requisitos:
- CMake 3.20+
- Un compilador C++ (MinGW, Clang, o MSVC)

### Pasos:
1. Descargar SDL2 desde: https://github.com/libsdl-org/SDL/releases
2. Extraer en **`C:/SDL2/`**
3. Abrir terminal en la carpeta del proyecto
4. Ejecutar:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

---

## CONTROLES DEL JUEGO
- **W/A/S/D**: Mover cámara (isométrico)
- **Q/E**: Subir/Bajar altura
- **ESC**: Salir
- **P**: Pausar/Reanudar

---

## ESTRUCTURA DEL PROYECTO
```
Isometric-Game-main/
├── include/          # Headers (.hpp)
│   ├── Block.hpp     # Tipos de bloques
│   ├── Chunk.hpp     # Sistema de chunks (16x16x16)
│   ├── World.hpp     # Mundo procedural infinito
│   ├── Camera.hpp    # Cámara isométrica
│   ├── Renderer.hpp  # Renderizador isométrico
│   └── Game.hpp      # Loop principal
├── src/              # Implementaciones (.cpp)
├── assets/tiles/     # Texturas de los tiles
├── libs/             # FastNoiseLite, stb_image
└── CMakeLists.txt    # Configuración de CMake
```

---

## CARACTERÍSTICAS IMPLEMENTADAS
✅ Mundo 3D procedural infinito con semilla
✅ Sistema de chunks dinámicos (16x16x16 bloques)
✅ Terreno generado con ruido Perlin (biomas: pasto, nieve, desierto, etc.)
✅ Renderizado isométrico con depth sorting
✅ Cámara libre con movimiento en 3D
✅ Carga/descarga dinámica de chunks
✅ Optimización: solo renderiza bloques visibles
✅ Altura del mundo: 256 bloques

---

## SOLUCIÓN DE PROBLEMAS

### Error: "SDL2 no encontrado"
**Solución**: Descargar SDL2-devel-2.XX.X-VC.zip y extraer en C:/SDL2/

### Error: "No se puede abrir SDL2/SDL.h"
**Solución**: Verificar que SDL2 esté en C:/SDL2/include/

### Error de linker
**Solución**: Usar Visual Studio Community, es el más estable en Windows

### El juego no carga texturas
**Solución**: Asegurarse que la carpeta `assets/tiles/` esté junto al .exe

---

## NOTAS
- El proyecto usa **C++20**
- Requiere **CMake 3.20+**
- Las texturas deben estar en `assets/tiles/`
- La primera vez que corras el juego, generará chunks alrededor de (0,0,0)
