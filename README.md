# 🎮 Juego Isométrico 2D - Sandbox

Un juego sandbox estilo Minecraft con vista isométrica 2D, mundo procedural infinito, y sistema de chunks dinámicos.

![Versión](https://img.shields.io/badge/version-1.0-blue)
![C++](https://img.shields.io/badge/C++-20-blue)
![SDL2](https://img.shields.io/badge/SDL2-2.0+-green)

---

## 🎯 Características

### Mundo Infinito Procedural
- ✅ **Generación procedural** basada en semilla (como Minecraft)
- ✅ **Chunks dinámicos** de 16x16x256 bloques
- ✅ **Carga/descarga automática** de chunks
- ✅ **Sistema de biomas**: Pasto, Nieve, Desierto, Hierba Roja

### Terreno 3D
- ✅ **Altura variable**: 50-110 bloques sobre el nivel del mar
- ✅ **Cuevas 3D** generadas con ruido Simplex
- ✅ **Sistema de capas**: Bedrock → Piedra → Tierra → Superficie
- ✅ **9 tipos de bloques**: Aire, Pasto, Tierra, Piedra, Arena, Nieve, Agua, Hierba Sangre, Pasto Full

### Renderizado Isométrico
- ✅ **Proyección isométrica** correcta
- ✅ **Depth sorting** para renderizar en orden (back-to-front)
- ✅ **Frustum culling**: Solo renderiza bloques visibles
- ✅ **Texturas de 32x32** píxeles

### Controles
- **W/A/S/D** - Mover cámara horizontalmente
- **Q/E** - Subir/Bajar altura
- **P** - Pausar/Reanudar
- **ESC** - Salir

---

## 📸 Capturas

*Agregar capturas del juego aquí*

---

## 🚀 Cómo Compilar

### Requisitos
- **Visual Studio Community 2022** (gratis) o equivalente
- **CMake 3.20+**
- **SDL2-devel-2.XX.X-VC.zip**

### Instrucciones Rápidas

1. **Instalar Visual Studio Community 2022**
   - Descargar: https://visualstudio.microsoft.es/downloads/
   - Seleccionar "Desarrollo de escritorio con C++"

2. **Descargar SDL2**
   - Ir a: https://github.com/libsdl-org/SDL/releases
   - Descargar `SDL2-devel-2.XX.X-VC.zip`
   - Extraer en `C:/SDL2/`

3. **Compilar**
   ```cmd
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Release
   ```

4. **Ejecutar**
   ```cmd
   cd Release
   Isometrico2D.exe
   ```

📖 **Instrucciones detalladas**: Ver [INSTRUCCIONES_COMPILACION.md](INSTRUCCIONES_COMPILACION.md)

---

## 📁 Estructura del Proyecto

```
Isometric-Game-main/
├── include/              # Headers
│   ├── Block.hpp        # Tipos de bloques
│   ├── Chunk.hpp        # Sistema de chunks
│   ├── World.hpp        # Mundo procedural
│   ├── Camera.hpp       # Cámara isométrica
│   ├── Renderer.hpp     # Renderizador
│   └── Game.hpp         # Loop principal
├── src/                 # Implementación
│   ├── main.cpp
│   ├── Game.cpp
│   ├── Renderer.cpp
│   ├── World.cpp
│   ├── Chunk.cpp
│   └── Camera.cpp
├── assets/tiles/        # Texturas
├── libs/                # FastNoiseLite, stb_image
├── CMakeLists.txt       # Configuración CMake
├── INSTRUCCIONES_COMPILACION.md
├── LICENSES.txt
├── CREDITS.txt
└── README.md
```

---

## 🛠️ Tecnologías

- **C++20** - Lenguaje
- **SDL2** - Ventana y rendering (zlib License)
- **FastNoiseLite** - Generación procedural (MIT License)
- **stb_image** - Carga de imágenes (MIT License)
- **CMake** - Sistema de build

---

## 📜 Licencia

Este proyecto utiliza librerías Open Source con licencias permisivas que permiten uso comercial sin regalías.

Ver [LICENSES.txt](LICENSES.txt) para más información.

---

## 🎯 Próximas Características (Roadmap)

- [ ] Sistema de guardado/carga de mundos
- [ ] Colocación/destrucción de bloques
- [ ] Jugador con física
- [ ] Sistema de inventario
- [ ] Múltiples capas de renderizado
- [ ] day/night cycle
- [ ] Entidades y mobs
- [ ] Red multijugador

---

## 🤝 Contribuir

Las contribuciones son bienvenidas. Por favor:

1. Fork el proyecto
2. Crea una rama para tu feature (`git checkout -b feature/MiFeature`)
3. Commit tus cambios (`git commit -m 'Add some MiFeature'`)
4. Push a la rama (`git push origin feature/MiFeature`)
5. Abre un Pull Request

---

## 📧 Contacto

- **Autor**: Tu nombre
- **Email**: tu@email.com
- **Web**: https://tusitio.com

---

## ⭐ Agradecimientos

- Comunidad de SDL2
- Contribuidores de FastNoiseLite
- Comunidad de desarrollo de videojuegos

---

**¡Disfruta el juego! 🎮**
