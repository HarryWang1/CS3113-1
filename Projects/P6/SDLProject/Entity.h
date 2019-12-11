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

// eneity types
enum  EntityType { PLAYER, ENEMY, PLATFORM };

// entity state
enum EntityState {SIDE, BOTTOM, STILL, WALKING, DEAD, AI };

// entity direction - which way the entity is facing defined by dirrection last moved
enum EntityDir { LEFT, RIGHT, AUTO };

// class
class Entity {
public:

    // entity state attributes
    EntityType entityType;
    EntityState entityState;
    EntityDir entityDir;
    EntityType lastCollision;
    bool isStatic;
    bool isActive;
    int lives;
    bool lifeLock = false;

    // entity physics attributes
    glm::vec3 startPosition;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float speed;
    int rate;

    // entity textures - holds multiple states of same texture
    GLuint textures[2];

    // eneity size attributes
    float width;
    float height;

    // texture
    GLuint textureID;
    float vertices[COORDS_SIZE];
    float texCoords[COORDS_SIZE];

    // constructor
    Entity();

    // player colliison functions
    bool CheckCollision(Entity* other);
    void CheckCollisionsX(Entity* objects, int objectCount);
    void CheckCollisionsY(Entity* objects, int objectCount);
    
    // update function
    void Update(float deltaTime, Entity* objects, int objectCount);
    void Update(float deltaTime, Entity objects, int objectCount);

    // render function
    void Render(ShaderProgram* program);

    //EnemyAttribute 
    void EnemyAttributes();

    // player collision flags
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;

};



