#include "Player.h"

Player::Player()
{
    position = glm::vec3(0);
    prevPosition = glm::vec3(0);
    speed = 0;
    float width = 1.0f;
    float height = 1.0f;
}

void Player::Update(float deltaTime)
{
    if (!(lock)) {
        prevPosition = glm::vec3(position.x, position.y, position.z);
        position += movement * speed * deltaTime;
    } else {
        lock = false;
        position.x = prevPosition.x;
        position.y = prevPosition.y;
        position.z = prevPosition.z;
    }
}


void Player::Render(ShaderProgram *program) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
