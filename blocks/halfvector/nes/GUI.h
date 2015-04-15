#pragma once

#include "Platform.h"
#include "miguel/sdl2/include/SDL.h"

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


