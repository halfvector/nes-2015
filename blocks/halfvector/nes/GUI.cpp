#include "GUI.h"
#include "Logging.h"

GUI::GUI(tCPU::byte* screenBuffer) : screenBuffer(screenBuffer) {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        PrintError("SDL_Init failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }

    window = SDL_CreateWindow("Nes Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        256, 256, 0);

    if(window == NULL) {
        PrintError("SDL_CreateWindow failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL) {
        PrintError("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateRenderer failed");
    }

    texture = SDL_CreateTexture(renderer,
                       SDL_PIXELFORMAT_ARGB8888,
                       SDL_TEXTUREACCESS_STREAMING,
                       256, 256);

    if(texture == NULL) {
        PrintError("SDL_CreateTexture failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateTexture failed");
    }

    SDL_RaiseWindow(window);
}

void
GUI::render() {
    if(SDL_UpdateTexture(texture, NULL, screenBuffer, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture failed: %s\n", SDL_GetError());
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // pump the event loop to ensure window visibility
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}
}

GUI::~GUI() {
    SDL_Quit();
}