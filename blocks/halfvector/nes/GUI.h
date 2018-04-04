#pragma once

#include <SDL2/SDL.h>
#include "Platform.h"
#include "PPU.h"

class GUI {
public:
    GUI(Raster *raster);

    ~GUI();

    void render();

protected:
    Raster *raster;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *finalTexture;

    // debug buffers
    SDL_Texture *patternTexture, *attributeTexture, *paletteTexture;
};


