#define GL_SILENCE_DEPRECATION 1
#define GL3_PROTOTYPES 1
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <chrono>

//#include <SDL2/SDL_opengl.h>
#include "GUI.h"
#include "Logging.h"

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    if ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
//            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
//            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
        std::cout << error << " (" << file << ":" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

GUI::GUI(Raster* raster)
	: raster(raster) {

	showEnhancedPPU = false;
	showDebuggerPPU = true;
	showDebuggerAPU = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		PrintError("SDL_Init failed: %s", SDL_GetError());
		throw std::runtime_error("SDL_Init failed");
	}

    if (showDebuggerAPU) {
        // Create software-rendering Window
        apuDebugWindow = SDL_CreateWindow("APU Debugger", 0, 0, 520, 828, SDL_WINDOW_ALLOW_HIGHDPI);
	    if (apuDebugWindow == nullptr) {
            PrintError("SDL_CreateWindow#1 failed: %s", SDL_GetError());
            throw std::runtime_error("SDL_CreateWindow#1 failed");
        }

        // This will create an OpenGL 2.1 context behind the scenes.
        apuDebugRenderer = SDL_CreateRenderer(apuDebugWindow, -1, SDL_RENDERER_PRESENTVSYNC);
	    if (apuDebugRenderer == nullptr) {
            PrintError("SDL_CreateRenderer failed: %s", SDL_GetError());
            throw std::runtime_error("SDL_CreateRenderer failed");
        }

        SDL_RaiseWindow(apuDebugWindow);
	
        // set logical size equal to window size
        // this allows for unscaled/nearest-neighbor pixel-accurate output
        // on hi-dpi screens where renderer output size != window size
        int w, h;
        SDL_GetWindowSize(apuDebugWindow, &w, &h);
        PrintInfo("APU Debug Window size: %d x %d", w, h);
        SDL_RenderSetLogicalSize(apuDebugRenderer, w, h);

        square1FFTTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 64);
        square1WaveformTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 64);
        square2FFTTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 64);
        square2WaveformTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 64);
        triangleFFTTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 64);
        triangleWaveformTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 64);
        noiseFFTTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 64);
        noiseWaveformTexture = SDL_CreateTexture(apuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 1024, 64);
    }

	if (showDebuggerPPU) {
		// Create software-rendering Window
		ppuDebugWindow = SDL_CreateWindow("NES - Debug", 0, 330, 1320, 564, SDL_WINDOW_ALLOW_HIGHDPI);
		if (ppuDebugWindow == nullptr) {
			PrintError("SDL_CreateWindow#1 failed: %s", SDL_GetError());
			throw std::runtime_error("SDL_CreateWindow#1 failed");
		}

		// This will create an OpenGL 2.1 context behind the scenes.
		ppuDebugRenderer = SDL_CreateRenderer(ppuDebugWindow, -1, SDL_RENDERER_PRESENTVSYNC);
		if (ppuDebugRenderer == nullptr) {
			PrintError("SDL_CreateRenderer failed: %s", SDL_GetError());
			throw std::runtime_error("SDL_CreateRenderer failed");
		}

		SDL_RendererInfo info;
		SDL_GetRendererInfo(ppuDebugRenderer, &info);
		PrintInfo("info.num_texture_formats = %d", info.num_texture_formats);
		for (int i = 0; i < info.num_texture_formats; i++) {
			PrintInfo("  %s = %d bytes/pixel", SDL_GetPixelFormatName(info.texture_formats[i]),
				SDL_BYTESPERPIXEL(info.texture_formats[i]));
		}

		finalTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
		patternTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 256);
		attributeTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
		paletteTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 32);
		nametableTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 512);
		backgroundMaskTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STREAMING, 256, 256);
		spriteMaskTexture = SDL_CreateTexture(ppuDebugRenderer, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STREAMING, 256, 256);

		if (finalTexture == nullptr) {
			PrintError("SDL_CreateTexture failed: %s", SDL_GetError());
			throw std::runtime_error("SDL_CreateTexture failed");
		}

		if (backgroundMaskTexture == nullptr) {
			PrintError("SDL_CreateTexture failed: %s", SDL_GetError());
			throw std::runtime_error("SDL_CreateTexture failed");
		}

		SDL_RaiseWindow(ppuDebugWindow);

		int w, h;

		// set logical size equal to window size
		// this allows for unscaled/nearest-neighbor pixel-accurate output
		// on hi-dpi screens where renderer output size != window size
		SDL_GetWindowSize(ppuDebugWindow, &w, &h);
		PrintInfo("Debug Window size: %d x %d", w, h);
		SDL_RenderSetLogicalSize(ppuDebugRenderer, w, h);

		//    SDL_GetWindowSize(glWindow, &w, &h);
		//    PrintInfo("OpenGL Window size: %d x %d", w, h);

		SDL_GL_GetDrawableSize(ppuDebugWindow, &w, &h);
		PrintInfo("Debug Window drawable size: %d x %d", w, h);

		//    SDL_GL_GetDrawableSize(glWindow, &w, &h);
		//    PrintInfo("OpenGL Window drawable size: %d x %d", w, h);
		//    glCheckError();

		SDL_GetRendererOutputSize(ppuDebugRenderer, &w, &h);
		PrintInfo("Debug Window Renderer output size: %d x %d", w, h);

		SDL_Rect rect;
		SDL_RenderGetViewport(ppuDebugRenderer, &rect);
		PrintInfo("Debug Window Renderer viewport: %d x %d @ %d, %d", rect.w, rect.h, rect.x, rect.y);

		SDL_RenderGetLogicalSize(ppuDebugRenderer, &w, &h);
		PrintInfo("Debug Window Renderer logical size: %d x %d", w, h);
	}

    if (showEnhancedPPU) {
        // Reconfigure preferred OpenGL settings: Profile = Core and Version = 3.1
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG) < 0) {
            PrintError("SDL_GL_SetAttribute failed: %s", SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0) {
            PrintError("SDL_GL_SetAttribute failed: %s", SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) < 0) {
            PrintError("SDL_GL_SetAttribute failed: %s", SDL_GetError());
        }
        if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) < 0) {
            PrintError("SDL_GL_SetAttribute failed: %s", SDL_GetError());
        }
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        // Create OpenGL Window and Context
        glWindow = SDL_CreateWindow("NES - Composite", 0, 0, 1320, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
        if (glWindow == nullptr) {
            PrintError("SDL_CreateWindow#2 failed: %s", SDL_GetError());
            throw std::runtime_error("SDL_CreateWindow#2 failed");
        }

        glContext = SDL_GL_CreateContext(glWindow);
        if (glContext == nullptr) {
            PrintError("SDL_GL_CreateContext failed: %s", SDL_GetError());
        }

		// initialize OpenGL driver
		GLenum glewErr = glewInit();
		if (glewErr != GLEW_OK) {
			std::cout << "glew error: " << glewGetErrorString(glewErr) << std::endl;
		}
		
        // Dump OpenGL versioning info

        PrintInfo("OpenGL Vendor: %s", glGetString(GL_VENDOR));
        PrintInfo("OpenGL Renderer: %s", glGetString(GL_RENDERER));
        PrintInfo("OpenGL Version: %s", glGetString(GL_VERSION));
        PrintInfo("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

//    glRenderTarget = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 256, 256);
//    if (glRenderTarget == nullptr) {
//        PrintError("SDL_CreateTexture failed: %s", SDL_GetError());
//        throw std::runtime_error("SDL_CreateTexture failed");
//    }

//    SDL_WM_SetCaption( "TTF Test", NULL );

        //SDL_GL_SetSwapInterval(1);
        //int swapInterval = SDL_GL_GetSwapInterval();
        //PrintInfo("VSync interval: %d", swapInterval);

        SDL_RaiseWindow(glWindow);

        createQuad();
        createRenderTargets();
        createShaders();
    }

    if (TTF_Init() < 0) {
        PrintInfo("Failed to initialize sdl2_ttf library: %s", SDL_GetError());
        exit(1);
    }

    font = TTF_OpenFont("fonts/visitor2.ttf", 19);
    if (font == nullptr) {
        PrintError("Failed to open font: %s", TTF_GetError());
        exit(1);
    }

    TTF_SetFontHinting(font, TTF_HINTING_MONO);
    TTF_SetFontKerning(font, 0);
    TTF_SetFontOutline(font, 0);
}

