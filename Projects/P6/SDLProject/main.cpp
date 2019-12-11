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
int currentLevel = 0; // level count starts from zero for reasons ;^)


// define level count
#define LEVELS 3

// define MAX barrier count per level
#define MAX_PLAT 8000

// define MAX enemies per level
#define MAX_ENEMY 4

// define MAX banner per level
#define MAX_BANNER 3


// define live enemy count for game and start helper flag
int liveCount = 0;
bool start = true;

// define used platform count for rendering
int platCount = 0;

// define enemy count for each level in the game
int enemyCount[LEVELS] = { 4, 4, 1 };

// original view matrix as list
float scaleFactor = 1.50f;
float vmConst[4] = { -5.0f, 5.0f, -3.75f, 3.75f };


// player
Entity player;

// game barriers - 80 = (640/64) * ceil(480/64) - maximum tiles of (64x64) pixels in screen
Entity platforms[MAX_PLAT];


// banners - menu, win, lost, etc.
Entity banners[MAX_BANNER];


//define GameState object - will keep track of objects in the game
struct GameState {

    // enemies
    Entity enemies[MAX_ENEMY];

    // enemy locations
    glm::vec3 enemy_loc[MAX_ENEMY];

    // enemy states
    EntityState enemy_state[MAX_ENEMY];

    // enemy directions
    EntityDir enemy_dir[MAX_ENEMY];
    
    // level goal
    glm::vec3 goal = glm::vec3(3.5, -3.5, 0);

};


//declare GameState object
GameState states[LEVELS];


//define load texture function
GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        std::cout<<filePath<<"\n";
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
void initPlayer(Entity* player, GLuint* textures) {

    // initialize player attributes
    player->entityType = PLAYER;
    player->isStatic = false;
    player->width = 1.0f;
    player->position = player->startPosition;
    player->entityDir = RIGHT;
    //player->sensorLeft = glm::vec3(player->position.x + 0.6f, player->position.y - 0.6f, 0);
    //player->sensorRight = glm::vec3(player->position.x - 0.6f, player->position.y - 0.6f, 0);
    player->acceleration = glm::vec3(0, 0, 0);
    player->textures[0] = textures[0];
    player->textures[1] = textures[1];
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
        enemies[i].acceleration = glm::vec3(0, 0, 0);
        enemies[i].textures[0] = textures[0];
        enemies[i].textures[1] = textures[1];
        float enemy_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float enemy_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
        std::memcpy(enemies[i].vertices, enemy_vertices, sizeof(enemies[i].vertices));
        std::memcpy(enemies[i].texCoords, enemy_texCoords, sizeof(enemies[i].texCoords));
    }
}


