#include "include/Game.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "=== Juego Isometrico 2D - Versión Original ===" << std::endl;
    Game game;
    if (!game.init()) {
        std::cerr << "Error al inicializar el juego!" << std::endl;
        return -1;
    }
    game.run();
    game.cleanup();
    return 0;
}