void uploadTexture(SDL_Texture *texture, void *ptr, int pitch) {
    if (SDL_UpdateTexture(texture, nullptr, ptr, pitch) < 0) {
        PrintError("Failed to SDL_UpdateTexture(): %s", SDL_GetError());
    }
}

void renderTexture(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect rect) {
    if (SDL_RenderCopy(renderer, texture, nullptr, &rect) < 0) {
        PrintError("Failed to SDL_RenderCopy(): %s", SDL_GetError());
    }
}

void
GUI::render() {
    if (showDebuggerPPU) {
        auto now = std::chrono::high_resolution_clock::now();
        // render into debug window
        renderDebugViews();
        auto span = std::chrono::high_resolution_clock::now() - now;
        //PrintInfo("Rendering PPU Debugger took %d msec", std::chrono::duration_cast<std::chrono::milliseconds>(span));
    }

    if (showEnhancedPPU) {
        auto now = std::chrono::high_resolution_clock::now();
        // render into opengl window
        renderPostProcessing();
        auto span = std::chrono::high_resolution_clock::now() - now;
        PrintInfo("Rendering Post-Processing took %d msec", std::chrono::duration_cast<std::chrono::milliseconds>(span));
    }

    if (showDebuggerAPU) {
        auto now = std::chrono::high_resolution_clock::now();
        // clear screen
        SDL_SetRenderDrawColor(apuDebugRenderer, 0, 0, 0, 255);
        SDL_RenderClear(apuDebugRenderer);

        int padding = 15;
        int height = 64;

        uploadTexture(square1FFTTexture, raster->square1FFT, 512 * 4);
        renderTexture(apuDebugRenderer, square1FFTTexture, SDL_Rect{4, 10, 512, height});
        drawText(apuDebugRenderer, "Pulse 1 - FFT", 4, 0);

        uploadTexture(square1WaveformTexture, raster->square1Waveform, 1024 * 4);
        renderTexture(apuDebugRenderer, square1WaveformTexture, SDL_Rect{4, height + padding * 2, 512, height});
        drawText(apuDebugRenderer, "Pulse 1 - Waveform", 4, height + padding);

        uploadTexture(square2FFTTexture, raster->square2FFT, 512 * 4);
        renderTexture(apuDebugRenderer, square2FFTTexture, SDL_Rect{4, height * 2 + padding * 3, 512, height});
        drawText(apuDebugRenderer, "Pulse 2 - FFT", 4, height * 2 + padding * 2);

        uploadTexture(square2WaveformTexture, raster->square2Waveform, 1024 * 4);
        renderTexture(apuDebugRenderer, square2WaveformTexture, SDL_Rect{4, height * 3 + padding * 4, 512, height});
        drawText(apuDebugRenderer, "Pulse 2 - Waveform", 4, height * 3 + padding * 3);

        uploadTexture(triangleFFTTexture, raster->triangleFFT, 512 * 4);
        renderTexture(apuDebugRenderer, triangleFFTTexture, SDL_Rect{4, height * 4 + padding * 5, 512, height});
        drawText(apuDebugRenderer, "Triangle - FFT", 4, height * 4 + padding * 4);

        uploadTexture(triangleWaveformTexture, raster->triangleWaveform, 1024 * 4);
        renderTexture(apuDebugRenderer, triangleWaveformTexture, SDL_Rect{4, height * 5 + padding * 6, 512, height});
        drawText(apuDebugRenderer, "Triangle - Waveform", 4, height * 5 + padding * 5);

        uploadTexture(noiseFFTTexture, raster->noiseFFT, 512 * 4);
        renderTexture(apuDebugRenderer, noiseFFTTexture, SDL_Rect{4, height * 6 + padding * 7, 512, height});
        drawText(apuDebugRenderer, "Noise - FFT", 4, height * 6 + padding * 6);

        uploadTexture(noiseWaveformTexture, raster->noiseWaveform, 1024 * 4);
        renderTexture(apuDebugRenderer, noiseWaveformTexture, SDL_Rect{4, height * 7 + padding * 8, 512, height});
        drawText(apuDebugRenderer, "Noise - Waveform", 4, height * 7 + padding * 7);

        // flip
        SDL_RenderPresent(apuDebugRenderer);
        auto span = std::chrono::high_resolution_clock::now() - now;
        //PrintInfo("Rendering APU Debugger took %d msec", std::chrono::duration_cast<std::chrono::milliseconds>(span));
    }
}