// define ground & side barrier initialization function
void initgPlatform(Entity* platforms, GLuint texture, int currentLevel) {
    // reset platcount
    platCount = 0;

    // loop through and initialize ground & side barriers
    float platform_vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float platform_texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    // compute iteration range and position scale from level
    int range = 8;
    float posScale = 1.0f;
    if (currentLevel > 0) {
        posScale = pow(scaleFactor, currentLevel);
        range = range * posScale;
    }

    // build bottom barrier
    for (int i = 0; i < range; i++) {
        platforms[i].entityType = PLATFORM;
        platforms[i].textureID = texture;
        platforms[i].position = glm::vec3(i - (4.5f * posScale), -3.5f * posScale, 0);
        std::memcpy(platforms[i].vertices, platform_vertices, sizeof(platforms[i].vertices));
        std::memcpy(platforms[i].texCoords, platform_texCoords, sizeof(platforms[i].texCoords));
        platCount++;
    }
    // build top barrier
    for (int i = range; i < (range * 2); i++) {
        platforms[i].entityType = PLATFORM;
        platforms[i].textureID = texture;
        platforms[i].position = glm::vec3((4.5f * posScale) - (i - range), 3.5f * posScale, 0);
        std::memcpy(platforms[i].vertices, platform_vertices, sizeof(platforms[i].vertices));
        std::memcpy(platforms[i].texCoords, platform_texCoords, sizeof(platforms[i].texCoords));
        platCount++;
    }

    // build right barrier
    for (int i = (range * 2); i < (range * 3); i++) {
        platforms[i].entityType = PLATFORM;
        platforms[i].textureID = texture;
        platforms[i].position = glm::vec3(4.5f * posScale, (3.5f * posScale) - (i - (range * 2)), 0);
        std::memcpy(platforms[i].vertices, platform_vertices, sizeof(platforms[i].vertices));
        std::memcpy(platforms[i].texCoords, platform_texCoords, sizeof(platforms[i].texCoords));
        platCount++;
    }

    // build left barrier
    for (int i = (range * 3); i < (range * 4); i++) {
        platforms[i].entityType = PLATFORM;
        platforms[i].textureID = texture;
        platforms[i].position = glm::vec3(-4.5f * posScale, (-3.5 * posScale) + (i - (range * 3)), 0);
        std::memcpy(platforms[i].vertices, platform_vertices, sizeof(platforms[i].vertices));
        std::memcpy(platforms[i].texCoords, platform_texCoords, sizeof(platforms[i].texCoords));
        platCount++;
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
GLuint Initialize() {

    // init Audio/Video
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Music* music;
    music = Mix_LoadMUS("mystery.wav");
    Mix_PlayMusic(music, -1);

    // init display window
    displayWindow = SDL_CreateWindow("CASTLE RAIDER v1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // init viewport
    glViewport(0, 0, 640, 480);

    // load texture shader
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    // initialize player
    GLuint playerLeftID = LoadTexture("olive.png");
    GLuint playerRightID = LoadTexture("olive2.png");
    GLuint player_textures[2] = { playerLeftID, playerRightID };
    initPlayer(&player, player_textures);

    // initialize enemy entities in all levels
    GLuint enemyLeft = LoadTexture("bullet.png");
    GLuint enemyRight = LoadTexture("SideBullet.png");
    GLuint enemy_textures[2] = { enemyLeft, enemyRight };
    for (int i = 0; i < LEVELS; i++) {
        initEnemy(states[i].enemies, enemy_textures, enemyCount[i]);
    }
    
    // initialize goal in all levels
    for (int i = 1; i < LEVELS; i++) {
        states[i].goal *= pow(scaleFactor, i);
    }

    // initlize some other enemy ettributesd
    // LEVEL 1:
    //TWO bullets from Bottom
    states[0].enemies[0].rate = 5;
    states[0].enemies[0].position = glm::vec3(2, 0, 0);
    states[0].enemies[0].entityState = BOTTOM;
    states[0].enemies[0].entityDir = LEFT;
    states[0].enemies[0].textureID = states[0].enemies[0].textures[0];

    states[0].enemies[1].rate = 1;
    states[0].enemies[1].position = glm::vec3(0, 0, 0);
    states[0].enemies[1].entityState = BOTTOM;
    states[0].enemies[1].entityDir = LEFT;
    states[0].enemies[1].textureID = states[0].enemies[1].textures[0];

    //TWO bullets from SIDE
    states[0].enemies[2].rate = -1;
    states[0].enemies[2].position = glm::vec3(2, 1, 0);
    states[0].enemies[2].entityState = SIDE;
    states[0].enemies[2].entityDir = LEFT;
    states[0].enemies[2].textureID = states[0].enemies[2].textures[1];

    states[0].enemies[3].rate = -1;
    states[0].enemies[3].position = glm::vec3(1, 1, 0);
    states[0].enemies[3].entityState = SIDE;
    states[0].enemies[3].entityDir = LEFT;
    states[0].enemies[3].textureID = states[0].enemies[2].textures[1];

      
      
    // level 2
    //TWO bullets from Bottom
    states[1].enemies[0].rate = 1;
    states[1].enemies[0].position = glm::vec3(2, 0, 0);
    states[1].enemies[0].entityState = BOTTOM;
    states[1].enemies[0].entityDir = LEFT;
    states[1].enemies[0].textureID = states[0].enemies[0].textures[0];

    states[1].enemies[1].rate = 1;
    states[1].enemies[1].position = glm::vec3(1, 0, 0);
    states[1].enemies[1].entityState = BOTTOM;
    states[1].enemies[1].entityDir = LEFT;
    states[1].enemies[1].textureID = states[0].enemies[1].textures[0];

    //TWO bullets from SIDE
    states[1].enemies[2].rate = -1;
    states[1].enemies[2].position = glm::vec3(2, 1, 0);
    states[1].enemies[2].entityState = SIDE;
    states[1].enemies[2].entityDir = LEFT;
    states[1].enemies[2].textureID = states[0].enemies[2].textures[1];

    states[1].enemies[3].rate = -1;
    states[1].enemies[3].position = glm::vec3(1, 1, 0);
    states[1].enemies[3].entityState = SIDE;
    states[1].enemies[3].entityDir = LEFT;
    states[1].enemies[3].textureID = states[0].enemies[2].textures[1];
    
    
    //    // level 3
    //    // update player's position from default to the middle for this level
    //    states[2].player.position = glm::vec3(0, 4, 0);
    //
    //    //enemy 1
    states[2].enemies[0].position = glm::vec3(-1, 0, 0);
    states[2].enemies[0].entityState = STILL;
    states[2].enemies[0].entityDir = LEFT;
    states[2].enemies[0].textureID = states[0].enemies[0].textures[0];
    //    //enemy 2
    //    states[2].enemies[1].position = glm::vec3(-5, 4, 0);
    //    states[2].enemies[1].entityState = AI;
    //    states[2].enemies[1].entityDir = RIGHT;
    //    states[2].enemies[1].textureID = states[0].enemies[0].textures[0];


    //load platform attributes and textures
    GLuint groundTextureID = LoadTexture("tdGrass.png");
    initgPlatform(platforms, groundTextureID, currentLevel);

    // init banner textures
    GLuint startBannerID = LoadTexture("start.png");
    GLuint winBannerID = LoadTexture("win.png");
    GLuint loseBannerID = LoadTexture("lost.png");
    // initialize banners
    GLuint banner_textures[3] = { startBannerID,winBannerID, loseBannerID };
    initBanner(banners, banner_textures);

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
    glClearColor(0.2f, 0.7f, 0.5f, 1.0f);

    return groundTextureID;
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
            case SDLK_RETURN:
                if (startMenu) {
                    startMenu = false;
                    gameIsRunning = true;
                }
            }
            break;
        }
    }

    // reset player velocity to prevent continuous movement
    player.velocity.x = 0;
    player.velocity.y = 0;

    // Check for pressed/held keys below
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    // if A key is help down
    if (keys[SDL_SCANCODE_A]) {
        player.velocity.x = -3.0f;
        player.entityDir = LEFT;
    }
    // if W key is help down
    else if (keys[SDL_SCANCODE_W]) {
        player.velocity.y = 3.0f;
        player.entityDir = LEFT;
    }
    // if S key is held down
    else if (keys[SDL_SCANCODE_S]) {
        player.velocity.y = -3.0f;
        player.entityDir = LEFT;
    }
    // if D key is held dowm
    else if (keys[SDL_SCANCODE_D]) {
        player.velocity.x = 3.0f;
        player.entityDir = RIGHT;
    }
}


