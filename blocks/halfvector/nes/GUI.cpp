#include "GUI.h"
#include "Logging.h"

GUI::GUI(Raster *raster)
        : raster(raster) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        PrintError("SDL_Init failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }

    window = SDL_CreateWindow("Nes Emulator",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              1320, 564, 0);

    if (window == NULL) {
        PrintError("SDL_CreateWindow failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        PrintError("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateRenderer failed");
    }

    finalTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    patternTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 256);
    attributeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    paletteTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 32);
    nametableTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 512);
    backgroundMaskTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    spriteMaskTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 256, 256);

    if (finalTexture == NULL) {
        PrintError("SDL_CreateTexture failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateTexture failed");
    }

    if (backgroundMaskTexture == NULL) {
        PrintError("SDL_CreateTexture failed: %s\n", SDL_GetError());
        throw std::runtime_error("SDL_CreateTexture failed");
    }

    SDL_RaiseWindow(window);
}

void
GUI::render() {
    if (SDL_UpdateTexture(finalTexture, NULL, raster->screenBuffer, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(final) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(patternTexture, NULL, raster->patternTable, 128 * 4) < 0) {
        PrintError("SDL_UpdateTexture(patternTable) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(attributeTexture, NULL, raster->attributeTable, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(attributeTable) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(paletteTexture, NULL, raster->palette, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(palette) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(nametableTexture, NULL, raster->nametables, 512 * 4) < 0) {
        PrintError("SDL_UpdateTexture(nametableTexture) failed: %s\n", SDL_GetError());
    }

    // disable rendering masks for performance reasons
//    if (SDL_UpdateTexture(backgroundMaskTexture, NULL, raster->backgroundMask, 256) < 0) {
//        PrintError("SDL_UpdateTexture(backgroundMaskTexture) failed: %s\n", SDL_GetError());
//    }
//
//    if (SDL_UpdateTexture(spriteMaskTexture, NULL, raster->spriteMask, 256) < 0) {
//        PrintError("SDL_UpdateTexture(spriteMaskTexture) failed: %s\n", SDL_GetError());
//    }

    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    // copy textures into render surface
    auto finalRect = SDL_Rect{
            0, 0, 256, 256
    };
    SDL_RenderCopy(renderer, finalTexture, NULL, &finalRect);

    // copy textures into render surface
    auto patternRect = SDL_Rect{
            266, 0, 256, 512
    };
    SDL_RenderCopy(renderer, patternTexture, NULL, &patternRect);

    auto attributesRect = SDL_Rect{
            0, 266, 256, 256
    };
    SDL_RenderCopy(renderer, attributeTexture, NULL, &attributesRect);

    auto paletteRect = SDL_Rect{
            0, 532, 256, 32
    };
    SDL_RenderCopy(renderer, paletteTexture, NULL, &paletteRect);

    auto nametableRect = SDL_Rect{
            532, 0, 512, 512
    };
    SDL_RenderCopy(renderer, nametableTexture, NULL, &nametableRect);

    auto backgroundMaskRect = SDL_Rect{
            1054, 0, 256, 256
    };
    SDL_RenderCopy(renderer, backgroundMaskTexture, NULL, &backgroundMaskRect);

    auto spriteMaskRect = SDL_Rect{
            1054, 266, 256, 256
    };
    SDL_RenderCopy(renderer, spriteMaskTexture, NULL, &spriteMaskRect);

    // present surface
    SDL_RenderPresent(renderer);
}

GUI::~GUI() {
    SDL_Quit();
}