#include "CPU6502.hpp"
#include "Bus.hpp"
#include "Cartridge.hpp"
#include "PPU.hpp"
#include <SDL2/SDL.h>
#undef main
#include <fstream>
#include <chrono>

int main()
{
    // Initialize SDL2
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        256 * 3, 240 * 3,  // 3x scale
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        256, 240
    );

    // Initialize components
    Cartridge cart("Excitebike (Japan, USA).nes");
    PPU ppu;
    Bus bus;
    CPU6502 cpu;

    // Connect components
    ppu.connectCartridge(&cart);
    bus.connectCartridge(&cart);
    bus.connectPPU(&ppu);
    bus.connectCPU(&cpu);
    cpu.connectBus(&bus);

    cpu.Reset();

    bool running = true;
    SDL_Event event;

    while (running)
    {
        // Handle SDL events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            // Controller input
            if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                bool pressed = (event.type == SDL_KEYDOWN);
                uint8_t state = bus.getControllerState(0);

                switch (event.key.keysym.sym) 
                {
                    case SDLK_x: pressed ? (state |= 0x80) : (state &= ~0x80); break;
                    case SDLK_z: pressed ? (state |= 0x40) : (state &= ~0x40); break;
                    case SDLK_a: pressed ? (state |= 0x20) : (state &= ~0x20); break;
                    case SDLK_s: pressed ? (state |= 0x10) : (state &= ~0x10); break;
                    case SDLK_UP: pressed ? (state |= 0x08) : (state &= ~0x08); break;
                    case SDLK_DOWN: pressed ? (state |= 0x04) : (state &= ~0x04); break;
                    case SDLK_LEFT: pressed ? (state |= 0x02) : (state &= ~0x02); break;
                    case SDLK_RIGHT: pressed ? (state |= 0x01) : (state &= ~0x01); break;
                }

                bus.setControllerState(0, state);
            }
        }

        // Run one frame
        ppu.frameComplete = false;
        while (!ppu.frameComplete)
        {
            // Tick CPU
            cpu.clock();

            // Tick PPU 3x per CPU cycle
            ppu.clock();
            ppu.clock();
            ppu.clock();

            // Check NMI
            if (ppu.nmiOccurred)
            {
                printf("NMI triggered\n");
                ppu.nmiOccurred = false;
                cpu.NMI();
            }
        }

        // Present frame
        SDL_UpdateTexture(texture, nullptr, ppu.getFrameBuffer(), 256 * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        static auto lastTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;

        if (elapsed.count() < (1.0 / 60.0))
        {
            SDL_Delay(1);
            continue;
        }

        lastTime = currentTime;
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}