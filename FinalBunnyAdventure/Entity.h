#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Mesh.h"

enum EntityType { PLAYER, PLATFORM, ENEMY, CUBE, SHIP, FLOOR, CRATE, BULLET, ENEMYBULLET, DOOR };

enum AIType { WALKERGROUND, WALKERSTEPS, STATIONARYSHOOTER};
enum AIState { MOVE_RIGHT, MOVE_LEFT, MOVE_BACK, MOVE_FRONT, SHOOTRIGHT, SHOOTLEFT};

class Entity {
public:
    EntityType entityType;
    AIType aiType;
    AIState aiState;
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    glm::vec3 rotation;
    glm::vec3 scale;
    
    float speed;
    bool billboard;
    float width;
    float height;
    float depth;
    
    bool collidedTop = false;
    bool collidedBottom = false;
    bool collidedLeft = false;
    bool collidedRight = false;

    int numLivesLeft = 3;
    int numCarrotsCollected = 0;
    bool startOver = false;
    bool isActive = true;
    bool bulletOn = true;
    bool gunReady = true;
    int bulletCount = 0;
    bool collidedBottomStep = false;
    bool collidedSideStep = false;
    bool reachedDoorAndFinished = false;
    bool freezePlayer = true;
    bool gameStarted = false;
    bool gameOver = false;
    std::string hasWon;

    bool jump = false;
    float jumpPower = 0;
    
    GLuint textureID;
    Mesh* mesh;
    
    glm::mat4 modelMatrix;
    
    Entity();
    
    bool CheckCollision(Entity* other);
    void CheckCollisionsX(Entity* objects, int objectCount);
    void CheckCollisionsY(Entity* objects, int objectCount);
    void CheckCollisionsXObject(Entity* objects, int objectCount);
    void CheckCollisionsYObject(Entity* objects, int objectCount);
    void CheckCollisionsZObject(Entity* objects, int objectCount);
    void AI(Entity* player, Entity* enemyBullets1, Entity* enemyBullets2);
    void AIShooter(Entity* player, Entity* enemyBullets, Entity* enemyBullets2);
    void AIWalkerGround();
    void AIWalkerSteps();
    void shootBullet(Entity* player, Entity* bullets);
    void Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Entity* enemies, int enemyCount, Entity* carrots, int carrotCount, Entity* trees, int treeCount, Entity* bushes, int bushCount, Entity* bullets, Entity* enemyBullets1, Entity* enemyBullets2, Entity* steps, int stepCount, Entity* door);
    void Render(ShaderProgram *program);
    void DrawBillboard(ShaderProgram* program);
};