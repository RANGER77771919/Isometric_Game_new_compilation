# Tools

Herramientas de desarrollo del proyecto.

## 📁 Contenido

### **shader_compiler/**
Compilador de shaders para bgfx.
- Compila `.sc` → `.bin`
- Integración con shaderc de bgfx

### **asset_packer/**
Empaquetador de assets.
- Combina múltiples texturas en atlas
- Comprime assets
- Genera header files

### **profiler/**
Profiler de rendimiento.
- Análisis de hotspots
- Visualización de stats

## 🚀 Usar las Tools

### Compilar Shaders:
```bash
./tools/shader_compiler/compile_shaders.bat
```

### Empaquetar Assets:
```bash
./tools/asset_packer/pack_assets.bat
```

### Profiler:
```bash
./build/profiler.exe
```

## 📋 Tareas

- [ ] Script para compilar todos los shaders
- [ ] Asset packer para texturas
- [ ] Profiler visual
