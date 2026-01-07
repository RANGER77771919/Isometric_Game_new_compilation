# Documentación

Documentación técnica del proyecto.

## 📁 Estructura

### **architecture/**
Diagramas y descripción de arquitectura.
- `BGFX_ARCHITECTURE.md` (ya existe)
- Diagramas de módulos
- Pipeline de renderizado

### **api/**
Documentación de APIs públicas.
- Doxygen comments en código
- Generated docs

### **tutorials/**
Tutoriales y guías.
- Cómo agregar un nuevo módulo
- Cómo crear un componente ECS
- Cómo compilar shaders

### **design/**
Documentos de diseño.
- Game design docs
- Technical decisions
- Roadmap

## 📋 Documentos Existentes

- `../BGFX_ARCHITECTURE.md` - Arquitectura de migración a bgfx
- `../BGFX_MIGRATION_STATUS.md` - Estado de migración
- `../README.md` - README principal del proyecto

## 🚀 Generar API Docs

Con Doxygen:

```bash
doxygen Doxyfile
```

Output: `docs/api/html/index.html`
