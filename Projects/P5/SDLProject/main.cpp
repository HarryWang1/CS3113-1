#define GL_SILENCE_DEPRECATION
#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
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
glm::vec3 startPosition = glm::vec3(-4, 3, 0);


// define platform count for game
#define PLATFORM_COUNT 16


// define enemy count for game
#define ENEMY_COUNT 3

// define level count
#define LEVELS 3


// define live enemy count for game and helper flag
bool start = true;
int liveCount = 0;


//define GameState object - will keep track of objects in the game
struct GameState {
    // player
    Entity player;

    // game platforms
    Entity platforms[PLATFORM_COUNT];

    // enemies
    Entity enemies[ENEMY_COUNT];

    // banners - win & lost
    Entity banners[2];
};


//declare GameState object
GameState states[LEVELS];


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


// define initialize function
void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music* music;
    music = Mix_LoadMUS("bgMusic.wav");
    Mix_PlayMusic(music, -1);
    displayWindow = SDL_CreateWindow("AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    // load texture shader
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    // initialize player attributes
    states[0].player.entityType = PLAYER;
    states[0].player.isStatic = false;
    states[0].player.width = 1.0f;
    states[0].player.position = states[0].player.startPosition;
    states[0].player.sensorLeft = glm::vec3(states[0].player.position.x + 0.6f, states[0].player.position.y - 0.6f, 0);
    states[0].player.sensorRight = glm::vec3(states[0].player.position.x - 0.6f, states[0].player.position.y - 0.6f, 0);
    states[0].player.acceleration = glm::vec3(0, -9.81f, 0);
    states[0].player.textures[0] = LoadTexture("player_left.png");
    states[0].player.textures[1] = LoadTexture("player_right.png");
    states[0].player.textureID = states[0].player.textures[1];
    float player_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float player_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(states[0].player.vertices, player_vertices, sizeof(states[0].player.vertices));
    std::memcpy(states[0].player.texCoords, player_texCoords, sizeof(states[0].player.texCoords));


    // initialize enemy attributes
    GLuint enemyLeft = LoadTexture("enemy_left.png");
    GLuint enemyRight = LoadTexture("enemy_right.png");
    for (int i = 0; i < ENEMY_COUNT; i++) {
        states[0].enemies[i].entityType = ENEMY;
        states[0].enemies[i].isStatic = false;
        states[0].enemies[i].width = 1.0f;
        states[0].enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        states[0].enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        states[0].enemies[i].textures[0] = enemyLeft;
        states[0].enemies[i].textures[1] = enemyRight;
        float enemy_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float enemy_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        std::memcpy(states[0].enemies[i].vertices, enemy_vertices, sizeof(states[0].enemies[i].vertices));
        std::memcpy(states[0].enemies[i].texCoords, enemy_texCoords, sizeof(states[0].enemies[i].texCoords));
    }
    // enemy positions
    states[0].enemies[0].position = glm::vec3(4, 4, 0);
    states[0].enemies[1].position = glm::vec3(0, 3, 0);
    states[0].enemies[2].position = glm::vec3(0, 3, 0);
    // enemy states[0]
    states[0].enemies[0].entityState = STILL;
    states[0].enemies[1].entityState = WALKING;
    states[0].enemies[2].entityState = AI;
    // enemy default direction
    states[0].enemies[0].entityDir = LEFT;
    states[0].enemies[1].entityDir = LEFT;
    states[0].enemies[2].entityDir = AUTO;
    // enemy default texture
    states[0].enemies[0].textureID = states[0].enemies[0].textures[0];
    states[0].enemies[1].textureID = states[0].enemies[1].textures[0];
    states[0].enemies[2].textureID = states[0].enemies[2].textures[1];


    //load platform textures
    GLuint groundTextureID = LoadTexture("ground_stone.png");
    GLuint grassTextureID = LoadTexture("air_stone.png");


    // loop through and initialize ground platform
    float platform_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float platform_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    for (int i = 0; i < 10; i++) {
        states[0].platforms[i].entityType = PLATFORM;
        states[0].platforms[i].textureID = groundTextureID;
        states[0].platforms[i].position = glm::vec3(i - 4.5f, -3.5f, 0);
        std::memcpy(states[0].platforms[i].vertices, platform_vertices, sizeof(states[0].platforms[i].vertices));
        std::memcpy(states[0].platforms[i].texCoords, platform_texCoords, sizeof(states[0].platforms[i].texCoords));
    }


    // initialize platforms in the air
    states[0].platforms[10].entityType = PLATFORM;
    states[0].platforms[10].textureID = grassTextureID;
    states[0].platforms[10].position = glm::vec3(-4.5f, 1.50f, 0);
    std::memcpy(states[0].platforms[10].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[10].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));

    states[0].platforms[11].entityType = PLATFORM;
    states[0].platforms[11].textureID = grassTextureID;
    states[0].platforms[11].position = glm::vec3(-3.5f, 1.50f, 0);
    std::memcpy(states[0].platforms[11].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[11].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));

    states[0].platforms[12].entityType = PLATFORM;
    states[0].platforms[12].textureID = grassTextureID;
    states[0].platforms[12].position = glm::vec3(-2.5f, 1.50f, 0);
    std::memcpy(states[0].platforms[12].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[12].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));


    states[0].platforms[13].entityType = PLATFORM;
    states[0].platforms[13].textureID = grassTextureID;
    states[0].platforms[13].position = glm::vec3(2.5f, -0.50f, 0);
    std::memcpy(states[0].platforms[13].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[13].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));

    states[0].platforms[14].entityType = PLATFORM;
    states[0].platforms[14].textureID = grassTextureID;
    states[0].platforms[14].position = glm::vec3(3.5f, -0.50f, 0);
    std::memcpy(states[0].platforms[14].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[14].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));

    states[0].platforms[15].entityType = PLATFORM;
    states[0].platforms[15].textureID = grassTextureID;
    states[0].platforms[15].position = glm::vec3(4.5f, -0.50f, 0);
    std::memcpy(states[0].platforms[15].vertices, platform_vertices, sizeof(states[0].platforms[0].vertices));
    std::memcpy(states[0].platforms[15].texCoords, platform_texCoords, sizeof(states[0].platforms[0].texCoords));


    // initialize win banner attributes
    states[0].banners[0].isStatic = true;
    // initialize win banner textures
    states[0].banners[0].textureID = LoadTexture("win.png");
    float banner1_vertices[] = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner1_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(states[0].banners[0].vertices, banner1_vertices, sizeof(states[0].banners[0].vertices));
    std::memcpy(states[0].banners[0].texCoords, banner1_texCoords, sizeof(states[0].banners[0].texCoords));


    // initialize lose banner attributes
    states[0].banners[1].isStatic = true;
    // initialize lose banner textures
    states[0].banners[1].textureID = LoadTexture("lost.png");
    float banner2_vertices[] = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner2_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(states[0].banners[1].vertices, banner2_vertices, sizeof(states[0].banners[1].vertices));
    std::memcpy(states[0].banners[1].texCoords, banner2_texCoords, sizeof(states[0].banners[1].texCoords));


    // set program view, model, and projection matricies
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);


    // set matricies
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);


    // use our shader program
    glUseProgram(program.programID);


    // enables and sets blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // sets background color
    glClearColor(0.2f, 0.1f, 0.2f, 1.0f);
}


