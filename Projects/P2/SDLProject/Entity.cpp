#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    float width = 10.0f;
    float height = 0.3f;
}

void Entity::Render(ShaderProgram *program) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    
    float vertices[] = { -5.0, -0.15, -5.0, 0.15, 5.0, 0.15, 5.0, 0.15, 5.0, -0.15, -5.0, -0.15 };
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);

}
