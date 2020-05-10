#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <vector>
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include "Util.h"
#include "Entity.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

glm::mat4 uiViewMatrix, uiProjectionMatrix;
GLuint fontTextureID;
GLuint heartTextureID;

Mix_Music* music;
Mix_Chunk* success;
Mix_Chunk* lostGame;

bool showInstructionsPage0 = true;
bool showInstructionsPage1 = false;
bool showInstructionsPage2 = false;
bool showInstructionsPage3 = false;
bool showInstructionsPage4 = false;

#define OBJECT_COUNT 3
#define ENEMY_COUNT 25
#define CARROT_COUNT 56
#define TREE_COUNT 39
#define BUSH_COUNT 205
#define BULLET_COUNT 10
#define STEP_COUNT 48

struct GameState {
    Entity* player;
    Entity* objects;
    Entity* enemies;
    Entity* carrots;
    Entity* trees;
    Entity* bushes;
    Entity* bullets;
    Entity* steps;
    Entity* enemyBullets1;
    Entity* enemyBullets2;
    Entity* door;
};

GameState state;

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Bunny Adventure!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 1280, 720);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("GameMusic.mp3");
    Mix_PlayMusic(music, -1);

    success = Mix_LoadWAV("successSound.wav");

    lostGame = Mix_LoadWAV("lostGameSound.wav");

    uiViewMatrix = glm::mat4(1.0);
    uiProjectionMatrix = glm::ortho(-6.4f, 6.4f, -3.6f, 3.6f, -1.0f, 1.0f);
    fontTextureID = Util::LoadTexture("font1.png");
    heartTextureID = Util::LoadTexture("platformPack_item017.png");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::perspective(glm::radians(45.0f), 1.777f, 0.1f, 100.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);

    glUseProgram(program.programID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.53f, 0.80f, 0.92f, 1.0f);

    state.player = new Entity();
    state.player->entityType = PLAYER;
    state.player->position = glm::vec3(0, 2.75f, 12);
    state.player->rotation = glm::vec3(0, 0, 0);
    state.player->acceleration = glm::vec3(0, -9.81f, 0);
    state.player->jumpPower = 5.0f;
    state.player->bulletCount = -1;

    GLuint doorTextureID = Util::LoadTexture("door1.png");
    state.door = new Entity();
    state.door->textureID = doorTextureID;
    state.door->entityType = DOOR;
    state.door->billboard = true;
    state.door->position = glm::vec3(0, 3.00f, -120);
    state.door->scale = glm::vec3(8, 8, 8);
    state.door->rotation = glm::vec3(0, 0, 0);
    state.door->acceleration = glm::vec3(0, 0, 0);

    GLuint bulletTextureID = Util::LoadTexture("ballYellowFinal.png");
    state.bullets = new Entity();
    state.bullets->textureID = bulletTextureID;
    state.bullets->billboard = true;
    state.bullets->entityType = BULLET;
    state.bullets->position = glm::vec3(state.player->position.x, 0.50f, state.player->position.z);
    state.bullets->rotation = glm::vec3(0, 0, 0);
    state.bullets->acceleration = glm::vec3(0, 0, 0);
    state.bullets->isActive = false;

    state.enemyBullets1 = new Entity();
    state.enemyBullets1->textureID = bulletTextureID;
    state.enemyBullets1->billboard = true;
    state.enemyBullets1->entityType = ENEMYBULLET;
    state.enemyBullets1->position = glm::vec3(5.0, 0.5, -49);
    state.enemyBullets1->rotation = glm::vec3(0, 0, 0);
    state.enemyBullets1->acceleration = glm::vec3(0, 0, 0);
    state.enemyBullets1->isActive = false;

    state.enemyBullets2 = new Entity();
    state.enemyBullets2->textureID = bulletTextureID;
    state.enemyBullets2->billboard = true;
    state.enemyBullets2->entityType = ENEMYBULLET;
    state.enemyBullets2->position = glm::vec3(-5.0, 0.5, -53);
    state.enemyBullets2->rotation = glm::vec3(0, 0, 0);
    state.enemyBullets2->acceleration = glm::vec3(0, 0, 0);
    state.enemyBullets2->isActive = false;

    state.objects = new Entity[OBJECT_COUNT];

    GLuint floorTextureID = Util::LoadTexture("grass03.png");
    Mesh* cubeMesh = new Mesh();
    cubeMesh->LoadOBJ("cube.obj", 20);

    state.objects[0].textureID = floorTextureID;
    state.objects[0].mesh = cubeMesh;
    state.objects[0].position = glm::vec3(0, -0.25f, 0);
    state.objects[0].rotation = glm::vec3(0, 0, 0);
    state.objects[0].acceleration = glm::vec3(0, 0, 0);
    state.objects[0].scale = glm::vec3(20, 0.5f, 35);
    state.objects[0].entityType = FLOOR;

    state.objects[1].textureID = floorTextureID;
    state.objects[1].mesh = cubeMesh;
    state.objects[1].position = glm::vec3(0, -0.25f, -26);
    state.objects[1].rotation = glm::vec3(0, 0, 0);
    state.objects[1].acceleration = glm::vec3(0, 0, 0);
    state.objects[1].scale = glm::vec3(50, 0.5f, 24);
    state.objects[1].entityType = FLOOR;

    state.objects[2].textureID = floorTextureID;
    state.objects[2].mesh = cubeMesh;
    state.objects[2].position = glm::vec3(0, -0.25f, -92);
    state.objects[2].rotation = glm::vec3(0, 0, 0);
    state.objects[2].acceleration = glm::vec3(0, 0, 0);
    state.objects[2].scale = glm::vec3(15, 0.5f, 120);
    state.objects[2].entityType = FLOOR;

    GLuint stepTextureID = Util::LoadTexture("crate2.png");
    Mesh* crateMesh = new Mesh();
    crateMesh->LoadOBJ("cube.obj", 1);

    state.steps = new Entity[STEP_COUNT];

    int r1 = -28;
    for (int i = 0; i < 8; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(13, 0.5, r1);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r1 += 1;
    }

    int r2 = -28;
    for (int i = 8; i < 16; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(14, 0.5, r2);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r2 += 1;
    }
    int r3 = -28;
    for (int i = 16; i < 24; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(15, 0.5, r3);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r3 += 1;
    }
    int r4 = -28;
    for (int i = 24; i < 32; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(16, 0.5, r4);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r4 += 1;
    }

    int r5 = -28;
    for (int i = 32; i < 40; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(17, 0.5, r5);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r5 += 1;
    }
    int r6 = -28;
    for (int i = 40; i < 48; i++) {
        state.steps[i].textureID = stepTextureID;
        state.steps[i].mesh = crateMesh;
        state.steps[i].position = glm::vec3(18, 0.5, r6);
        state.steps[i].scale = glm::vec3(1, 1, 1);
        state.steps[i].entityType = CRATE;
        r6 += 1;
    }

    state.enemies = new Entity[ENEMY_COUNT];

    GLuint enemyTextureID = Util::LoadTexture("bunny4.png");

    int standEnemy1 = -5;
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].billboard = true;
        state.enemies[i].textureID = enemyTextureID;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].position = glm::vec3(standEnemy1, 0.5, -1);
        state.enemies[i].rotation = glm::vec3(0, 0, 0);
        state.enemies[i].acceleration = glm::vec3(0, 0, 0);
        standEnemy1 += 1;
    }

    state.enemies[3].billboard = true;
    state.enemies[3].textureID = enemyTextureID;
    state.enemies[3].entityType = ENEMY;
    state.enemies[3].aiType = WALKERGROUND;
    state.enemies[3].aiState = MOVE_RIGHT;
    state.enemies[3].position = glm::vec3(-13, 0.5, -26);
    state.enemies[3].rotation = glm::vec3(0, 0, 0);
    state.enemies[3].acceleration = glm::vec3(0, 0, 0);

    state.enemies[4].billboard = true;
    state.enemies[4].textureID = enemyTextureID;
    state.enemies[4].entityType = ENEMY;
    state.enemies[4].aiType = WALKERGROUND;
    state.enemies[4].aiState = MOVE_LEFT;
    state.enemies[4].position = glm::vec3(-18, 0.5, -28);
    state.enemies[4].rotation = glm::vec3(0, 0, 0);
    state.enemies[4].acceleration = glm::vec3(0, 0, 0);

    state.enemies[5].billboard = true;
    state.enemies[5].textureID = enemyTextureID;
    state.enemies[5].entityType = ENEMY;
    state.enemies[5].aiType = WALKERGROUND;
    state.enemies[5].aiState = MOVE_BACK;
    state.enemies[5].position = glm::vec3(-16, 0.5, -32);
    state.enemies[5].rotation = glm::vec3(0, 0, 0);
    state.enemies[5].acceleration = glm::vec3(0, 0, 0);

    state.enemies[6].billboard = true;
    state.enemies[6].textureID = enemyTextureID;
    state.enemies[6].entityType = ENEMY;
    state.enemies[6].aiType = WALKERSTEPS;
    state.enemies[6].aiState = MOVE_LEFT;
    state.enemies[6].position = glm::vec3(12, 0.5, -27);
    state.enemies[6].rotation = glm::vec3(0, 0, 0);
    state.enemies[6].acceleration = glm::vec3(0, 0, 0);

    state.enemies[7].billboard = true;
    state.enemies[7].textureID = enemyTextureID;
    state.enemies[7].entityType = ENEMY;
    state.enemies[7].aiType = WALKERSTEPS;
    state.enemies[7].aiState = MOVE_BACK;
    state.enemies[7].position = glm::vec3(18, 0.5, -29);
    state.enemies[7].rotation = glm::vec3(0, 0, 0);
    state.enemies[7].acceleration = glm::vec3(0, 0, 0);

    state.enemies[8].billboard = true;
    state.enemies[8].textureID = enemyTextureID;
    state.enemies[8].entityType = ENEMY;
    state.enemies[8].aiType = WALKERSTEPS;
    state.enemies[8].aiState = MOVE_RIGHT;
    state.enemies[8].position = glm::vec3(20, 0.5, -21);
    state.enemies[8].rotation = glm::vec3(0, 0, 0);
    state.enemies[8].acceleration = glm::vec3(0, 0, 0);

    state.enemies[9].billboard = true;
    state.enemies[9].textureID = enemyTextureID;
    state.enemies[9].entityType = ENEMY;
    state.enemies[9].aiType = WALKERSTEPS;
    state.enemies[9].aiState = MOVE_FRONT;
    state.enemies[9].position = glm::vec3(13, 0.5, -20);
    state.enemies[9].rotation = glm::vec3(0, 0, 0);
    state.enemies[9].acceleration = glm::vec3(0, 0, 0);

    state.enemies[10].billboard = true;
    state.enemies[10].textureID = enemyTextureID;
    state.enemies[10].entityType = ENEMY;
    state.enemies[10].aiType = STATIONARYSHOOTER;
    state.enemies[10].aiState = SHOOTLEFT;
    state.enemies[10].position = glm::vec3(5.0, 0.5, -49);
    state.enemies[10].rotation = glm::vec3(0, 0, 0);
    state.enemies[10].acceleration = glm::vec3(0, 0, 0);

    state.enemies[11].billboard = true;
    state.enemies[11].textureID = enemyTextureID;
    state.enemies[11].entityType = ENEMY;
    state.enemies[11].aiType = STATIONARYSHOOTER;
    state.enemies[11].aiState = SHOOTRIGHT;
    state.enemies[11].position = glm::vec3(-5.0, 0.5, -53);
    state.enemies[11].rotation = glm::vec3(0, 0, 0);
    state.enemies[11].acceleration = glm::vec3(0, 0, 0);

    int standEnemy2x = -6;
    for (int i = 12; i < 25; i++) {
        state.enemies[i].billboard = true;
        state.enemies[i].textureID = enemyTextureID;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].position = glm::vec3(standEnemy2x, 0.5, -110);
        state.enemies[i].rotation = glm::vec3(0, 0, 0);
        state.enemies[i].acceleration = glm::vec3(0, 0, 0);
        standEnemy2x += 1;
    }

    state.carrots = new Entity[CARROT_COUNT];

    GLuint carrot2TextureID = Util::LoadTexture("carrot2.png");

    int carrotSet1 = -2;
    for (int i = 0; i < 6; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(-4, 0.1, carrotSet1);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet1 -= 1;
    }

    int carrotSet2ax = -14;
    int carrotSet2az = -27;
    for (int i = 6; i < 10; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet2ax, 0.1, carrotSet2az);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet2az -= 1;
    }

    int carrotSet2bx = -15;
    int carrotSet2bz = -27;
    for (int i = 10; i < 14; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet2bx, 0.1, carrotSet2bz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet2bz -= 1;
    }

    int carrotSet2cx = -16;
    int carrotSet2cz = -27;
    for (int i = 14; i < 18; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet2cx, 0.1, carrotSet2cz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet2cz -= 1;
    }

    int carrotSet2dx = -17;
    int carrotSet2dz = -27;
    for (int i = 18; i < 22; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet2dx, 0.1, carrotSet2dz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet2dz -= 1;
    }

    int carrotSet3az = -26;
    for (int i = 22; i < 27; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(14.5, 1.1, carrotSet3az);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet3az += 1;
    }

    int carrotSet3bz = -26;
    for (int i = 27; i < 32; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(15.5, 1.1, carrotSet3bz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet3bz += 1;
    }

    int carrotSet3cz = -26;
    for (int i = 32; i < 37; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(16.5, 1.1, carrotSet3cz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet3cz += 1;
    }

    int carrotSet4ax = 0;
    int carrotSet4az = -56;
    for (int i = 37; i < 41; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4ax, 0.1, carrotSet4az);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4ax += 0.5f;
        carrotSet4az -= 2.0f;
    }

    int carrotSet4bx = 0;
    int carrotSet4bz = -64.5;
    for (int i = 41; i < 44; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4bx, 0.1, carrotSet4bz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4bx += 1;
        carrotSet4bz -= 2.0f;
    }

    int carrotSet4cx = 3;
    int carrotSet4cz = -70;
    for (int i = 44; i < 47; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4cx, 0.1, carrotSet4cz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4cx -= 1;
        carrotSet4cz -= 2.0f;
    }

    int carrotSet4dx = 0;
    int carrotSet4dz = -76;
    for (int i = 47; i < 50; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4dx, 0.1, carrotSet4dz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4dx -= 1;
        carrotSet4dz -= 2.0f;
    }

    int carrotSet4ex = -3;
    int carrotSet4ez = -82;
    for (int i = 50; i < 53; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4ex, 0.1, carrotSet4ez);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4ex += 1;
        carrotSet4ez -= 2.0f;
    }

    int carrotSet4fx = 0;
    int carrotSet4fz = -88;
    for (int i = 53; i < 56; i++) {
        state.carrots[i].billboard = true;
        state.carrots[i].textureID = carrot2TextureID;
        state.carrots[i].position = glm::vec3(carrotSet4fx, 0.1, carrotSet4fz);
        state.carrots[i].scale = glm::vec3(0.5, 0.5, 0.5);
        state.carrots[i].rotation = glm::vec3(0, 0, 0);
        state.carrots[i].acceleration = glm::vec3(0, 0, 0);
        carrotSet4fz -= 2.0f;
    }

    state.trees = new Entity[TREE_COUNT];

    GLuint treeTextureID = Util::LoadTexture("tree1.png");

    state.trees[0].billboard = true;
    state.trees[0].textureID = treeTextureID;
    state.trees[0].position = glm::vec3(-7, 2.0, 16);
    state.trees[0].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[0].rotation = glm::vec3(0, 0, 0);
    state.trees[0].acceleration = glm::vec3(0, 0, 0);

    state.trees[1].billboard = true;
    state.trees[1].textureID = treeTextureID;
    state.trees[1].position = glm::vec3(-4.5, 2.0, 16);
    state.trees[1].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[1].rotation = glm::vec3(0, 0, 0);
    state.trees[1].acceleration = glm::vec3(0, 0, 0);

    state.trees[2].billboard = true;
    state.trees[2].textureID = treeTextureID;
    state.trees[2].position = glm::vec3(-2, 2.0, 16);
    state.trees[2].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[2].rotation = glm::vec3(0, 0, 0);
    state.trees[2].acceleration = glm::vec3(0, 0, 0);

    state.trees[3].billboard = true;
    state.trees[3].textureID = treeTextureID;
    state.trees[3].position = glm::vec3(0.5, 2.0, 16);
    state.trees[3].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[3].rotation = glm::vec3(0, 0, 0);
    state.trees[3].acceleration = glm::vec3(0, 0, 0);

    state.trees[4].billboard = true;
    state.trees[4].textureID = treeTextureID;
    state.trees[4].position = glm::vec3(3, 2.0, 16);
    state.trees[4].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[4].rotation = glm::vec3(0, 0, 0);
    state.trees[4].acceleration = glm::vec3(0, 0, 0);

    state.trees[5].billboard = true;
    state.trees[5].textureID = treeTextureID;
    state.trees[5].position = glm::vec3(5.5, 2.0, 16);
    state.trees[5].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[5].rotation = glm::vec3(0, 0, 0);
    state.trees[5].acceleration = glm::vec3(0, 0, 0);

    state.trees[6].billboard = true;
    state.trees[6].textureID = treeTextureID;
    state.trees[6].position = glm::vec3(8, 2.0, 16);
    state.trees[6].scale = glm::vec3(3.5, 4.0, 3.0);
    state.trees[6].rotation = glm::vec3(0, 0, 0);
    state.trees[6].acceleration = glm::vec3(0, 0, 0);

    int startTree2a = 14;
    for (int i = 7; i < 16; i++) {
        state.trees[i].billboard = true;
        state.trees[i].textureID = treeTextureID;
        state.trees[i].position = glm::vec3(-8, 2.0, startTree2a);
        state.trees[i].scale = glm::vec3(3.5, 4.0, 3.0);
        state.trees[i].rotation = glm::vec3(0, 0, 0);
        state.trees[i].acceleration = glm::vec3(0, 0, 0);
        startTree2a -= 1.5;
    }

    int startTree2b = startTree2a;
    for (int i = 16; i < 23; i++) {
        state.trees[i].billboard = true;
        state.trees[i].textureID = treeTextureID;
        state.trees[i].position = glm::vec3(-8, 2.0, startTree2b);
        state.trees[i].scale = glm::vec3(3.5, 4.0, 3.0);
        state.trees[i].rotation = glm::vec3(0, 0, 0);
        state.trees[i].acceleration = glm::vec3(0, 0, 0);
        startTree2b -= 2.5;
    }

    int startTree3 = 14;
    for (int i = 23; i < 32; i++) {
        state.trees[i].billboard = true;
        state.trees[i].textureID = treeTextureID;
        state.trees[i].position = glm::vec3(8, 2.0, startTree3);
        state.trees[i].scale = glm::vec3(3.5, 4.0, 3.0);
        state.trees[i].rotation = glm::vec3(0, 0, 0);
        state.trees[i].acceleration = glm::vec3(0, 0, 0);
        startTree3 -= 1.5;
    }
    int startTree4 = startTree3;
    for (int i = 32; i < TREE_COUNT; i++) {
        state.trees[i].billboard = true;
        state.trees[i].textureID = treeTextureID;
        state.trees[i].position = glm::vec3(8, 2.0, startTree4);
        state.trees[i].scale = glm::vec3(3.5, 4.0, 3.0);
        state.trees[i].rotation = glm::vec3(0, 0, 0);
        state.trees[i].acceleration = glm::vec3(0, 0, 0);
        startTree4 -= 2.5;
    }

    state.bushes = new Entity[BUSH_COUNT];

    GLuint bushTextureID = Util::LoadTexture("bush1.png");

    int startBush1 = -9;
    for (int i = 0; i < 11; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(startBush1, 0.5, 17);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush1 += 2.0;
    }

    int startBush2a = 15;
    for (int i = 11; i < 18; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(-9, 0.5, startBush2a);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush2a -= 2.0;
    }

    state.bushes[18].billboard = true;
    state.bushes[18].textureID = bushTextureID;
    state.bushes[18].position = glm::vec3(-9, 0.5, 0.5);
    state.bushes[18].scale = glm::vec3(3, 3, 3);
    state.bushes[18].rotation = glm::vec3(0, 0, 0);
    state.bushes[18].acceleration = glm::vec3(0, 0, 0);

    int startBush2b = 15;
    for (int i = 19; i < 32; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(-9, 0.5, startBush2b);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush2b -= 2.7;
    }

    int startBush3a = 15;
    for (int i = 32; i < 39; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(9, 0.5, startBush3a);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush3a -= 2.0;
    }

    state.bushes[39].billboard = true;
    state.bushes[39].textureID = bushTextureID;
    state.bushes[39].position = glm::vec3(9, 0.5, 0.5);
    state.bushes[39].scale = glm::vec3(3, 3, 3);
    state.bushes[39].rotation = glm::vec3(0, 0, 0);
    state.bushes[39].acceleration = glm::vec3(0, 0, 0);

    int startBush3b = 15;
    for (int i = 40; i < 53; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(9, 0.5, startBush3b);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush3b -= 2.7;
    }

    int startBush4 = -9;
    for (int i = 53; i < 61; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(startBush4, 0.5, -14);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush4 -= 2.7;
    }

    int startBush5 = -14;
    for (int i = 61; i < 74; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(-25, 0.5, startBush5);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush5 -= 2.7;
    }

    int startBush6 = -25;
    for (int i = 74; i < 84; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(startBush6, 0.5, -37);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush6 += 2.0;
    }

    int startBush7 = 9;
    for (int i = 84; i < 92; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(startBush7, 0.5, -14);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush7 += 2.7;
    }

    int startBush8 = -14;
    for (int i = 92; i < 105; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(25, 0.5, startBush8);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush8 -= 2.7;
    }

    int startBush9 = 25;
    for (int i = 105; i < 115; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(startBush9, 0.5, -37);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush9 -= 2.0;
    }

    int startBush10 = -38;
    for (int i = 115; i < 160; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(-7, 0.5, startBush10);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush10 -= 2.7;
    }

    int startBush11 = -38;
    for (int i = 160; i < 205; i++) {
        state.bushes[i].billboard = true;
        state.bushes[i].textureID = bushTextureID;
        state.bushes[i].position = glm::vec3(7, 0.5, startBush11);
        state.bushes[i].scale = glm::vec3(3, 3, 3);
        state.bushes[i].rotation = glm::vec3(0, 0, 0);
        state.bushes[i].acceleration = glm::vec3(0, 0, 0);
        startBush11 -= 2.7;
    }
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {

            case SDLK_SPACE:
                if (state.player->freezePlayer == false) {
                    if (state.player->collidedBottom) {
                        state.player->jump = true;
                    }
                }
                break;
            case SDLK_RETURN:
                if (showInstructionsPage0 == true && showInstructionsPage1 == false && showInstructionsPage2 == false && showInstructionsPage3 == false && showInstructionsPage4 == false) {
                    showInstructionsPage0 = false;
                    showInstructionsPage1 = true;
                }
                else if (showInstructionsPage0 == false && showInstructionsPage1 == true && showInstructionsPage2 == false && showInstructionsPage3 == false && showInstructionsPage4 == false) {
                    showInstructionsPage1 = false;
                    showInstructionsPage2 = true;
                }
                else if (showInstructionsPage0 == false && showInstructionsPage1 == false && showInstructionsPage2 == true && showInstructionsPage3 == false && showInstructionsPage4 == false) {
                    showInstructionsPage2 = false;
                    showInstructionsPage3 = true;
                }
                else if (showInstructionsPage0 == false && showInstructionsPage1 == false && showInstructionsPage2 == false && showInstructionsPage3 == true && showInstructionsPage4 == false) {
                    showInstructionsPage3 = false;
                    showInstructionsPage4 = true;
                }
                else if (showInstructionsPage0 == false && showInstructionsPage1 == false && showInstructionsPage2 == false && showInstructionsPage3 == false && showInstructionsPage4 == true) {
                    showInstructionsPage4 = false;
                    state.player->freezePlayer = false;
                    state.player->gameStarted = true;
                }
            }

            break;
        }
    }
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        if (state.player->freezePlayer == false) {
            state.player->rotation.y += 0.21f;
        }
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        if (state.player->freezePlayer == false) {
            state.player->rotation.y -= 0.21f;
        }
    }

    if (keys[SDL_SCANCODE_S]) {
        if (state.player->freezePlayer == false) {
            state.player->shootBullet(state.player, state.bullets);
        }
    }

    state.player->velocity.x = 0;
    state.player->velocity.z = 0;

    if (keys[SDL_SCANCODE_UP]) {
        if (state.player->freezePlayer == false) {
            state.player->velocity.z = cos(glm::radians(state.player->rotation.y)) * -3.5f;
            state.player->velocity.x = sin(glm::radians(state.player->rotation.y)) * -3.5f;
        }
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        if (state.player->freezePlayer == false) {
            state.player->velocity.z = cos(glm::radians(state.player->rotation.y)) * 3.5f;
            state.player->velocity.x = sin(glm::radians(state.player->rotation.y)) * 3.5f;
        }
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        state.player->Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);

        for (int i = 0; i < OBJECT_COUNT; i++) {
            state.objects[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        for (int i = 0; i < CARROT_COUNT; i++) {
            state.carrots[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        for (int i = 0; i < TREE_COUNT; i++) {
            state.trees[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        for (int i = 0; i < BUSH_COUNT; i++) {
            state.bushes[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        for (int i = 0; i < STEP_COUNT; i++) {
            state.steps[i].Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);
        }

        state.bullets->Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);

        state.enemyBullets1->Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);

        state.enemyBullets2->Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);

        state.door->Update(FIXED_TIMESTEP, state.player, state.objects, OBJECT_COUNT, state.enemies, ENEMY_COUNT, state.carrots, CARROT_COUNT, state.trees, TREE_COUNT, state.bushes, BUSH_COUNT, state.bullets, state.enemyBullets1, state.enemyBullets2, state.steps, STEP_COUNT, state.door);

        if (state.player->numLivesLeft == 0) {
            state.player->hasWon = "No";
            Mix_PlayChannel(-1, lostGame, 0);
            state.player->gameOver = true;
        }
        else if (state.player->reachedDoorAndFinished == true) {
            state.player->hasWon = "Yes";
            Mix_PlayChannel(-1, success, 0);
            state.player->gameOver = true;
        }
        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;

    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::rotate(viewMatrix, glm::radians(state.player->rotation.y), glm::vec3(0, -1.0f, 0));
    viewMatrix = glm::translate(viewMatrix, -state.player->position);
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    if ((state.player->gameStarted == false)) {
        if (showInstructionsPage0 == true) {
            Util::DrawText(&program, fontTextureID, "Welcome to... ", 0.6f, -0.25f, glm::vec3(-4.8f, 3.0f, 0));
            Util::DrawText(&program, fontTextureID, "BUNNY HOPPIN'S", 1.0f, -0.25f, glm::vec3(-4.8f, 1.8f, 0));
            Util::DrawText(&program, fontTextureID, "FUN ADVENTURE", 1.0f, -0.25f, glm::vec3(-4.5f, 0.6f, 0));
            Util::DrawText(&program, fontTextureID, "Retrieval of the", 0.6f, -0.25f, glm::vec3(0.0f, -0.3f, 0));
            Util::DrawText(&program, fontTextureID, "pesky carrots!", 0.6f, -0.25f, glm::vec3(0.0f, -1.0f, 0));
            Util::DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(2.8f, -2.6f, 0));
            Util::DrawText(&program, fontTextureID, "to the next page...", 0.40f, -0.15f, glm::vec3(2.8f, -3.1f, 0));
        }
        if (showInstructionsPage1 == true) {
            Util::DrawText(&program, fontTextureID, "THE STORY ", 0.8f, -0.25f, glm::vec3(-2.3f, 4.3f, 0));
            Util::DrawText(&program, fontTextureID, "Your name is Bunny Hoppins.", 0.45f, -0.20f, glm::vec3(-6.3f, 2.9f, 0));
            Util::DrawText(&program, fontTextureID, "A nearby village of bunnies has stolen all of your", 0.45f, -0.20f, glm::vec3(-6.3f, 1.7f, 0));
            Util::DrawText(&program, fontTextureID, "village's carrots! You have been chosen by your ", 0.45f, -0.20f, glm::vec3(-6.3f, 0.5f, 0));
            Util::DrawText(&program, fontTextureID, "village to get all of the carrots back. ", 0.45f, -0.20f, glm::vec3(-6.3f, -0.7f, 0));
            Util::DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(2.8f, -2.6f, 0));
            Util::DrawText(&program, fontTextureID, "to the next page...", 0.40f, -0.15f, glm::vec3(2.8f, -3.1f, 0));
        }
        if (showInstructionsPage2 == true) {
            Util::DrawText(&program, fontTextureID, "THE STORY ", 0.8f, -0.25f, glm::vec3(-2.3f, 4.3f, 0));
            Util::DrawText(&program, fontTextureID, "You must collect all of the carrots!", 0.45f, -0.20f, glm::vec3(-6.3f, 2.9f, 0));
            Util::DrawText(&program, fontTextureID, "You can throw a lemon to get rid of an enemy bunny.", 0.45f, -0.20f, glm::vec3(-6.3f, 1.7f, 0));
            Util::DrawText(&program, fontTextureID, "But remember! Don't let any of their lemons touch you!", 0.45f, -0.20f, glm::vec3(-6.3f, 0.5f, 0));
            Util::DrawText(&program, fontTextureID, "Once you have collected every carrot, go to the door.", 0.45f, -0.20f, glm::vec3(-6.3f, -0.7f, 0));
            Util::DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(2.8f, -2.6f, 0));
            Util::DrawText(&program, fontTextureID, "to the next page...", 0.40f, -0.15f, glm::vec3(2.8f, -3.1f, 0));

        }
        if (showInstructionsPage3 == true) {
            Util::DrawText(&program, fontTextureID, "Instructions: ", 0.8f, -0.25f, glm::vec3(-3.3f, 4.3f, 0));
            Util::DrawText(&program, fontTextureID, "Press the RIGHT ARROW key (->) to look right", 0.45f, -0.20f, glm::vec3(-5.1f, 2.9f, 0));
            Util::DrawText(&program, fontTextureID, "Press the LEFT ARROW key (<-) to look left", 0.45f, -0.20f, glm::vec3(-5.1f, 2.1f, 0));
            Util::DrawText(&program, fontTextureID, "Press the UP ARROW key to move forward", 0.45f, -0.20f, glm::vec3(-5.1f, 1.3f, 0));
            Util::DrawText(&program, fontTextureID, "Press the DOWN ARROW key to move back", 0.45f, -0.20f, glm::vec3(-5.1f, 0.5f, 0));
            Util::DrawText(&program, fontTextureID, "Press the SPACEBAR to jump", 0.45f, -0.20f, glm::vec3(-5.1f, -0.3f, 0));
            Util::DrawText(&program, fontTextureID, "Press the 'S' key to shoot a lemon", 0.45f, -0.20f, glm::vec3(-5.1f, -1.1f, 0));
            Util::DrawText(&program, fontTextureID, "You can only shoot 1 lemon at a time!", 0.45f, -0.20f, glm::vec3(-5.1f, -1.9f, 0));
            Util::DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(2.8f, -2.6f, 0));
            Util::DrawText(&program, fontTextureID, "to the next page...", 0.40f, -0.15f, glm::vec3(2.8f, -3.1f, 0));
        }

        if (showInstructionsPage4 == true) {
            Util::DrawText(&program, fontTextureID, "Good Luck! ", 0.6f, -0.25f, glm::vec3(-4.7f, 2.8f, 0));
            Util::DrawText(&program, fontTextureID, "Remember! Go to the door ONLY after you", 0.45f, -0.20f, glm::vec3(-4.7f, 1.5f, 0));
            Util::DrawText(&program, fontTextureID, "have collected ALL of the carrots.", 0.45f, -0.20f, glm::vec3(-4.7f, 0.2f, 0));
            Util::DrawText(&program, fontTextureID, "Press 'Enter' to go", 0.40f, -0.15f, glm::vec3(2.8f, -2.6f, 0));
            Util::DrawText(&program, fontTextureID, "begin the game...", 0.40f, -0.15f, glm::vec3(2.8f, -3.1f, 0));

        }
    }
    else if ((state.player->gameStarted == true) && (state.player->gameOver == false)) {
        for (int i = 0; i < OBJECT_COUNT; i++) {
            state.objects[i].Render(&program);
        }

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Render(&program);
        }

        for (int i = 0; i < CARROT_COUNT; i++) {
            state.carrots[i].Render(&program);
        }

        for (int i = 0; i < TREE_COUNT; i++) {
            state.trees[i].Render(&program);
        }

        for (int i = 0; i < BUSH_COUNT; i++) {
            state.bushes[i].Render(&program);
        }

        for (int i = 0; i < STEP_COUNT; i++) {
            state.steps[i].Render(&program);
        }

        state.bullets->Render(&program);

        state.enemyBullets1->Render(&program);

        state.enemyBullets2->Render(&program);

        state.door->Render(&program);

        program.SetProjectionMatrix(uiProjectionMatrix);
        program.SetViewMatrix(uiViewMatrix);

        std::string s = std::to_string(state.player->numLivesLeft);
        std::string s2 = std::to_string(state.player->numCarrotsCollected);
        std::string s3 = std::to_string(CARROT_COUNT);

        Util::DrawText(&program, fontTextureID, "Lives: " + s, 0.5, -0.3f, glm::vec3(-6, 3.2, 0));
        Util::DrawText(&program, fontTextureID, "Carrots Collected: " + s2 + "/" + s3, 0.5, -0.3f, glm::vec3(-2, 3.2, 0));
        for (int i = 0; i < state.player->numLivesLeft; i++)
        {
            // These icons are small, so just move 0.5 to the right for each one.
            Util::DrawIcon(&program, heartTextureID, glm::vec3(5 + (i * 0.5f), 3.2, 0));
        }
    }

    else if (state.player->gameOver == true) {
        if (state.player->hasWon == "Yes") {
            Util::DrawText(&program, fontTextureID, "YOU WIN!", 1.0, -0.3f, glm::vec3(-2.5, 2.2, 0));
            state.player->freezePlayer = true;
        }
        if (state.player->hasWon == "No") {
            Util::DrawText(&program, fontTextureID, "GAME OVER!", 0.95, -0.3f, glm::vec3(-2.8, 2.4, 0));
            state.player->freezePlayer = true;
        }
    }

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}
