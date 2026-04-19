#include <SDL.h>
#include "core/emulator.hpp"
#include "common/log.hpp"
#include <chrono>
#include <thread>

int main(int argc, char** argv) {
    if (argc < 2) {
        Log::Error("Usage: %s <rom>", argv[0]);
        return 1;
    }

    Emulator emu;
    if (!emu.LoadROM(argv[1])) {
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        Log::Error("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "zenboy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        160 * 4, 144 * 4,
        SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        Log::Error("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    Log::Info("Renderer: %s", info.name);

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        160, 144
    );

    bool running = true;
    SDL_Event e;

    const double targetFrameTime = 1000.0 / 59.7275;

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8* state = SDL_GetKeyboardState(NULL);

        // Action buttons: bit 4 (A, B, Select, Start)
        // 0 = pressed
        u8 action = 0x0F;
        if (state[SDL_SCANCODE_X]) action &= ~0x01; // A
        if (state[SDL_SCANCODE_Z]) action &= ~0x02; // B
        if (state[SDL_SCANCODE_BACKSPACE]) action &= ~0x04; // Select
        if (state[SDL_SCANCODE_RETURN]) action &= ~0x08; // Start

        // Direction buttons: bit 5 (Right, Left, Up, Down)
        u8 dpad = 0x0F;
        if (state[SDL_SCANCODE_RIGHT]) dpad &= ~0x01;
        if (state[SDL_SCANCODE_LEFT]) dpad &= ~0x02;
        if (state[SDL_SCANCODE_UP]) dpad &= ~0x04;
        if (state[SDL_SCANCODE_DOWN]) dpad &= ~0x08;

        emu.SetInput(action, dpad);

        // Advance 1 frame
        emu.StepFrame();

        // Render
        void* pixels;
        int pitch;
        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        memcpy(pixels, emu.GetFrameBuffer(), 160 * 144 * 4);
        SDL_UnlockTexture(texture);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // Pacing
        auto end = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        if (elapsedMs < targetFrameTime) {
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(targetFrameTime - elapsedMs));
        }
    }

    emu.Shutdown();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
