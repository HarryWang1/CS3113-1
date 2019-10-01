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

#include "Entity.h"


class Player : public Entity {
public:
    
    glm::vec3 movement;
    
    glm::vec3 prevPosition;
    
    float speed;
    
    bool lock;
    
    GLuint textureID;
    
    Player();
    
    void Update(float deltaTime);
    //void rotatePlayer(float angle, ShaderProgram *program);
    void Render(ShaderProgram *program);
};