void GUI::renderDebugViews() {// store internal buffers as a texture
    uploadTexture(finalTexture, raster->screenBuffer, 256 * 4);
    uploadTexture(patternTexture, raster->patternTable, 128 * 4);
    uploadTexture(attributeTexture, raster->attributeTable, 256 * 4);
    uploadTexture(paletteTexture, raster->palette, 256 * 4);
    uploadTexture(nametableTexture, raster->nametables, 512 * 4);

    // TODO: fix CPU based format conversion overhead for these two textures
    uploadTexture(backgroundMaskTexture, raster->backgroundMask, 256);
    uploadTexture(spriteMaskTexture, raster->spriteMask, 256);

    // clear screen
    SDL_SetRenderDrawColor(ppuDebugRenderer, 10, 10, 10, 255);
    SDL_RenderClear(ppuDebugRenderer);

    // draw textures
    renderTexture(ppuDebugRenderer, finalTexture, SDL_Rect{0, 0, 256, 256});
    drawText(ppuDebugRenderer, "Render", 0, 0);

    renderTexture(ppuDebugRenderer, patternTexture, SDL_Rect{266, 0, 256, 512});
    drawText(ppuDebugRenderer, "Pattern Table", 266, 0);

    renderTexture(ppuDebugRenderer, attributeTexture, SDL_Rect{0, 266, 256, 256});
    drawText(ppuDebugRenderer, "Attributes Table", 0, 256);

    renderTexture(ppuDebugRenderer, paletteTexture, SDL_Rect{0, 532, 256, 32});
    drawText(ppuDebugRenderer, "Palette Map", 0, 522);

    renderTexture(ppuDebugRenderer, nametableTexture, SDL_Rect{532, 0, 512, 512});
    drawText(ppuDebugRenderer, "Nametable $2000", 532, 0);
    drawText(ppuDebugRenderer, "Nametable $2400", 788, 0);
    drawText(ppuDebugRenderer, "Nametable $2400", 788, 256);
    drawText(ppuDebugRenderer, "Nametable $2800", 532, 256);

    renderTexture(ppuDebugRenderer, backgroundMaskTexture, SDL_Rect{1054, 0, 256, 256});
    drawText(ppuDebugRenderer, "Background Mask", 1054, 0);

    renderTexture(ppuDebugRenderer, spriteMaskTexture, SDL_Rect{1054, 266, 256, 256});
    drawText(ppuDebugRenderer, "Sprite Mask", 1054, 256);

    // flip to screen
    SDL_RenderPresent(ppuDebugRenderer);
}

