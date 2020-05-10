#include "Entity.h"
#include <SDL_mixer.h>

Mix_Chunk* collectCarrot;
Mix_Chunk* lostLife;

Entity::Entity()
{
    position = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    rotation = glm::vec3(0);
    scale = glm::vec3(1);

    modelMatrix = glm::mat4(1.0f);
    
    speed = 0.0f;

    billboard = false;

    width = 1.0f;
    height = 1.0f;
    depth = 1.0f;

    collectCarrot = Mix_LoadWAV("CollectCarrot.wav");
    lostLife = Mix_LoadWAV("lostLifeSound.wav");
}

bool Entity::CheckCollision(Entity* other)
{
    if (isActive == false || other->isActive == false) return false;

    float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    float zdist = fabs(position.z - other->position.z) - ((depth + other->depth) / 2.0f);
    if (xdist < 0 && ydist < 0 && zdist < 0) return true;

    return false;
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (glm::distance(position.x, objects->position.x) < 1)
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
            }
        }
    }
}

void Entity::CheckCollisionsXObject(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object->position.x);
            float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
            }
        }
    }
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (glm::distance(position.y, objects->position.y) < 1)
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
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

void Entity::CheckCollisionsYObject(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object->position.y);
            float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
                collidedBottomStep = true;
            }
        }
    }
}

void Entity::CheckCollisionsZObject(Entity* objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity* object = &objects[i];
        if (CheckCollision(object))
        {
            float zdist = fabs(position.z - object->position.z);
            float penetrationZ = fabs(zdist - (width / 2.0f) - (object->width / 2.0f));
            if (velocity.z > 0) {
                position.z -= penetrationZ;
                velocity.z = 0;
            }
            else if (velocity.z < 0) {
                position.z += penetrationZ;
                velocity.z = 0;
            }
        }
    }
}

void Entity::AIWalkerGround() {
    velocity.x = 0;
    velocity.z = 0;
    switch (aiState) {
    case MOVE_RIGHT:
        velocity.x = 0;
        velocity.z = -2;
        if (position.z <= -32.0f) {
            aiState = MOVE_BACK;
        }
        break;
    case MOVE_BACK:
        velocity.x = -2;
        velocity.z = 0;
        if (position.x <= -18.0f) {
            aiState = MOVE_LEFT;
        }
        break;
    case MOVE_LEFT:
        velocity.x = 0;
        velocity.z = 2;
        if (position.z >= -26.0f) {
            aiState = MOVE_FRONT;
        }
        break;
    case MOVE_FRONT:
        velocity.x = 2;
        velocity.z = 0;
        if (position.x >= -13.0f) {
            aiState = MOVE_RIGHT;
        }
        break;
    }
}

void Entity::AIWalkerSteps() {
    velocity.x = 0;
    velocity.z = 0;
    switch (aiState) {
    case MOVE_LEFT:
        velocity.x = 0;
        velocity.z = -3;
        if (position.z <= -29.0f) {
            aiState = MOVE_BACK;
        }
        break;
    case MOVE_BACK:
        velocity.x = 3;
        velocity.z = 0;
        if (position.x >= 19.0f) {
            aiState = MOVE_RIGHT;
        }
        break;
    case MOVE_RIGHT:
        velocity.x = 0;
        velocity.z = 3;
        if (position.z >= -20.0f) {
            aiState = MOVE_FRONT;
        }
        break;
    case MOVE_FRONT:
        velocity.x = -3;
        velocity.z = 0;
        if (position.x <= 12.0f) {
            aiState = MOVE_LEFT;
        }
        break;
    }
}

void Entity::AIShooter(Entity* player, Entity* enemyBullets1, Entity* enemyBullets2) {
    switch (aiState) {
    case SHOOTRIGHT:
        enemyBullets2->isActive = true;
        enemyBullets2->velocity.x = 11;
        enemyBullets2->velocity.z = 0;

        if (glm::distance(enemyBullets2->position, player->position) < 0.80f) {
            if (player->startOver == false) {
                player->numLivesLeft -= 1;
                player->startOver = true;
            }
        }

        if (glm::distance(enemyBullets2->position.x, position.x) > 11.0f) {
            enemyBullets2->isActive = false;
            enemyBullets2->position = position;
            aiState = SHOOTRIGHT;

        }
        break;
    case SHOOTLEFT:
        enemyBullets1->isActive = true;
        enemyBullets1->velocity.x = -11;
        enemyBullets1->velocity.z = 0;

        if (glm::distance(enemyBullets1->position, player->position) < 0.80f) {
            if (player->startOver == false) {
                player->numLivesLeft -= 1;
                player->startOver = true;
            }
        }

        if (glm::distance(enemyBullets1->position.x, position.x) > 11.0f) {
            enemyBullets1->isActive = false;
            enemyBullets1->position = position;
            aiState = SHOOTLEFT;
            
        }
        break;
    }
     
}

void Entity::AI(Entity* player, Entity* enemyBullets1, Entity* enemyBullets2) {
    switch (aiType) {
    case WALKERGROUND:
        AIWalkerGround();
        break;
    case WALKERSTEPS:
        AIWalkerSteps();
        break;
    case STATIONARYSHOOTER:
        AIShooter(player, enemyBullets1, enemyBullets2);
        break;
    }
}

