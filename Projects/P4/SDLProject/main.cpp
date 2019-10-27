#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "stb_image.h"
#include "Entity.h"


//declare some global variables for program
SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

//variables for game state
bool gameIsRunning = true;
bool gameWon = false;
bool gameLost = false;

// define platform count for game
#define PLATFORM_COUNT 1

// define obstacle count
#define OBS_COUNT 10

// define gravity for game
#define GRAVITY -0.15f


//define GameState object - will keep track of objects in the game
struct GameState {
    // declare player
    Entity player;
    
    //declare platforms
    Entity platforms[PLATFORM_COUNT];
    
    // declare obsticles
    Entity obsticles[OBS_COUNT];
    
    //declare game banners - win & lost
    Entity banners[2];
};


//declare GameState object
GameState state;


//define load texture function
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

// randomly generates and returns coordinate
float randomCoord(float min, float max) {
    return (min + 1) + (((float) rand()) / (float) RAND_MAX) * (max - (min + 1));
}

//define initialize funciton
void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Lander - kp1732", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // set size of viewport
    glViewport(0, 0, 640, 480);
    
    // load textured shader program, to be used by elements containing textures
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    // initialize player attributes
    state.player.entityType = PLAYER;
    state.player.IsStatic = false;
    state.player.position = glm::vec3(-4.5f, 3.5f, 0.0f);
    state.player.scale = 0.5f;
    state.player.height *= state.player.scale;
    state.player.width *= state.player.scale;
    state.player.acceleration = glm::vec3(0.0f, GRAVITY, 0.0f); // initialize acceleration for player
    //initialize player textures
    state.player.textureID = LoadTexture("ship.png");
    float player_vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float player_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(state.player.vertices, player_vertices, sizeof(state.player.vertices));
    std::memcpy(state.player.texCoords, player_texCoords, sizeof(state.player.texCoords));

    
    // initialize platform attributes
    state.platforms[0].entityType = PLATFORM;
    state.platforms[0].IsStatic = true;
    state.platforms[0].height = 0.15f;
    state.platforms[0].width = 0.5f;
    state.platforms[0].position = glm::vec3(4.5f, -3.7f, 0.0f);
    // initialize platform textures
    state.platforms[0].textureID = LoadTexture("platform.png");
    float platform_vertices[]  = { -0.5, -0.1, 0.5, -0.1, 0.5, 0.1, -0.5, -0.1, 0.5, 0.1, -0.5, 0.1 };
    float platform_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(state.platforms[0].vertices, platform_vertices, sizeof(state.platforms[0].vertices));
    std::memcpy(state.platforms[0].texCoords, platform_texCoords, sizeof(state.platforms[0].texCoords));
    
    
    // loop through blocks and initialize each
    GLuint blockTexID = LoadTexture("block.png");
    for (int i = 0; i < OBS_COUNT; i++) {
        // attributes
        state.obsticles[i].entityType = BLOCK;
        state.obsticles[i].IsStatic = true;
        state.obsticles[i].scale = 0.5f;
        state.obsticles[i].height *= state.obsticles[i].scale;
        state.obsticles[i].width *= state.obsticles[i].scale;
        
        //set random position for each obsticle block
        state.obsticles[i].position.x = randomCoord(-4.5f, 4.5f);
        state.obsticles[i].position.y = randomCoord(-3.5f, 3.5f);
        
        //texture
        state.obsticles[i].textureID = blockTexID;
        float block_vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float block_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        std::memcpy(state.obsticles[i].vertices, block_vertices, sizeof(state.obsticles[i].vertices));
        std::memcpy(state.obsticles[i].texCoords, block_texCoords, sizeof(state.obsticles[i].texCoords));
    }
    
    
    // initialize win banner attributes
    state.banners[0].IsStatic = true;
    // initialize win banner textures
    state.banners[0].textureID = LoadTexture("win.png");
    float banner1_vertices[]  = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner1_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(state.banners[0].vertices, banner1_vertices, sizeof(state.banners[0].vertices));
    std::memcpy(state.banners[0].texCoords, banner1_texCoords, sizeof(state.banners[0].texCoords));
    
    
    // initialize lose banner attributes
    state.banners[1].IsStatic = true;
    // initialize lose banner textures
    state.banners[1].textureID = LoadTexture("lost.png");
    float banner2_vertices[]  = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner2_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(state.banners[1].vertices, banner2_vertices, sizeof(state.banners[1].vertices));
    std::memcpy(state.banners[1].texCoords, banner2_texCoords, sizeof(state.banners[1].texCoords));

    
    // define view, model and projection matricies
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    //set matricies
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    
    // tell GL which shader programs to use
    glUseProgram(program.programID);
    
    // enable blending and set blend function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // set the background color of window
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}


//define process input function
void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                
            // if quit event
            case SDL_QUIT:
            
            // if windows close event
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
            
            // if key pressed
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {

                    // space is pressed
                    case SDLK_SPACE:
                        // exit game
                        gameIsRunning = false;
                        break;
                }
                break;
        }
    }
    
    // Check for pressed/held keys below
    const Uint8 *keys = SDL_GetKeyboardState(NULL); //stores current state of keyboard
    
    //reset player acceleration each time or else player keeps moving
    state.player.acceleration.x = 0;
    state.player.acceleration.y = GRAVITY;
    
    // check for arror input, lock player rotations to 90 degrees
    if (keys[SDL_SCANCODE_LEFT])
    {
        // set x acceleration
        state.player.acceleration.x = GRAVITY;
    }
    else if  (keys[SDL_SCANCODE_RIGHT])
    {
        // set x acceleration
        state.player.acceleration.x = GRAVITY * -1;
    }
    else if  (keys[SDL_SCANCODE_UP])
    {
        // set y acceleration
        state.player.acceleration.y = GRAVITY * -1;
        
    }
}


//define variables for update function
#define FIXED_TIMESTEP 0.0166666f // represents 1/60th of a second
float lastTicks = 0;
float accumulator = 0.0f;


//define update function
void Update() {
    
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }
    
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player.Update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        state.player.Update(FIXED_TIMESTEP, state.obsticles, OBS_COUNT);
        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;

    
    // check game for win/loss
    if (state.player.lastCollision == PLATFORM) {
        gameWon = true;
    }
    else if (state.player.lastCollision == WALL or state.player.lastCollision == BLOCK) {
        gameLost = true;
    }
}


// define render function
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!gameWon and !gameLost) {
        // render player
        state.player.Render(&program);
        // render platform
        state.platforms[0].Render(&program);
        // render obsticle blocks
        for (Entity block : state.obsticles) {
            block.Render(&program);
        }
    }
    else if (gameWon) {
        state.banners[0].Render(&program);
    }
    else {
        state.banners[1].Render(&program);
    }

    // swap window and display next frame
    SDL_GL_SwapWindow(displayWindow);
}


//define shutdown function
void Shutdown() {
    SDL_Quit();
}


//define main function
int main(int argc, char* argv[]) {
    Initialize();
    
    //main game loop
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