/**
 * Draws static text on the screen. Caches aggressively.
 */
void GUI::drawText(SDL_Renderer *renderer, const char *message, const int posX, const int posY) {
    SDL_Texture *textTexture = generateTextureLabel(message, renderer);

    auto textRect = SDL_Rect{posX, posY};
    SDL_QueryTexture(textTexture, nullptr, nullptr, &textRect.w, &textRect.h);
    SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
}

SDL_Texture *GUI::generateTextureLabel(const char *message, SDL_Renderer *renderer) {
    std::hash<std::string> hasher;

    auto hash = hasher(message);

    // return a cached texture if available
    auto cachedEntry = labelCache.find(hash);
    if (cachedEntry != labelCache.end()) {
        return cachedEntry->second;
    }

    // otherwise generate a new one
    SDL_Color color = {255, 255, 255, 255};
    SDL_Color bg = {0, 0, 0, 255};
    SDL_Surface *textSurface = TTF_RenderText_Shaded(font, message, color, bg);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    labelCache[hash] = textTexture;

    return textTexture;
}

GUI::~GUI() {
    if (showEnhancedPPU) {
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(glWindow);
    }
    if (showDebuggerPPU) {
        SDL_DestroyWindow(ppuDebugWindow);
    }
    SDL_Quit();
}

void GUI::createQuad() {
    glGenTextures(1, &rt);
    glBindTexture(GL_TEXTURE_2D, rt);
    glCheckError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create a Vertex Buffer Object and copy the vertex data to it
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
            0, 256, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
            256, 256, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
            256, 0, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
            0, 0, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, // Bottom-left
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // define layout of the vertex buffer object. related vertex shaders must define same input layout.
    GLuint posAttrib = 0;
    GLuint colAttrib = 1;
    GLuint texCoord = 2;
    glEnableVertexAttribArray(posAttrib);
    glCheckError();
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), nullptr);
    glCheckError();

    glEnableVertexAttribArray(colAttrib);
    glCheckError();
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *) (2 * sizeof(GLfloat)));
    glCheckError();

    glEnableVertexAttribArray(texCoord);
    glCheckError();
    glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *) (5 * sizeof(GLfloat)));
    glCheckError();

    // Create an element array
    glGenBuffers(1, &ebo);

    GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

//    glBindVertexArray(0);
}

GLuint loadShader(GLenum type, const char *shaderName, const char *shaderFilePath);

