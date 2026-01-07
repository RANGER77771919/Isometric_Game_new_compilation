/**
 * Hello World con bgfx + SDL2
 *
 * Este programa demuestra la integración básica de:
 * - SDL2 para creación de ventana y manejo de input
 * - bgfx para renderizado gráfico
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <cstdio>
#include <cstdint>

struct PosColorVertex
{
    float x, y, z;
    uint32_t abgr;

    static void init()
    {
        ms_layout
            .begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;

static PosColorVertex s_cubeVertices[] =
{
    {-1.0f,  1.0f,  1.0f, 0xff000000 },
    { 1.0f,  1.0f,  1.0f, 0xff0000ff },
    {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
    { 1.0f, -1.0f,  1.0f, 0xff00ffff },
    {-1.0f,  1.0f, -1.0f, 0xffff0000 },
    { 1.0f,  1.0f, -1.0f, 0xffff00ff },
    {-1.0f, -1.0f, -1.0f, 0xffffff00 },
    { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeIndices[] =
{
    0, 1, 2, 1, 3, 2,
    4, 6, 5, 5, 6, 7,
    0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3,
    0, 4, 1, 4, 5, 1,
    2, 3, 6, 6, 3, 7,
};

int main(int argc, char** argv)
{
    printf("=== bgfx + SDL2 Hello World ===\n");

    // Inicializar SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Error inicializando SDL2: %s\n", SDL_GetError());
        return 1;
    }

    // Crear ventana SDL2
    SDL_Window* window = SDL_CreateWindow(
        "bgfx + SDL2 Hello World",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        1280, 720,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        printf("Error creando ventana SDL2: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Ventana SDL2 creada: 1280x720\n");

    // Obtener información nativa de la ventana para bgfx
    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    SDL_GetWindowWMInfo(window, &wmi);

    // Inicializar bgfx
    bgfx::Init init;
    init.type = bgfx::RendererType::Count; // Auto-detectar API

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    init.platformData.ndt = wmi.info.x11.display;
    init.platformData.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
    init.platformData.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
    init.platformData.nwh = wmi.info.win.window;
#endif

    init.resolution.width = 1280;
    init.resolution.height = 720;
    init.resolution.reset = BGFX_RESET_VSYNC;

    if (!bgfx::init(init))
    {
        printf("Error inicializando bgfx\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("bgfx inicializado exitosamente\n");

    // Configurar view y clear color
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, 1280, 720);

    printf("View configurado\n");

    // Inicializar vertex layout
    PosColorVertex::init();

    // Crear vertex buffer
    bgfx::VertexBufferHandle vbh = bgfx::createVertexBuffer(
        bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)),
        PosColorVertex::ms_layout
    );

    // Crear index buffer
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(
        bgfx::makeRef(s_cubeIndices, sizeof(s_cubeIndices))
    );

    printf("Buffers creados\n");

    // Crear shaders simples (solid color)
    // Por ahora, sin shaders - solo probamos inicialización

    bool running = true;
    SDL_Event event;

    printf("\n=== Iniciando loop principal ===\n");
    printf("Presiona ESC para cerrar\n\n");

    int frameCount = 0;

    while (running)
    {
        // Manejar eventos SDL2
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                running = false;
            }
        }

        // Configurar view para este frame
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, uint16_t(1280), uint16_t(720));

        // Simular modelo simple (rotación)
        static float time = 0.0f;
        time += 0.01f;

        float view[16];
        float proj[16];

        // Configurar view matrix (cámara)
        bx::mtxLookAt(
            view,
            {0.0f, 0.0f, -5.0f}, // eye
            {0.0f, 0.0f, 0.0f},  // at
            {0.0f, 1.0f, 0.0f}   // up
        );

        // Configurar projection matrix
        bx::mtxProj(
            proj,
            60.0f, // fov
            float(1280) / float(720), // aspect
            0.1f,  // near
            100.0f, // far
            false, // homogeneous
            bx::Handedness::Left
        );

        bgfx::setViewTransform(0, view, proj);

        // Configurar model matrix (rotación del cubo)
        float model[16];
        bx::mtxRotateXY(model, time * 0.5f, time * 0.3f);
        bgfx::setTransform(model);

        // Submit draw call (sin shader por ahora, solo test)
        // bgfx::setVertexBuffer(0, vbh);
        // bgfx::setIndexBuffer(ibh);
        // bgfx::submit(0, program);

        bgfx::touch(0); // Marcar view como usado

        // Presentar frame
        bgfx::frame();

        frameCount++;
        if (frameCount % 60 == 0)
        {
            printf("Frame %d - bgfx funcionando correctamente\n", frameCount);
        }
    }

    printf("\n=== Cerrando ===\n");
    printf("Total frames: %d\n", frameCount);

    // Cleanup
    bgfx::destroy(vbh);
    bgfx::destroy(ibh);

    bgfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("bgfx + SDL2 Hello World finalizado exitosamente!\n");

    return 0;
}
