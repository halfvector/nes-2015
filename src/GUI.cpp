#define GL_SILENCE_DEPRECATION 1
#define GL3_PROTOTYPES 1

#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL_opengl_glext.h>


//#include <SDL2/SDL_opengl.h>
#include "GUI.h"
#include "Logging.h"

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
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

GUI::GUI(Raster *raster)
        : raster(raster) {

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        PrintError("SDL_Init failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }

    // Create software-rendering Window
    debugWindow = SDL_CreateWindow("NES - Debug", 0, 330, 1320, 564, SDL_WINDOW_SHOWN);
    if (debugWindow == nullptr) {
        PrintError("SDL_CreateWindow#1 failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow#1 failed");
    }

    // This will create an OpenGL 2.1 context behind the scenes.
    renderer = SDL_CreateRenderer(debugWindow, -1, SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        PrintError("SDL_CreateRenderer failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_CreateRenderer failed");
    }

    finalTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    patternTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 128, 256);
    attributeTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    paletteTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 32);
    nametableTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 512, 512);
    backgroundMaskTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 256, 256);
    spriteMaskTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 256, 256);

    if (finalTexture == nullptr) {
        PrintError("SDL_CreateTexture failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_CreateTexture failed");
    }

    if (backgroundMaskTexture == nullptr) {
        PrintError("SDL_CreateTexture failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_CreateTexture failed");
    }

    // Reconfigure preferred OpenGL settings: Profile = Core and Version = 3.1
    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG) <
        0) {
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
    glWindow = SDL_CreateWindow("NES - Composite", 0, 0, 1320, 256, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (glWindow == nullptr) {
        PrintError("SDL_CreateWindow#2 failed: %s", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow#2 failed");
    }

    glContext = SDL_GL_CreateContext(glWindow);
    if (glContext == nullptr) {
        PrintError("SDL_GL_CreateContext failed: %s", SDL_GetError());
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

    SDL_GL_SetSwapInterval(1);
    int swapInterval = SDL_GL_GetSwapInterval();
    PrintInfo("VSync interval: %d", swapInterval);

    SDL_RaiseWindow(debugWindow);
    SDL_RaiseWindow(glWindow);

    createQuad();
    createRenderTargets();
    createShaders();
}

void
GUI::render() {
    // blit software rasterized textures to window

    if (SDL_UpdateTexture(finalTexture, nullptr, raster->screenBuffer, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(final) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(patternTexture, nullptr, raster->patternTable, 128 * 4) < 0) {
        PrintError("SDL_UpdateTexture(patternTable) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(attributeTexture, nullptr, raster->attributeTable, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(attributeTable) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(paletteTexture, nullptr, raster->palette, 256 * 4) < 0) {
        PrintError("SDL_UpdateTexture(palette) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(nametableTexture, nullptr, raster->nametables, 512 * 4) < 0) {
        PrintError("SDL_UpdateTexture(nametableTexture) failed: %s\n", SDL_GetError());
    }

    // disable rendering masks for performance reasons
    if (SDL_UpdateTexture(backgroundMaskTexture, nullptr, raster->backgroundMask, 256) < 0) {
        PrintError("SDL_UpdateTexture(backgroundMaskTexture) failed: %s\n", SDL_GetError());
    }

    if (SDL_UpdateTexture(spriteMaskTexture, nullptr, raster->spriteMask, 256) < 0) {
        PrintError("SDL_UpdateTexture(spriteMaskTexture) failed: %s\n", SDL_GetError());
    }

    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
    SDL_RenderClear(renderer);

    // copy textures into render surface
    auto finalRect = SDL_Rect{
            0, 0, 256, 256
    };
    SDL_RenderCopy(renderer, finalTexture, nullptr, &finalRect);

    // copy textures into render surface
    auto patternRect = SDL_Rect{
            266, 0, 256, 512
    };
    SDL_RenderCopy(renderer, patternTexture, nullptr, &patternRect);

    auto attributesRect = SDL_Rect{
            0, 266, 256, 256
    };
    SDL_RenderCopy(renderer, attributeTexture, nullptr, &attributesRect);

    auto paletteRect = SDL_Rect{
            0, 532, 256, 32
    };
    SDL_RenderCopy(renderer, paletteTexture, nullptr, &paletteRect);

    auto nametableRect = SDL_Rect{
            532, 0, 512, 512
    };
    SDL_RenderCopy(renderer, nametableTexture, nullptr, &nametableRect);

    auto backgroundMaskRect = SDL_Rect{
            1054, 0, 256, 256
    };
    SDL_RenderCopy(renderer, backgroundMaskTexture, nullptr, &backgroundMaskRect);

    auto spriteMaskRect = SDL_Rect{
            1054, 266, 256, 256
    };
    SDL_RenderCopy(renderer, spriteMaskTexture, nullptr, &spriteMaskRect);

//     present surface
    SDL_RenderPresent(renderer);


    // render OpenGL window
//    if(SDL_SetRenderTarget(glRenderer, glRenderTarget) < 0) {
//        PrintError("SDL_SetRenderTarget(final) failed: %s\n", SDL_GetError());
//    }
//    SDL_SetRenderDrawColor(glRenderer, 100, 255, 200, 255);
//    SDL_RenderClear(glRenderer);

    // reset OpenGL Context

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    int w, h;
    SDL_GetWindowSize(glWindow, &w, &h);
    glViewport(0, 0, w, h);
//    glClearDepth(1.0);
    SDL_GL_MakeCurrent(glWindow, glContext);
    glClearColor(1.0, 0.8, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

//    glGenBuffers(1, &pbo);
//    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
//    glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo)

    renderQuad();

    glFlush();
    SDL_GL_SwapWindow(glWindow);
}

GUI::~GUI() {
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(debugWindow);
    SDL_DestroyWindow(glWindow);
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
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, "vertex shader", "../../src/shaders/passthru.vert");
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "../../src/shaders/passthru.frag");
    passThruShader = createProgram(vertexShader, fragmentShader);


    GLuint gBufferVertex = loadShader(GL_VERTEX_SHADER, "vertex shader", "../../src/shaders/create-gbuffer.vert");
    GLuint gBufferFragment = loadShader(GL_FRAGMENT_SHADER, "fragment shader", "../../src/shaders/create-gbuffer.frag");
    createGBufferShader = createProgram(gBufferVertex, gBufferFragment);
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
    glGenTextures(2, renderTargets);

    PrintInfo("renderTargets[0] = %d", renderTargets[0]);
    PrintInfo("renderTargets[1] = %d", renderTargets[1]);

    glBindTexture(GL_TEXTURE_2D, renderTargets[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargets[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderTargets[1], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GUI::renderQuad() {
    int w, h;
    SDL_GetWindowSize(glWindow, &w, &h);
    glm::mat4 projection = glm::ortho(0.0f, (float) w, 0.0f, (float) h, -10.0f, 10.0f);

    glUseProgram(createGBufferShader);
    glCheckError();
    glUniform1i(glGetAttribLocation(createGBufferShader, "texSampler"), 0);
    glCheckError();

    glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
    );

    glm::mat4 viewProjection = projection * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    // read from primary framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    // write to gbuffer and its multiple render-targets
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
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
    glCheckError();
//    auto now = std::chrono::high_resolution_clock::now();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, raster->screenBuffer);
//    glFlush();
//    auto span = std::chrono::high_resolution_clock::now() - now;
//    PrintInfo("glTexImage2D() took %d msec", std::chrono::duration_cast<std::chrono::milliseconds>(span));
    glCheckError();

    glBindVertexArray(vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glCheckError();
    glBindVertexArray(0);
    glCheckError();

    // write to primary framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // read from gbuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

    glBindVertexArray(vao);

    // draw render-targets to screen
    glUseProgram(passThruShader);
    glCheckError();

    // draw RT #1
    view = glm::lookAt(
            glm::vec3(0, 0, 1),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
    );

    viewProjection = projection * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));

    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, renderTargets[0]);
    glCheckError();
    glUniform1i(glGetAttribLocation(passThruShader, "texSampler"), 0);
    glCheckError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glCheckError();

    // shift over to the size to draw RT #2
    view = glm::lookAt(
            glm::vec3(-256, 0, 1),
            glm::vec3(-256, 0, 0),
            glm::vec3(0, 1, 0)
    );

    viewProjection = projection * view;
    glUniformMatrix4fv(glGetUniformLocation(passThruShader, "MVP"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glCheckError();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderTargets[1]);
    glUniform1i(glGetAttribLocation(passThruShader, "texSampler"), 0);
    glCheckError();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glCheckError();
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
