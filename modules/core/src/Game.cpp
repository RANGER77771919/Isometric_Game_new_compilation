#include "core/Game.hpp"
#include <iostream>
#include <chrono>
#include <cmath>  // Para std::floor

Game::Game()
    : m_window(nullptr)
    , m_renderer(nullptr)
    , m_world(nullptr)
    , m_camera(nullptr)
    , m_lastFrameTime(0)
    , m_lastChunkPos(0, 0)      // Inicializar en origen (0, 0)
    , m_lastChunkUpdateTime(0) // Inicializar timer de estadísticas
{
}

Game::~Game() {
    // Cleanup es llamado explícitamente en main.cpp
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL no pudo inicializarse: " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(
        "Juego Isometrico 2D - Sandbox",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "No se pudo crear la ventana: " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = std::make_unique<Renderer>(m_window);
    if (!m_renderer->getSDLRenderer()) {
        std::cerr << "No se pudo crear el renderer" << std::endl;
        return false;
    }

    uint32_t seed = static_cast<uint32_t>(std::time(nullptr));
    m_world = std::make_unique<World>(seed);

    m_camera = std::make_unique<Camera>();
    m_camera->setZoom(2.0f);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    m_camera->setCenter(windowWidth / 2.0f, windowHeight / 2.0f);

    updateChunks();

    m_player = std::make_unique<Player>(0.0f, 0.0f, 0.0f);
    m_player->spawnOnSurface(m_world.get());

    float playerX, playerY, playerZ;
    m_player->getPosition(playerX, playerY, playerZ);
    m_camera->setPosition(playerX, playerY, playerZ);

    return true;
}

void Game::run() {
    m_lastFrameTime = SDL_GetTicks64();

    while (m_state.running) {
        // Calcular deltaTime
        Uint64 currentTime = SDL_GetTicks64();
        float deltaTime = (currentTime - m_lastFrameTime) / 1000.0f;
        m_lastFrameTime = currentTime;

        // Procesar input
        handleInput();

        // Actualizar lógica
        if (!m_state.paused) {
            update(deltaTime);
        }

        // Renderizar
        render();

        // Limitar FPS
        Uint64 frameTime = SDL_GetTicks64() - currentTime;
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime);
        }
    }
}

void Game::cleanup() {
    m_renderer.reset();
    m_world.reset();
    m_camera.reset();
    m_player.reset();

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void Game::handleInput() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_state.running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        m_state.running = false;
                        break;

                    // Movimiento de cámara
                    case SDLK_w:
                        m_state.moveUp = true;
                        break;
                    case SDLK_s:
                        m_state.moveDown = true;
                        break;
                    case SDLK_a:
                        m_state.moveLeft = true;
                        break;
                    case SDLK_d:
                        m_state.moveRight = true;
                        break;

                    // Salto con ESPACIO
                    case SDLK_SPACE:
                        m_player->tryJump(m_world.get());
                        break;

                    // Control de zoom
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        m_state.zoomIn = true;
                        break;

                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        m_state.zoomOut = true;
                        break;

                    case SDLK_p:
                        m_state.paused = !m_state.paused;
                        break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        m_state.moveUp = false;
                        break;
                    case SDLK_s:
                        m_state.moveDown = false;
                        break;
                    case SDLK_a:
                        m_state.moveLeft = false;
                        break;
                    case SDLK_d:
                        m_state.moveRight = false;
                        break;

                    // Control de zoom - liberar teclas
                    case SDLK_PLUS:
                    case SDLK_KP_PLUS:
                        m_state.zoomIn = false;
                        break;

                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        m_state.zoomOut = false;
                        break;
                }
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // Actualizar centro de la cámara cuando se redimensiona la ventana
                    int newWidth = event.window.data1;
                    int newHeight = event.window.data2;
                    m_camera->setCenter(newWidth / 2.0f, newHeight / 2.0f);
                }
                break;
        }
    }
}

void Game::update(float deltaTime) {
    // Actualizar física del jugador (gravedad)
    m_player->update(deltaTime, m_world.get());

    // Actualizar posición de la cámara
    updateCamera(deltaTime);

    // Cargar/descargar chunks según posición de la cámara
    updateChunks();
}

