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
bool startMenu = true;
bool gameWon = false;
bool gameLost = false;
glm::vec3 startPosition = glm::vec3(-4, 3, 0);
int currentLevel = 0; // level count starts from zero for reasons ;^)


// define level count
#define LEVELS 3

// define MAX platforms per level
#define MAX_PLAT 80

// define MAX enemies per level
#define MAX_ENEMY 3

// define MAX banner per level
#define MAX_BANNER 3


// define live enemy count for game and helper flag
bool start = true;
int liveCount = 0;


// define enemy count for each level in the game
int enemyCount[LEVELS] = { 1, 1, 1 };


//define GameState object - will keep track of objects in the game
struct GameState {
    // player
    Entity player;

    // game platforms - 80 = (640/64) * ceil(480/64) - maximum tiles of 64 pixels in screen
    Entity platforms[MAX_PLAT];

    // game platform locations
    glm::vec3 platform_loc[MAX_PLAT];

    // enemies
    Entity enemies[MAX_ENEMY];

    // enemy locations
    glm::vec3 enemy_loc[MAX_ENEMY];

    // enemy states
    EntityState enemy_state[MAX_ENEMY];

    // enemy directions
    EntityDir enemy_dir[MAX_ENEMY];

    // banners - menu, win, lost, etc.
    Entity banners[MAX_BANNER];
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



// define player initialization function
void initPlayer(Entity* player) {

    // initialize player attributes
    player->entityType = PLAYER;
    player->isStatic = false;
    player->width = 1.0f;
    player->position = player->startPosition;
    player->sensorLeft = glm::vec3(player->position.x + 0.6f, player->position.y - 0.6f, 0);
    player->sensorRight = glm::vec3(player->position.x - 0.6f, player->position.y - 0.6f, 0);
    player->acceleration = glm::vec3(0, -9.81f, 0);
    player->textures[0] = LoadTexture("player_left.png");
    player->textures[1] = LoadTexture("player_right.png");
    player->textureID = player->textures[1];
    float player_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float player_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(player->vertices, player_vertices, sizeof(player->vertices));
    std::memcpy(player->texCoords, player_texCoords, sizeof(player->texCoords));
}



// define enemy initialization function
void initEnemy(Entity* enemies, GLuint* textures, int enemy_count) {

    for (int i = 0; i < enemy_count; i++) {
        // initialize enemy attributes
        enemies[i].entityType = ENEMY;
        enemies[i].isStatic = false;
        enemies[i].width = 1.0f;
        enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        enemies[i].acceleration = glm::vec3(0, -9.81f, 0);
        enemies[i].textures[0] = textures[0];
        enemies[i].textures[1] = textures[1];
        float enemy_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float enemy_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        std::memcpy(enemies[i].vertices, enemy_vertices, sizeof(enemies[i].vertices));
        std::memcpy(enemies[i].texCoords, enemy_texCoords, sizeof(enemies[i].texCoords));
    }
}



// define ground platform initialization function
void initgPlatform(Entity* platforms, GLuint texture) {

    // loop through and initialize ground platform
    float platform_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float platform_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    for (int i = 0; i < 10; i++) {
        platforms[i].entityType = PLATFORM;
        platforms[i].textureID = texture;
        platforms[i].position = glm::vec3(i - 4.5f, -3.5f, 0);
        std::memcpy(platforms[i].vertices, platform_vertices, sizeof(platforms[i].vertices));
        std::memcpy(platforms[i].texCoords, platform_texCoords, sizeof(platforms[i].texCoords));
    }
}



// define banner initialization function
void initBanner(Entity* banners, GLuint* textures) {

    // initialize Start banner attributes
    banners[0].isStatic = true;
    banners[0].textureID = textures[0];
    float banner0_vertices[] = { -5.0, -2.0, 5.0, -2.0, 5.0, 2.0, -5.0, -2.0, 5.0, 2.0, -5.0, 2.0 };
    float banner0_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(banners[0].vertices, banner0_vertices, sizeof(banners[0].vertices));
    std::memcpy(banners[0].texCoords, banner0_texCoords, sizeof(banners[0].texCoords));

    // initialize win banner attributes
    banners[1].isStatic = true;
    banners[1].textureID = textures[1];
    float banner1_vertices[] = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner1_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(banners[1].vertices, banner1_vertices, sizeof(banners[1].vertices));
    std::memcpy(banners[1].texCoords, banner1_texCoords, sizeof(banners[1].texCoords));

    // initialize lose banner attributes
    banners[2].isStatic = true;
    banners[2].textureID = textures[2];
    float banner2_vertices[] = { -2.5, -0.25, 2.5, -0.25, 2.5, 0.25, -2.5, -0.25, 2.5, 0.25, -2.5, 0.25 };
    float banner2_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    std::memcpy(banners[2].vertices, banner2_vertices, sizeof(banners[2].vertices));
    std::memcpy(banners[2].texCoords, banner2_texCoords, sizeof(banners[2].texCoords));
}



// define initialize function
void Initialize() {

    // init Audio/Video
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music* music;
    music = Mix_LoadMUS("bgMusic.wav");
    Mix_PlayMusic(music, -1);

    // init display window
    displayWindow = SDL_CreateWindow("AI!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // init viewport
    glViewport(0, 0, 640, 480);

    // load texture shader
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");


    // initialize player in all levels
    for (int i = 0; i < LEVELS; i++) {
        Entity* player = &(states[i].player);
        initPlayer(player);
    }


    // initialize enemy entities in all levels
    GLuint enemyLeft = LoadTexture("enemy_left.png");
    GLuint enemyRight = LoadTexture("enemy_right.png");
    for (int i = 0; i < LEVELS; i++) {
        GLuint enemy_textures[2] = { enemyLeft, enemyRight };
        initEnemy(states[i].enemies, enemy_textures, enemyCount[i]);
    }

    // initlize some other enemy ettributes
    // level 1
    states[0].enemies[0].position = glm::vec3(4, 4, 0);
    states[0].enemies[0].entityState = STILL;
    states[0].enemies[0].entityDir = LEFT;
    states[0].enemies[0].textureID = states[0].enemies[0].textures[0];

    // level 2
    states[1].enemies[0].position = glm::vec3(4, 4, 0);
    states[1].enemies[0].entityState = STILL;
    states[1].enemies[0].entityDir = LEFT;
    states[1].enemies[0].textureID = states[0].enemies[0].textures[0];

    // level 3
    states[2].enemies[0].position = glm::vec3(4, 4, 0);
    states[2].enemies[0].entityState = STILL;
    states[2].enemies[0].entityDir = LEFT;
    states[2].enemies[0].textureID = states[0].enemies[0].textures[0];
    


    //load platform textures
    GLuint groundTextureID = LoadTexture("ground_stone.png");
    GLuint airTextureID = LoadTexture("air_stone.png");
    // initialize ground platform in all levels
    for (int i = 0; i < LEVELS; i++) {
        GLuint plat_textures[2] = { groundTextureID, airTextureID };
        initgPlatform(states[i].platforms, plat_textures[0]);
    }


    //load banner textures
    GLuint startBannerID = LoadTexture("start.png");
    GLuint winBannerID = LoadTexture("win.png");
    GLuint loseBannerID = LoadTexture("lost.png");
    // initialize banners in all levels
    for (int i = 0; i < LEVELS; i++) {
        GLuint banner_textures[3] = { startBannerID,winBannerID, loseBannerID };
        initBanner(states[i].banners, banner_textures);
    }


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

void playJump() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Chunk* SoundCrush = Mix_LoadWAV("jumping.wav");
    Mix_PlayChannel(-1, SoundCrush, 0);
    Mix_VolumeChunk(SoundCrush, MIX_MAX_VOLUME / 8);
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
                states[currentLevel].player.Jump(5.0f);
                playJump();
                break;

            case SDLK_RSHIFT:
                if (startMenu) {
                    startMenu = false;
                    gameIsRunning = true;
                }

            }
            break;
        }
    }

    // reset player velocity to prevent continuous movement
    states[currentLevel].player.velocity.x = 0;

    // Check for pressed/held keys below
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_A]) {
        states[currentLevel].player.velocity.x = -3.0f;
        states[currentLevel].player.entityDir = LEFT;
    }
    else if (keys[SDL_SCANCODE_D]) {
        states[currentLevel].player.velocity.x = 3.0f;
        states[currentLevel].player.entityDir = RIGHT;
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
        if (states[currentLevel].player.entityState == DEAD) {
            gameLost = true;
        }
        // liveCount is non-dead enemy count in game, check if all enemies are dead
        else if (!start and liveCount == 0) {
            std:: cout << "Evaluating" << "\n";
            // check if we're at the last level
            if (currentLevel < LEVELS) {
                states[currentLevel].player.lives = states[currentLevel - 1].player.lives;
                currentLevel++;
                std:: cout << "LEVEL " << currentLevel + 1 << "\n";
            }
            else {
                gameWon = true;
                std:: cout << "GAME WON" << "\n";
                return;
            }
            
        }

        // check and update player against enemies
        states[currentLevel].player.Update(FIXED_TIMESTEP, states[currentLevel].enemies, enemyCount[currentLevel]);


        // check and update player against platforms
        states[currentLevel].player.Update(FIXED_TIMESTEP, states[currentLevel].platforms, MAX_PLAT);

        // check and update all enemies in current level against platforms and player
        for (int i = 0; i < enemyCount[currentLevel]; i++) {

            // dont update if dead
            if (states[currentLevel].enemies[i].entityState != DEAD) {
                states[currentLevel].enemies[i].Update(FIXED_TIMESTEP, states[currentLevel].platforms, MAX_PLAT);
            }

            // start autonomous routine for enemies
            states[currentLevel].enemies[i].startWalk();
            states[currentLevel].enemies[i].startJump();
            states[currentLevel].enemies[i].startAI(states[currentLevel].player);

            // update enemey walking direction and texture that corresponds with it
            if (states[currentLevel].enemies[i].sensorLeftCol and !states[currentLevel].enemies[i].sensorRightCol) {
                states[currentLevel].enemies[i].entityDir = LEFT;
                states[currentLevel].enemies[i].textureID = states[currentLevel].enemies[i].textures[0];
            }
            else if (!states[currentLevel].enemies[i].sensorLeftCol and states[currentLevel].enemies[i].sensorRightCol) {
                states[currentLevel].enemies[i].entityDir = RIGHT;
                states[currentLevel].enemies[i].textureID = states[currentLevel].enemies[i].textures[1];
            }
            else if (states[currentLevel].enemies[i].velocity.x > 0 and states[currentLevel].enemies[i].entityState == AI) {
                states[currentLevel].enemies[i].textureID = states[currentLevel].enemies[i].textures[1];
            }
            else if (states[currentLevel].enemies[i].velocity.x < 0 and states[currentLevel].enemies[i].entityState == AI) {
                states[currentLevel].enemies[i].textureID = states[currentLevel].enemies[i].textures[0];
            }
        }

        // update player walking direction texture that corresponds with it
        if (states[currentLevel].player.entityDir == LEFT) {
            states[currentLevel].player.textureID = states[currentLevel].player.textures[0];
        }
        else if (states[currentLevel].player.entityDir == RIGHT) {
            states[currentLevel].player.textureID = states[currentLevel].player.textures[1];
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

    // control count of player lives left
    if (states[currentLevel].player.lifeLock) {
        states[currentLevel].player.lives--;
        states[currentLevel].player.position = startPosition;
        states[currentLevel].player.lifeLock = false;
    }
}


// define render
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (startMenu) {
        //render Start banner
        states[currentLevel].banners[0].Render(&program);
    }
    if (gameWon) {
        //render won banner
        states[currentLevel].banners[1].Render(&program);
    }
    else if (gameLost) {
        //render lost banner
        states[currentLevel].banners[2].Render(&program);
    }
    else {
        // render player
        states[currentLevel].player.Render(&program);

        // render non-dead enemies in current level
        liveCount = 0;
        for (int i = 0; i < enemyCount[currentLevel]; i++) {
            if (states[currentLevel].enemies[i].entityState != DEAD) {
                start = false;
                liveCount++;
                states[currentLevel].enemies[i].Render(&program);
            }
        }

        // render platforms in current level
        for (int i = 0; i < MAX_PLAT; i++) {
            states[currentLevel].platforms[i].Render(&program);
        }

    }

    // swap to new frame
    SDL_GL_SwapWindow(displayWindow);
}


// define quit function
void Shutdown() {
    SDL_Quit();
}

// main
int main(int argc, char* argv[]) {
    
    // initialize game
    Initialize();

    // master game loop
    while (gameIsRunning) {
        ProcessInput();
        if (startMenu) {
            states[currentLevel].banners[0].Render(&program);
        }
        else {
            Update();
            
        }
        Render();
    }

    Shutdown();
    return 0;
}