void Entity::shootBullet(Entity* player, Entity* bullets) {
    if (isActive == false) return;

    if (player->gunReady == true) {
        player->gunReady = false;
        bullets->isActive = true;
        bullets->position.x = position.x;
        bullets->position.z = position.z;
        bullets->position.y = position.y - 0.25f;
        bullets->velocity.z = cos(glm::radians(rotation.y)) * -4.0f;
        bullets->velocity.x = sin(glm::radians(rotation.y)) * -4.0f;
    }
}

void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Entity* enemies, int enemyCount, Entity* carrots, int carrotCount, Entity* trees, int treeCount, Entity* bushes, int bushCount, Entity* bullets, Entity* enemyBullets1, Entity* enemyBullets2, Entity* steps, int stepCount, Entity* door)
{
    if (isActive == false) return;

    glm::vec3 previousPosition = position;
    
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    collidedBottomStep = false;
    
    if (billboard) {
        float directionX = position.x - player->position.x;
        float directionZ = position.z - player->position.z;
        rotation.y = glm::degrees(atan2f(directionX, directionZ));
    }
    
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    if (entityType == ENEMY) {
        AI(player, enemyBullets1, enemyBullets2);
    }

    if (entityType == BULLET) {
        float directionX = position.x - player->position.x;
        float directionZ = position.z - player->position.z;
        rotation.y = glm::degrees(atan2f(directionX, directionZ));

        velocity.z = cos(glm::radians(rotation.y)) * 12.0f;
        velocity.x = sin(glm::radians(rotation.y)) * 12.0f;

        if (glm::distance(position, player->position) > 16.0f) {
            isActive = false;
            player->gunReady = true;
        }
        for (int i = 0; i < enemyCount; i++) {
            if (CheckCollision(&enemies[i])) {
                enemies[i].isActive = false;
                isActive = false;
                player->gunReady = true;
                if (i == 10) {
                    enemyBullets1->isActive = false;
                }
                if (i == 11) {
                    enemyBullets2->isActive = false;
                }
            }
        }
        for (int i = 0; i < treeCount; i++) {
            if (CheckCollision(&trees[i])) {
                isActive = false;
                player->gunReady = true;
            }
        }
        for (int i = 0; i < bushCount; i++) {
            if (CheckCollision(&bushes[i])) {
                isActive = false;
                player->gunReady = true;
            }
        }

        for (int i = 0; i < stepCount; i++) {
            if (CheckCollision(&steps[i])) {
                isActive = false;
                player->gunReady = true;
            }
        }
    }

    if (entityType == PLAYER) {

        if (startOver == true) {
            position = glm::vec3(0, 0.75f, 12);
            rotation = glm::vec3(0, 0, 0);
            Mix_PlayChannel(-1, lostLife, 0);
            Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
            startOver = false;
        }

        if (position.z <= -119) {
            if (numCarrotsCollected == carrotCount) {
                player->reachedDoorAndFinished = true;
                position = glm::vec3(0, 0.75f, 12);
                rotation = glm::vec3(0, 0, 0);
            }
            else {
                position = previousPosition;
            }
        }

        

        for (int i = 0; i < enemyCount; i++) {
            if (CheckCollision(&enemies[i])) {
                if (startOver == false) {
                    numLivesLeft -= 1;
                    startOver = true;
                }  
            }
        }

        for (int i = 0; i < carrotCount; i++) {
            if (CheckCollision(&carrots[i])) {
                numCarrotsCollected += 1;
                carrots[i].isActive = false;
                Mix_PlayChannel(-1, collectCarrot, 0);
                Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
            }
        }

        for (int i = 0; i < treeCount; i++) {
            if (CheckCollision(&trees[i])) {
                position = previousPosition;
                break;
            }
        }
        for (int i = 0; i < bushCount; i++) {
            if(CheckCollision(&bushes[i])) {
                position = previousPosition;
                break;
            }
        }   
        CheckCollisionsY(objects, objectCount);
        CheckCollisionsYObject(steps, stepCount);
        
        for (int i = 0; i < stepCount; i++) {
            if (CheckCollision(&steps[i])) {
                if (collidedBottomStep) {
                    position.y = previousPosition.y;
                }
                else {
                    position.x = previousPosition.x;
                    position.y = previousPosition.y;
                    position.z = previousPosition.z;
                }                
                collidedBottom = true;
                break;
            }
        }
        CheckCollisionsY(objects, objectCount);
        CheckCollisionsYObject(steps, stepCount);

        if (jump) {
            jump = false;
            velocity.y += jumpPower;
            position.y += velocity.y * deltaTime;
        }
    }

    if (entityType == CUBE) {
        rotation.y += 45 * deltaTime;
        rotation.z += 45 * deltaTime;
    }
    else if (entityType == ENEMY) {
        rotation.y += 30 * deltaTime;
    }    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
}

void Entity::Render(ShaderProgram *program) {
    if (isActive == false) return;

    program->SetModelMatrix(modelMatrix);
    
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (billboard) {
        DrawBillboard(program);
    }
    else {
        mesh->Render(program);
    }
}

void Entity::DrawBillboard(ShaderProgram* program) {
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