// define input function
void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_SPACE:
                states[0].player.Jump(5.0f);
                break;

            }
            break;
        }
    }

    // reset player velocity to prevent continuous movement
    states[0].player.velocity.x = 0;

    // Check for pressed/held keys below
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_A]) {
        states[0].player.velocity.x = -3.0f;
        states[0].player.entityDir = LEFT;
    }
    else if (keys[SDL_SCANCODE_D]) {
        states[0].player.velocity.x = 3.0f;
        states[0].player.entityDir = RIGHT;
    }
}


// define update function
#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

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

        // check if player lost
        if (states[0].player.entityState == DEAD) {
            gameLost = true;
        }
        else if (!start and liveCount == 0) {
            gameWon = true;
        }

        // check and update player against enemies
        states[0].player.Update(FIXED_TIMESTEP, states[0].enemies, ENEMY_COUNT);
        if (states[0].player.lifeLock) {


            states[0].player.lives--;
            states[0].player.position = startPosition;
            states[0].player.lifeLock = false;
        }


        // check and update player against platforms
        states[0].player.Update(FIXED_TIMESTEP, states[0].platforms, PLATFORM_COUNT);

        // check and update enemies against platforms and player
        for (int i = 0; i < ENEMY_COUNT; i++) {
             
            // dont update if dead
            if (states[0].enemies[i].entityState != DEAD) {
                states[0].enemies[i].Update(FIXED_TIMESTEP, states[0].platforms, PLATFORM_COUNT);
            }


            // start autonomous routine for enemies
            states[0].enemies[i].startWalk();
            states[0].enemies[i].startJump();
            states[0].enemies[i].startAI(states[0].player);

            // update enemey walking direction
            if (states[0].enemies[i].sensorLeftCol and !states[0].enemies[i].sensorRightCol) {
                states[0].enemies[i].entityDir = LEFT;
                states[0].enemies[i].textureID = states[0].enemies[i].textures[0];
            }
             


            else if (!states[0].enemies[i].sensorLeftCol and states[0].enemies[i].sensorRightCol) {
                states[0].enemies[i].entityDir = RIGHT;
                states[0].enemies[i].textureID = states[0].enemies[i].textures[1];
            }

            else if (states[0].enemies[i].velocity.x > 0 and states[0].enemies[i].entityState==AI) {
                states[0].enemies[i].textureID = states[0].enemies[i].textures[1];
            }
            else if (states[0].enemies[i].velocity.x < 0 and states[0].enemies[i].entityState == AI) {
                states[0].enemies[i].textureID = states[0].enemies[i].textures[0];
            }

        }

        // update player walking direction
        if (states[0].player.entityDir == LEFT) {
            states[0].player.textureID = states[0].player.textures[0];
        }
        else if (states[0].player.entityDir == RIGHT) {
            states[0].player.textureID = states[0].player.textures[1];
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

     
}


// define render
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameWon) {
        //render won banner
        states[0].banners[0].Render(&program);
    }
    else if (gameLost) {
        //render lost banner
        states[0].banners[1].Render(&program);
    }
    else {

        // render player
        states[0].player.Render(&program);

        //GENERALIZE ALL OF THIS INTO A FUNCTION
        /////////////////////////////////////////
        // render non-dead enemies
        liveCount = 0;
        for (int i = 0; i < ENEMY_COUNT; i++) {
            if (states[0].enemies[i].entityState != DEAD) {
                start = false;
                liveCount++;
                states[0].enemies[i].Render(&program);
            }
        }

        // render platforms
        for (int i = 0; i < PLATFORM_COUNT; i++) {
            states[0].platforms[i].Render(&program);
        }
        /////////////////////////////////////////
    }
    // swap to new frame
    SDL_GL_SwapWindow(displayWindow);

    //DEBUG
    std::cout << states[0].player.lives << "\n";
}


// define quit function
void Shutdown() {
    SDL_Quit();
}

// main
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
