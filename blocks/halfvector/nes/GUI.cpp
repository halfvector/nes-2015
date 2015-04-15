#include "GUI.h"

GUI::GUI(tCPU::byte* screenBuffer) : screenBuffer(screenBuffer) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Nes Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        256, 256, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);

    texture = SDL_CreateTexture(renderer,
                       SDL_PIXELFORMAT_ARGB8888,
                       SDL_TEXTUREACCESS_STREAMING,
                       256, 256);
}

void
GUI::render() {
    SDL_UpdateTexture(texture, NULL, screenBuffer, 256 * 4);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

GUI::~GUI() {
    SDL_Quit();
}