void Game::updateCamera(float deltaTime) {
    // SISTEMA DE MOVIMIENTO DISCRETO POR GRID
    // En lugar de movimiento continuo, usamos tryMove() para moverse un bloque completo

    // Intentar mover en el eje Z (Norte/Sur)
    if (m_state.moveUp) {
        // W: Mover hacia el NORTE (Z negativo)
        m_player->tryMove(0, 0, -1, m_world.get());
    } else if (m_state.moveDown) {
        // S: Mover hacia el SUR (Z positivo)
        m_player->tryMove(0, 0, 1, m_world.get());
    }

    // Intentar mover en el eje X (Oeste/Este)
    if (m_state.moveLeft) {
        // A: Mover hacia el OESTE (X negativo)
        m_player->tryMove(-1, 0, 0, m_world.get());
    } else if (m_state.moveRight) {
        // D: Mover hacia el ESTE (X positivo)
        m_player->tryMove(1, 0, 0, m_world.get());
    }

    // Hacer que la cámara siga al player en X, Y, Z
    // La cámara usa las coordenadas visuales (interpoladas) para movimiento suave
    float playerX, playerY, playerZ;
    m_player->getPosition(playerX, playerY, playerZ);
    m_camera->setPosition(playerX, playerY, playerZ);

    // Aplicar zoom
    if (m_state.zoomIn || m_state.zoomOut) {
        float currentZoom = m_camera->getZoom();
        float zoomChange = (4.9f / ZOOM_SPEED) * deltaTime;  // Rango 0.1-5.0 = 4.9 de diferencia

        if (m_state.zoomIn) {
            m_camera->setZoom(currentZoom + zoomChange);
        }
        if (m_state.zoomOut) {
            m_camera->setZoom(currentZoom - zoomChange);
        }
    }
}

void Game::updateChunks() {
    // Obtener posición actual de la cámara en coordenadas mundiales
    float camX, camY, camZ;
    m_camera->getPosition(camX, camY, camZ);

    // OPTIMIZACIÓN FASE 4: Usar cache para evitar std::floor repetidos
    ChunkPos currentChunkPos = getCameraChunkPos(camX, camY, camZ);

    // Calcular distancia de movimiento desde la última posición
    // Usamos distancia Manhattan para mejor rendimiento
    int distX = abs(currentChunkPos.x - m_lastChunkPos.x);
    int distZ = abs(currentChunkPos.z - m_lastChunkPos.z);
    int totalMovement = distX + distZ;

    // Solo procesar carga/descarga si nos movimos suficientemente
    // Esto evita procesamiento innecesario cuando el jugador está quieto
    if (totalMovement >= MOVEMENT_THRESHOLD) {
        // Paso 1: Descartar chunks muy lejanos (liberar memoria)
        m_world->unloadChunksFarFrom(currentChunkPos, UNLOAD_DISTANCE);

        // Paso 2: Cargar nuevos chunks necesarios (genera si no existen)
        m_world->getChunksAround(currentChunkPos, LOAD_RADIUS);

        // Paso 3: Actualizar última posición de la cámara
        m_lastChunkPos = currentChunkPos;
    }
}

void Game::render() {
    // Limpiar pantalla
    m_renderer->clear();

    // Obtener posición actual de la cámara
    float camX, camY, camZ;
    m_camera->getPosition(camX, camY, camZ);

    // OPTIMIZACIÓN FASE 4: Usar cache para evitar std::floor repetidos
    ChunkPos camChunkPos = getCameraChunkPos(camX, camY, camZ);

    // OPTIMIZACIÓN FASE 1: Usar move semantics para evitar allocation adicional
    m_visibleChunksCache = std::move(m_world->getChunksAround(camChunkPos, RENDER_RADIUS));

    // Renderizar mundo
    m_renderer->renderWorld(m_visibleChunksCache, *m_camera);

    // Renderizar player
    float playerX, playerY, playerZ;
    m_player->getPosition(playerX, playerY, playerZ);
    m_renderer->renderPlayer(*m_camera, playerX, playerY, playerZ, m_player->getTileName());

    // Presentar renderizado
    m_renderer->present();
}
