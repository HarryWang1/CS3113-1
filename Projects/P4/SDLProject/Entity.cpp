#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    speed = 0;
    width = 1;
    height = 1;
    scale = 1;
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object.position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object.position.x);
            float penetrationX = fabs(xdist - (width / 2) - (object.width / 2));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
            }
        }
    }
}

bool Entity::CheckCollision(Entity other)
{
    float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
    float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);
    if (xdist < 0 && ydist < 0)
    {
        lastCollision = other.entityType;
        return true;
    }
    
    return false;
}


// check left sensor
void Entity::CheckSensorLeft(Entity *platforms, int platCount) {
    
    for (int i = 0; i < platCount; i++) {
        
        // only check sensor against platform entities
        if (platforms[i].entityType != PLATFORM) {
            return;
        }
        
        Entity platform  = platforms[i];
        
        // check player position against other's position
        float xdist = fabs(sensorLeft.x - platform.position.x) - ((platform.width) / 2.0f);
        float ydist = fabs(sensorLeft.y - platform.position.y) - ((platform.height) / 2.0f);
        
        if (xdist < 0 && ydist < 0) {
            sensorLeftCol = true;
        }
    }
    
    sensorLeftCol = false;;
}


//check right sensor
void Entity::CheckSensorRight(Entity *platforms, int platCount) {
    
    for (int i = 0; i < platCount; i++) {
        
        // only check sensor against platform entities
        if (platforms[i].entityType != PLATFORM) {
            return;
        }
        
        Entity platform  = platforms[i];
        
        // check player position against other's position
        float xdist = fabs(sensorRight.x - platform.position.x) - ((platform.width) / 2.0f);
        float ydist = fabs(sensorRight.y - platform.position.y) - ((platform.height) / 2.0f);
        
        if (xdist < 0 && ydist < 0) {
            sensorRightCol = true;
        }
    }
    
    sensorRightCol = false;;
}


void Entity::Update(float deltaTime, Entity *objects, int objectCount)
{
    if (entityType == WALL) {
        return;
    }
    else if (entityType == PLATFORM) {
        return;
    }
    else if (entityType == BLOCK) {
        return;
    }
    else if (entityType == PLAYER) {
        
        // update velocity
        velocity += acceleration * deltaTime;
        
        // update x and y positions, check for and fix collisions
        position.y += velocity.y * deltaTime; // Move on Y
        CheckCollisionsY(objects, objectCount); // Fix if needed
        position.x += velocity.x * deltaTime; // Move on X
        CheckCollisionsX(objects, objectCount); // Fix if needed
    }
}

void Entity::Render(ShaderProgram *program) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    // translate entity according to current position
    modelMatrix = glm::translate(modelMatrix, position);
    
    // scale entity according to current scale value
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, 0.0f));
    
    // set the matrix
    program->SetModelMatrix(modelMatrix);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}
