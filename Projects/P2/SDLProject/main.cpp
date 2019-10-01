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

#include "Player.h"
#include "Entity.h"

#include <iostream>
#include <string>
#include <vector>

//define window
SDL_Window* displayWindow;

//define game state
bool gameIsRunning = true;

//define shader programs
ShaderProgram program;
ShaderProgram program2;

//define game matrixies
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

//define players
Player player;
Player player2;

//define game elements
Entity wall;
Entity wall2;
Player pong; //pong is defined as player since it's a moving textured entity

//define players default positions
glm::vec3 player_position = glm::vec3(-4.75,0,0);
glm::vec3 player2_position = glm::vec3(4.75,0,0);
glm::vec3 pong_position = glm::vec3(0,0,0);

//define Entities default positions
glm::vec3 wall_position = glm::vec3(0,3.5,0);
glm::vec3 wall2_position = glm::vec3(0,-3.5,0);

//define movement vectors
glm::vec3 player_movement = glm::vec3(0, 0, 0);
glm::vec3 player2_movement = glm::vec3(0, 0, 0);
glm::vec3 pong_movement = glm::vec3(0, 0, 0);


//define load texture
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

//define collision detection
bool collision(Entity& entity1, Entity& entity2, std::string entName) {
    
    if (&entity1 != &entity2) {
        //distance between entity1 and entity2 center points
        float xDiff = fabs(entity2.position.x - entity1.position.x);
        float yDiff = fabs(entity2.position.y - entity1.position.y);
        
        //distance between entity1 and entity2 bounds
        float xDist = xDiff - (((float)(entity1.width + entity2.width))/2.0f);
        float yDist = yDiff - (((float)(entity1.height + entity2.height))/2.0f);
        
        std::cout << xDist << " " << yDist << "------" << entity1.position.x << " " << entity1.position.y << "\n";
        
        //check for  every possible collision - the constants used here are all measured offsets (this is needed since textures aren't perfect)
        if (entName == "player" and xDist < 0.300918f and yDist < 0.631998) {
            return true;
        }
        else if (entName == "player2" and !(xDist > 0.300918f) and yDist < 0.631998) {
            return true;
        }
        else if (entName == "wall" and yDist < 0.332115f) {
            return true;
        }
        else if (entName == "wall2" and yDist < 0.332115f) {
            return true;
        }
        else if (entName == "player-wall" and yDist < 0.65f) {
            return true;
        }
        else if (entName == "player-wall2" and entity1.position.y < 0 and yDist < 0.635f) {
            return true;
        }
        else if (entName == "player2-wall" and yDist < 0.65f) {
            return true;
        }
        else if (entName == "player2-wall2" and entity1.position.y < 0 and yDist < 0.635f) {
            return true;
        }
    }
    
    else {
        if (entName == "pong-left" and entity1.position.x < -4.8f) {
            return true;
        }
        if (entName == "pong-right" and entity1.position.x > 4.8f) {
            return true;
        }
    }
    
    return false;
}

//define bounce - determines ball movement after collision
void bounce(Player& pong, std::string entName) {
    //if bounced against wall
    if (entName == "wall" or entName == "wall2") {
        pong.movement.y *= -1;
    }
    else if (entName == "player" or entName == "player2") {
        pong.movement.x *= -1;
    }
}


void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    //load textured shaders
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    //load untextured shaders
    program2.Load("shaders/vertex.glsl", "shaders/fragment.glsl");
    
    //initilize player texture, position, speed
    player.textureID = LoadTexture("stick.png");
    player.position = player_position;
    player.speed = 4.5;
    
    //initilize player2 texture, position, speed
    player2.textureID = LoadTexture("stick.png");
    player2.position = player2_position;
    player2.speed = 4.5;
    
    //initialize pong texture, position, speed
    pong.textureID = LoadTexture("ball.png");
    pong.position = pong_position;
    pong.speed = 4;
    
    //initialize wall position
    wall.position = wall_position;
    wall2.position = wall2_position;
    
    //set view, projection, and color for all programs
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    program2.SetProjectionMatrix(projectionMatrix);
    program2.SetViewMatrix(viewMatrix);
    program2.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    //use all programs
    glUseProgram(program.programID);
    glUseProgram(program2.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void ProcessInput() {
    
    //set  player movement vectors
    player_movement = glm::vec3(0, 0, 0);
    player2_movement = glm::vec3(0, 0, 0);
    
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
                
            case SDL_QUIT:
                
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                     
                    //check for certain key presses
                    case SDLK_SPACE:
                        // pressing spacebar quits game
                        gameIsRunning = false;
                        break;
                }
                break;
        }
    }
    
    // Check for pressed/held keys below
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    
    //player 1 input
    if (keys[SDL_SCANCODE_W])
    {
        player_movement.y = 1;
    }
    
    else if  (keys[SDL_SCANCODE_S])
    {
        player_movement.y = -1;
    }
    
    if (glm::length(player_movement) > 1.0f)
    {
        player_movement = glm::normalize(player_movement);
    }
    
    
    //player 2 input
    if (keys[SDL_SCANCODE_UP])
    {
        //player2_movement.y = 1;
        player2_movement.y = 1;
    }
    
    else if  (keys[SDL_SCANCODE_DOWN])
    {
        //player2_movement.y = -1;
        player2_movement.y = -1;
    }

    if (glm::length(player_movement) > 1.0f)
    {
        player2_movement = glm::normalize(player_movement);
    }

    //update player movements
    player.movement = player_movement;
    player2.movement = player2_movement;
}