void checkShaderCompilationError(const char *shaderName, GLuint shaderId);

void GUI::createShaders() {
    GLuint passthruVertex = loadShader(GL_VERTEX_SHADER, "vertex shader", "shaders/passthru.vert");
    GLuint passthroughFragment = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "shaders/passthru.frag");
    passThruShader = createProgram(passthruVertex, passthroughFragment);

    GLuint gBufferVertex = loadShader(GL_VERTEX_SHADER, "vertex shader", "shaders/create-gbuffer.vert");
    GLuint gBufferFragment = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "shaders/create-gbuffer.frag");
    createGBufferShader = createProgram(gBufferVertex, gBufferFragment);

    GLuint blurVertex = loadShader(GL_VERTEX_SHADER, "vertex shader", "shaders/blur.vert");
    GLuint blurFragment = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "shaders/blur.frag");
    blurShader = createProgram(blurVertex, blurFragment);

    GLuint composeVertex = loadShader(GL_VERTEX_SHADER, "vertex shader", "shaders/compose.vert");
    GLuint composeFragment = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "shaders/compose.frag");
    composeShader = createProgram(composeVertex, composeFragment);

    GLint numAttributes;
    glGetProgramiv(composeShader, GL_ACTIVE_ATTRIBUTES, &numAttributes);
    PrintInfo("num of attributes: %d", numAttributes);
    GLenum type;
    GLint size;
    char name[256];
    for (int i = 0; i < numAttributes; i++) {
        glGetActiveAttrib(composeShader, i, 256, nullptr, &size, &type, name);
        PrintInfo("  attribute=%s size=%d", name, size);
    }
}

GLuint GUI::createProgram(GLuint vertexShader,
                          GLuint fragmentShader) const {// Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glCheckError();
    glAttachShader(shaderProgram, fragmentShader);
    glCheckError();
    glProgramParameteri(shaderProgram, GL_PROGRAM_SEPARABLE, true);
    glCheckError();

    // link the shaders together
    glLinkProgram(shaderProgram);
    glCheckError();

    int programLinked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &programLinked);
    if (programLinked == 0) {
        int maxLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        char *errorLog = new char[maxLength];
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, errorLog);
        PrintError("Failed to link shaders into program: %s", errorLog);
        delete[] errorLog;
        exit(1);
    }
    return shaderProgram;
}

