#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include "Platform.h"
#include "PPU.h"
#include <map>

class GUI {
public:
    GUI(Raster *raster);
    ~GUI();
    void render();

    bool showEnhancedPPU;
    bool showDebuggerPPU;
    bool showDebuggerAPU;

protected:
    Raster *raster;

    SDL_Window *debugWindow;
    SDL_Renderer *renderer;
    SDL_Texture *finalTexture;

    // opengl
    SDL_GLContext glContext;
    SDL_Window *glWindow;
    uint32_t vao, vbo, ebo, rt;
    uint32_t renderTargets[4], fbo;
    uint32_t passThruShader, createGBufferShader, blurShader, composeShader;
    TTF_Font *font;

    // debug buffers
    SDL_Texture *patternTexture, *attributeTexture, *paletteTexture;
    SDL_Texture *nametableTexture;
    SDL_Texture *backgroundMaskTexture;
    SDL_Texture *spriteMaskTexture;

    std::map<const char*, SDL_Texture*> labelCache;

    void renderPostProcessing();

    void createQuad();

    void createShaders();

    void createRenderTargets();

    uint32_t createProgram(uint32_t vertexShader, uint32_t fragmentShader) const;

    void drawText(const char *message, const int posX, const int posY);

    void renderDebugViews();

    SDL_Texture *generateTextureLabel(const char *message);
};


