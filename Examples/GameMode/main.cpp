#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

enum GameMode { MAIN_MENU, GAME_LEVEL, GAME_OVER };
GameMode mode = MAIN_MENU;

struct GameState {
    Entity player;
    Entity enemies[10];
    Entity items[5];
    int score;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

float lastTicks = 0;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

// ######################################################################
// Input Methods
// ######################################################################

void ProcessInputMainMenu() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
        }
    }
    
}

void ProcessInputGameLevel() {
}

void ProcessInputGameOver() {
}

void ProcessInput() {
    switch (mode) {
        case MAIN_MENU:
            ProcessInputMainMenu();
            break;

        case GAME_LEVEL:
            ProcessInputGameLevel();
            break;

        case GAME_OVER:
            ProcessInputGameOver();
            break;
    }
}

// ######################################################################
// Update Methods
// ######################################################################

void UpdateMainMenu(float deltaTime) {
}

void UpdateGameLevel(float deltaTime) {
}

void UpdateGameOver(float deltaTime) {
}

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    switch (mode) {
        case MAIN_MENU:
            UpdateMainMenu(deltaTime);
            break;

        case GAME_LEVEL:
            UpdateGameLevel(deltaTime);
            break;

        case GAME_OVER:
            UpdateGameOver(deltaTime);
            break;
    }
}

// ######################################################################
// Render Methods
// ######################################################################

void RenderMainMenu() {
}

void RenderGameLevel() {
}

void RenderGameOver() {
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
   switch (mode) {
       case MAIN_MENU:
           RenderMainMenu();
           break;

       case GAME_LEVEL:
           RenderGameLevel();
           break;

       case GAME_OVER:
           RenderGameOver();
           break;
   }
 
    SDL_GL_SwapWindow(displayWindow);
}

// ######################################################################

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
