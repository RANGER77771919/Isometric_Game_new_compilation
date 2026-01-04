#include "Game.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Juego Isometrico 2D - Sandbox ===" << std::endl;
    std::cout << "Controles: WASD=Mover, +/-=Zoom, P=Pausar, ESC=Salir" << std::endl;

    Game game;

    if (!game.init()) {
        std::cerr << "Error al inicializar el juego!" << std::endl;
        return -1;
    }

    game.run();
    game.cleanup();

    return 0;
}
