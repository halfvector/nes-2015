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

    SDL_Window *debugWindow;
    SDL_Renderer *renderer;
    SDL_Texture *finalTexture;

    // opengl
    SDL_GLContext glContext;
    SDL_Window *glWindow;
    uint32_t vao, vbo, ebo, rt;
    uint32_t renderTargets[2], fbo;
    uint32_t passThruShader, createGBufferShader;

    // debug buffers
    SDL_Texture *patternTexture, *attributeTexture, *paletteTexture;
    SDL_Texture *nametableTexture;
    SDL_Texture *backgroundMaskTexture;
    SDL_Texture *spriteMaskTexture;

    void renderQuad();

    void createQuad();

    void createShaders();

    void createRenderTargets();

    uint32_t createProgram(uint32_t vertexShader, uint32_t fragmentShader) const;
};