// define update function
#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update(GLuint groundTextureID) {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        
        std::cout << player.position[0] << "," << player.position[1] << "," << player.position[2] << "      " << states[currentLevel].goal[0] << "," << states[currentLevel].goal[1] << "," << states[currentLevel].goal[2] << "\n";

        // check if player lost
        if (player.entityState == DEAD) {
            gameLost = true;
        }
        // check if all enemies are dead, liveCount is non-dead enemy count in game
        else {
            if (player.position[0] >= states[currentLevel].goal[0] and player.position[1] <= states[currentLevel].goal[1]){
                // check if we're not at the last level
                if (currentLevel + 1 < LEVELS) {
                    currentLevel++;
                    
                    // scale up level
                    projectionMatrix = glm::ortho(vmConst[0] *= scaleFactor, vmConst[1] *= scaleFactor, vmConst[2] *= scaleFactor, vmConst[3] *= scaleFactor, -1.0f, 1.0f);
                    // set new projection/view matrix
                    program.SetProjectionMatrix(projectionMatrix);
                    program.SetViewMatrix(viewMatrix);

                    // re-draw the level
                    initgPlatform(platforms, groundTextureID, currentLevel);
                    
                    //scale user start position
                    if (currentLevel > 0){
                        // change player start level as oposite of goal
                        player.startPosition[0] = states[currentLevel].goal[1];
                        player.startPosition[1] = states[currentLevel].goal[0];
                    }
                    // set player start position
                    player.position = player.startPosition;
                }
                else {
                    gameWon = true;
                    return;
                }
            }
        }

        
        // check and update player against enemies
        player.Update(FIXED_TIMESTEP, states[currentLevel].enemies, enemyCount[currentLevel]);

        // check and update player against platforms
        player.Update(FIXED_TIMESTEP, platforms, platCount);

        // check and update all enemies in current level against platforms and player
        for (int i = 0; i < enemyCount[currentLevel]; i++) {
            // dont update if dead
            if (states[currentLevel].enemies[i].entityState != DEAD) {
                states[currentLevel].enemies[i].Update(FIXED_TIMESTEP, platforms, platCount);
            }
        }
        // update player walking direction texture that corresponds with it
        if (player.entityDir == LEFT) {
            player.textureID = player.textures[0];
        }
        else if (player.entityDir == RIGHT) {
            player.textureID = player.textures[1];
        }

        deltaTime -= FIXED_TIMESTEP;
    }
    accumulator = deltaTime;
    // control count of player lives left
    if (player.lifeLock) {
        player.lives--;
        player.position[0] = states[currentLevel].goal[1];
        player.position[1] = states[currentLevel].goal[0];
        player.lifeLock = false;
    }
}


// define render
void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (startMenu) {
        //render Start banner
        banners[0].Render(&program);
    }
    if (gameWon) {
        //render won banner
        banners[1].Render(&program);
    }
    else if (gameLost) {
        //render lost banner
        banners[2].Render(&program);
    }
    else {
        // render player
        player.Render(&program);

        // render non-dead enemies in current level
        liveCount = 0;
        for (int i = 0; i < enemyCount[currentLevel]; i++) {
            if (states[currentLevel].enemies[i].entityState != DEAD) {
                start = false;
                liveCount++;
                states[currentLevel].enemies[i].Render(&program);
            }
        }
        // render USED barriers in stage
        for (int i = 0; i < platCount; i++) {
            platforms[i].Render(&program);
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
    GLuint updateTexture = Initialize();

    // master game loop
    while (gameIsRunning) {
        ProcessInput();

        // dont update game while we're at start screen
        if (startMenu) {
            banners[0].Render(&program);
        }
        else {
            Update(updateTexture);

        }
        Render();
    }

    Shutdown();
    return 0;
}
