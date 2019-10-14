#pragma once
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

#define COORDS_SIZE 12 // size of array that stores texture coordinates

// define multiple types of entities in game
enum EntityType { PLAYER, WALL, BLOCK, PLATFORM };

class Entity {
public:
    
    // constructor
    Entity();
    
    // Entity identifiers
    EntityType entityType; // store what kinds of entity this is
    EntityType lastCollision; // stores the last Entity this Entity collided with
    bool IsStatic; // stores whether this entity is static or not
    
    // Entity mechanics
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    
    // Entity properties
    float width;
    float height;
    float speed;
    float scale;
    
    // Entity texture information
    GLuint textureID;
    float vertices[COORDS_SIZE];
    float texCoords[COORDS_SIZE];
    
    // collision functions for x and y
    bool CheckCollision(Entity other);
    void CheckCollisionsY(Entity *objects, int objectCount);
    void CheckCollisionsX(Entity *objects, int objectCount);
    
    // Update and render functions
    void Update(float deltaTime, Entity *objects, int objectCount);
    void Render(ShaderProgram *program);
};


