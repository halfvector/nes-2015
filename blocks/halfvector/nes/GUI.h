#pragma once

#include <SDL2/SDL.h>
#include "Platform.h"

class GUI {
public:
    GUI(tCPU::byte*);
    ~GUI();

    void render();

protected:
    tCPU::byte* screenBuffer;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};


