# Módulo Input

Sistema unificado de manejo de input (teclado, mouse, gamepad).

## 📦 Responsabilidades

- Manejar eventos SDL2 de input
- Proveer API limpia para consultar estado
- Soportar rebinding de controles
- Input buffering para frame-perfect input

## 📋 Tareas

- [ ] InputManager singleton
- [ ] Keyboard state
- [ ] Mouse state (position, buttons, scroll)
- [ ] Gamepad support (SDL2_GameController)
- [ ] Action mapping (WASD → Movement, etc.)

## 🔗 Dependencias

- **SDL2** (event system)