void GUI::createRenderTargets() {
    // configure render targets
    glGenTextures(4, renderTargets);

    // albedo
    glBindTexture(GL_TEXTURE_2D, renderTargets[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // bloom ping-pong
    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // bloom ping-pong
    glBindTexture(GL_TEXTURE_2D, renderTargets[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // final
    glBindTexture(GL_TEXTURE_2D, renderTargets[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GUI::renderPostProcessing() {
    // switch to OpenGL window and context
    SDL_GL_MakeCurrent(glWindow, glContext);

    glClearColor(1.0, 0.8, 0.3, 1.0); // debug color
    glClear(GL_COLOR_BUFFER_BIT);

    GLsizei w, h;
    SDL_GetWindowSize(glWindow, &w, &h);
    glViewport(0, 0, w, h);
    glm::mat4 projection = glm::ortho(0.0f, (float) w, 0.0f, (float) h, -10.0f, 10.0f);

    glUseProgram(createGBufferShader);
    glCheckError();

    glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
    );

    glm::mat scale = glm::scale(glm::vec3(1));

    glm::mat4 viewProjection = projection * scale * view;
    glUniformMatrix4fv(glGetUniformLocation(createGBufferShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    /////////////////////////////////////////////////
    // create G-Buffer and upload framebuffers from CPU to GPU

    // reset framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // write to multiple render-targets that make up the g-buffer
    // render standard albedo into RT0
    // render bright pixels into RT1
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderTargets[1], 0);
    GLenum renderTargetAliases[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, renderTargetAliases);
    glCheckError();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        PrintError("Framebuffer error");
        glCheckError();
        exit(1);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rt);
//    auto now = std::chrono::high_resolution_clock::now();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, raster->screenBuffer);
//    glFlush();
//    auto span = std::chrono::high_resolution_clock::now() - now;
//    PrintInfo("glTexImage2D() took %d msec", std::chrono::duration_cast<std::chrono::milliseconds>(span));
    glCheckError();

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    /////////////////////////////////////////////////
    // Bloom

    // read bright pixels from RT1
    // render horizontal blur into RT2
    GLenum singleTargetAlias[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, singleTargetAlias);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[2], 0);
    glUseProgram(blurShader);
    glUniformMatrix4fv(glGetUniformLocation(blurShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform1i(glGetUniformLocation(blurShader, "horizontalBlur"), 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glCheckError();
    glUniform1i(glGetUniformLocation(blurShader, "texSampler"), 0);
    glCheckError();
    glBindVertexArray(vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glCheckError();

    // read horizontal blur from RT2
    // render vertical blur into RT1
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[1], 0);
    glUniform1i(glGetUniformLocation(blurShader, "horizontalBlur"), 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[2]);
    glCheckError();
    glUniform1i(glGetUniformLocation(blurShader, "texSampler"), 0);
    glCheckError();
    glBindVertexArray(vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glCheckError();

    /////////////////////////////////////////////////
    // Compose final image from g-buffer

    // read albedo from RT0
    // read bloom from RT1
    // render final composite into RT3
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[3], 0);
    glUseProgram(composeShader);
    glUniformMatrix4fv(glGetUniformLocation(composeShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glUniform1i(glGetUniformLocation(composeShader, "albedoSampler"), 0);
    glUniform1i(glGetUniformLocation(composeShader, "bloomSampler"), 1);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glCheckError();

    /////////////////////////////////////////////////
    // Draw render-targets on screen

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // create projection matrix based on window-size (same width x height on low-dpi and high-dpi screens)
    // set OpenGL Viewport to framebuffer size (will vary based on screen dpi, eg: 2x higher on retina screens)
    // this gets us really simple dpi-invariant scaling for consistent rendering on screen
    // without changing shaders or any complex math
    SDL_GetWindowSize(glWindow, &w, &h);
    projection = glm::ortho(0.0f, (float) w, 0.0f, (float) h, -10.0f, 10.0f);
    SDL_GL_GetDrawableSize(glWindow, &w, &h);
    glViewport(0, 0, w, h);

    glUseProgram(passThruShader);
    glCheckError();

    // draw RT0
    view = glm::lookAt(
            glm::vec3(-512, -256, 1),
            glm::vec3(-512, -256, 0),
            glm::vec3(0, 1, 0)
    );

    scale = glm::scale(glm::vec3(1));

    viewProjection = projection * scale * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[0]);
    glUniform1i(glGetAttribLocation(passThruShader, "texSampler"), 0);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // draw RT1
    view = glm::lookAt(
            glm::vec3(-768, -256, 1),
            glm::vec3(-768, -256, 0),
            glm::vec3(0, 1, 0)
    );

    scale = glm::scale(glm::vec3(1));

    viewProjection = projection * scale * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glUniform1i(glGetAttribLocation(passThruShader, "texSampler"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // draw RT2
    view = glm::lookAt(
            glm::vec3(-1024, -256, 1),
            glm::vec3(-1024, -256, 0),
            glm::vec3(0, 1, 0)
    );

    scale = glm::scale(glm::vec3(1));

    viewProjection = projection * scale * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[2]);
    glUniform1i(glGetAttribLocation(passThruShader, "texSampler"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // draw RT3
    view = glm::lookAt(
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
    );

    scale = glm::scale(glm::vec3(2));

    viewProjection = projection * scale * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[3]);
    glUniform1i(glGetUniformLocation(passThruShader, "texSampler"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glFlush();
    SDL_GL_SwapWindow(glWindow);
}

GLuint loadShader(GLenum type, const char *shaderName, const char *shaderFilePath) {
    char *buffer = new char[100000];
    FILE *file = fopen(shaderFilePath, "r");
    if (file == nullptr) {
        PrintError("Failed to load shader=%s from file=%s", shaderName, shaderFilePath);
    }
    int len = fread(buffer, 1, 100000, file);
    fclose(file);

    PrintInfo("Read shader from file=%s (%d bytes)", shaderFilePath, len);

    // Create and compile the vertex shader
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &buffer, &len);
    glCheckError();
    glCompileShader(shader);
    glCheckError();
    checkShaderCompilationError(shaderName, shader);
    return shader;
}

void checkShaderCompilationError(const char *shaderName, GLuint shaderId) {
    int shaderCompiled;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &shaderCompiled);
    if (!shaderCompiled) {
        int maxLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
        char *errorLog = new char[maxLength];
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, errorLog);
        PrintError("Failed to compile shader=%s with error=%s", shaderName, errorLog);
        delete[] errorLog;
    }
}
