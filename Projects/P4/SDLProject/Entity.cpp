#include "Entity.h"

// construct entity
Entity::Entity()
{
    entityType = PLATFORM;
    isStatic = true;
    isActive = true;
    position = glm::vec3(0);
    speed = 0;
    width = 1;
    height = 1;
}

// check collisions
bool Entity::CheckCollision(Entity* other) {
    if (isStatic == true) return false;
    if (isActive == false || other->isActive == false) return false;
    if (other->entityState == DEAD) return false;
    
    // check player position against other's position
    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    if (xdist < 0 && ydist < 0) {
        lastCollision = other->entityType;
        return true;
    }
    
    return false;
}


// check collision and adjust y axis
void Entity::CheckCollisionsY(Entity *objects, int objectCount) {
    for (int i = 0; i < objectCount; i++) {
        Entity* object = &objects[i];
        
        if (CheckCollision(object)) {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object->height / 2));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
                object->collidedBottom = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
                object->collidedTop = true;
            }
        }
    }
}


// check collision and adjust x axis
void Entity::CheckCollisionsX(Entity *objects, int objectCount) {
    for (int i = 0; i < objectCount; i++) {
        Entity* object = &objects[i];
        
        if (CheckCollision(object)) {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2) - (object->width / 2));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
                collidedRight = true;
                object->collidedLeft = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
                collidedLeft = true;
                object->collidedRight = true;
            }
        }
    }
}


// check left sensor
bool senseLX = false;
bool senseLY = false;
void Entity::CheckSensorLeft(Entity *platforms, int platCount) {
    
    for (int i = 0; i < platCount; i++) {
        
        // only check sensor against platform entities
        if (platforms[i].entityType != PLATFORM) {
            return;
        }
        
        Entity platform  = platforms[i];
        
        // check player sensor position against other's position
        
        if (sensorLeft.x > (platform.position.x-((platform.width) / 2.0f))){
            senseLX = true;
        } else { senseLX = false; }
        
        if (sensorLeft.y < (platform.position.y+((platform.height) / 2.0f))){
            senseLY = true;
        } else { senseLY = false; }
        
        if (senseLX and senseLY and collidedBottom) {
            sensorLeftCol = true;
            return;
        }
    }
    
    sensorLeftCol = false;
}


//check right sensor
bool senseRX = false;
bool senseRY = false;
void Entity::CheckSensorRight(Entity *platforms, int platCount) {
    
    for (int i = 0; i < platCount; i++) {
        
        // only check sensor against platform entities
        if (platforms[i].entityType != PLATFORM) {
            return;
        }
        
        Entity platform  = platforms[i];
        
        // check player sensor position against other's position
        
        if (sensorRight.x < (platform.position.x+((platform.width) / 2.0f))){
            senseRX = true;
        } else { senseRX = false; }
        
        if (sensorRight.y < (platform.position.y+((platform.height) / 2.0f))){
            senseRY = true;
        } else { senseRY = false; }
        
        if (senseRX and senseRY and collidedBottom) {
            sensorRightCol = true;
            return;
        }
    }
    
    sensorRightCol = false;
}


// make entity jump
void Entity::Jump()
{
    if (collidedBottom) {
        velocity.y = 5.0f;
    }
}


// starts autonomous walking routine
void Entity::startWalk() {
    // only enemies set to WALKING state will start walk routine
    if (entityType == ENEMY and entityState == WALKING) {
        //do walk routine
        if (entityDir == LEFT) {
            velocity.x = -1.0f;
        }
        else if (entityDir == RIGHT) {
            velocity.x = 1.0f;
        }
    }
}


// check this entity against other
void Entity::Update(float deltaTime, Entity *objects, int objectCount) {
    
    // player collision flags
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    
    // sensor collision flags
    sensorLeftCol = false;
    sensorRightCol = false;
    
    velocity += acceleration * deltaTime;
    
    position.y += velocity.y * deltaTime;        // Move on Y
    CheckCollisionsY(objects, objectCount);    // Fix if needed
    
    position.x += velocity.x * deltaTime;        // Move on X
    CheckCollisionsX(objects, objectCount);    // Fix if needed
    
    sensorLeft.x = position.x - 0.6f;
    sensorLeft.y = position.y - 0.6f;
    CheckSensorLeft(objects, objectCount);
    
    sensorRight.x = position.x + 0.6f;
    sensorRight.y = position.y - 0.6f;
    CheckSensorRight(objects, objectCount);
    
    // check if enemy has killed player
    for (int i = 0; i < objectCount; i++) {
        
        Entity* other = &objects[i]; // this has to be a fucking pointer or else the entityState wont update - took me like 8 hours to realize
        
        // check if enemy has killed player
        if ((entityType == PLAYER and other->entityType == ENEMY) and (collidedLeft or collidedRight)) {
            entityState = DEAD;
        }
        
        // check if player has killed enemy
        if ((entityType == PLAYER and other->entityType == ENEMY) and (collidedBottom and other->collidedTop)) {
            other->entityState = DEAD;
        }
    }
}


// render this entity using shade program
void Entity::Render(ShaderProgram *program) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
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
