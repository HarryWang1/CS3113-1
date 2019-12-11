#include "Entity.h"
#include <SDL_mixer.h>

// construct entity
Entity::Entity()
{
    entityType = PLATFORM;
    isStatic = true;
    isActive = true;
    position = glm::vec3(0);
    startPosition = glm::vec3(-3.5, 3.5, 0);
    speed = 0;
    width = 1;
    height = 1;
    lives = 2;
}

// check collisions
bool Entity::CheckCollision(Entity* other) {
    if (isStatic == true) return false;
    if (isActive == false || other->isActive == false) return false;
    if (other->entityState == DEAD) return false; // only non-dead objects can collide

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
void Entity::CheckCollisionsY(Entity* objects, int objectCount) {
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
            else if (velocity.y == 0) {
                velocity.y = 0;
                collidedTop = true;
                collidedBottom = true;
                object->collidedTop = true;
                object->collidedBottom = true;
            }
        }
    }
}


// check collision and adjust x axis
void Entity::CheckCollisionsX(Entity* objects, int objectCount) {
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
            else if (velocity.x == 0) {
                velocity.x = 0;
                collidedLeft = true;
                collidedRight = true;
                object->collidedRight = true;
                object->collidedLeft = true;
            }
        }
    }
}


void Entity::EnemyAttributes() {

    if (entityType == ENEMY) {
        if (position.y > 2) {
            position.y = -2.5;
        }
        else if (position.x < -5) {
            position.x = 4;
        }
        if (entityState == SIDE) {
            velocity = glm::vec3(rate, 0, 0);
        }
        else {
            velocity = glm::vec3(0, rate, 0);
        }

    }
}

// check this entity against other
void Entity::Update(float deltaTime, Entity* objects, int objectCount) {
 
    // player collision flags
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;

    velocity += acceleration * deltaTime;
    position.y += velocity.y * deltaTime;        // Move on Y
    position.x += velocity.x * deltaTime;        // Move on X

    // dont collision detect with enemies
    if (entityType == ENEMY) { return; }

    CheckCollisionsY(objects, objectCount);
    CheckCollisionsX(objects, objectCount);

    // check if enemy has killed player
    for (int i = 0; i < objectCount; i++) {

        Entity* other = &objects[i]; // this has to be a fucking pointer or else the entityState wont update - took me like 8 hours to realize
        if ((collidedLeft or collidedRight or collidedTop or collidedBottom) and (entityType == PLAYER and other->entityType == APPLE)) {
            other->gotApple = true;
        }

        // if any collision happened between player and enemy
        if ((collidedLeft or collidedRight or collidedTop or collidedBottom) and (entityType == PLAYER and other->entityType == ENEMY )  ) {
     
                // lock helps stabalize the count of player lives
                if (lives > 0) {
                    lifeLock = true;
                    //set posiiton back to start
                    position = startPosition;
                }
                else {
                    entityState = DEAD;
                }

            }
        
    }
}



// render this entity using shade program
void Entity::Render(ShaderProgram* program) {
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
