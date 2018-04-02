#pragma once

#include <SDL2/SDL.h>
#include "Platform.h"

class GUI {
public:
    GUI(tCPU::byte* final, tCPU::byte* patternBuffer);
    ~GUI();

    void render();

protected:
    tCPU::byte* finalBuffer;
    tCPU::byte* patternBuffer;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* finalTexture;

    // debug buffers
    SDL_Texture* patternTexture;
};