float lastTicks = 0;
void Update() {
    //sync
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    //update player positions
    player.Update(deltaTime);
    player2.Update(deltaTime);
    
    //calculate pong movement
    if (pong_movement.x == 0 and pong_movement.y == 0 and pong_movement.z == 0)
    {
        //start with base movement vector
        pong_movement = glm::vec3(0.0f, -1.0f, 0.0f);
        
        //store all preset paths into vector
        std::vector<glm::vec3> presetPaths;
        
        //define some nice preset starting pong paths
        glm::vec3 pong_movement1 = glm::vec3(0.5f, 0.65f, 0.0f);
        presetPaths.push_back(pong_movement1);
        
        glm::vec3 pong_movement2 = glm::vec3(0.5f, -0.65f, 0.0f);
        presetPaths.push_back(pong_movement2);
        
        glm::vec3 pong_movement3 = glm::vec3(-0.5f, 0.65f, 0.0f);
        presetPaths.push_back(pong_movement3);
        
        glm::vec3 pong_movement4 = glm::vec3(-0.5f, -0.65f, 0.0f);
        presetPaths.push_back(pong_movement4);
        
        glm::vec3 pong_movement5 = glm::vec3(1.0f, 0.25f, 0.0f);
        presetPaths.push_back(pong_movement5);
        
        glm::vec3 pong_movement6 = glm::vec3(1.0f, -0.25f, 0.0f);
        presetPaths.push_back(pong_movement6);
        
        glm::vec3 pong_movement7 = glm::vec3(-1.0f, 0.25f, 0.0f);
        presetPaths.push_back(pong_movement7);
        
        glm::vec3 pong_movement8 = glm::vec3(-1.0f, -0.25f, 0.0f);
        presetPaths.push_back(pong_movement8);
        
        //chose random path
        int rand_ind = rand() % 8;
        
        //update pong movements
        pong.movement = presetPaths[rand_ind];
        
    }
    
    std::cout << "##################\n";
    
    //check for all possible collisions between Entities
    if (collision(pong, wall, "wall"))
    {
        bounce(pong, "wall");
        std::cout << " wall COLLIDED!\n";
    }
    
    else if (collision(pong, wall2, "wall2"))
    {
        bounce(pong, "wall2");
        std::cout << "wall2 COLLIDED!\n";
    }
    
    if (collision(pong, player, "player"))
    {
        bounce(pong, "player");
        std::cout << "player COLLIDED!\n";
    }
   
    else if (collision(pong, player2, "player2"))
    {
        bounce(pong, "player2");
        std::cout << "player2 COLLIDED!\n";
    }
    
    if (collision(player, wall, "player-wall"))
    {
        player.lock = true;
        std::cout << "player-wall COLLIDED!\n";
    }
    
    else if (collision(player, wall2, "player-wall2"))
    {
        player.lock = true;
        std::cout << "player-wall2 COLLIDED!\n";
    }
    
    if (collision(player2, wall, "player2-wall"))
    {
        player2.lock = true;
        std::cout << "player2-wall COLLIDED!\n";
    }
    
    else if (collision(player2, wall2, "player2-wall2"))
    {
        player2.lock = true;
        std::cout << "player2-wall2 COLLIDED!\n";
    }
    
    //special case collision, out of bounds/a player won
    if (collision(pong, pong, "pong-left"))
    {
        //player lost
        player.textureID = LoadTexture("looser.png");
        std::cout << "pong-left!\n";
    }
    else if (collision(pong, pong, "pong-right"))
    {
        //player2 lost
        player2.textureID = LoadTexture("looser.png");
        std::cout << "pong-right!\n";
    }
    
    //update pong position
    pong.Update(deltaTime);
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    //render players
    player.Render(&program);
    player2.Render(&program);
    
    //render other entities
    pong.Render(&program);
    wall.Render(&program2);
    wall2.Render(&program2);
    
    SDL_GL_SwapWindow(displayWindow);
}


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
