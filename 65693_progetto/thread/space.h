#ifndef GATCHASHOOTER_H
#define GATCHASHOOTER_H

#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAXY 36
#define MAXX 116
#define UP 65
#define DOWN 66
#define ALIEN_SIZE 3
#define PLAYER_SIZE 5
#define ALIENS_ALLOWED 2
#define WAVES 2
#define MAX_SHOTS 5
#define ATK_CHANCE 40
#define SPAWN_SPEED 10
#define MOVESPEED_LVL1 600000
#define MOVESPEED_LVL2 200000

typedef enum {DP, ORFIST, GRAVIDIA, METEOR, CRITICAL,BLANK} EntityType;
typedef enum {N,S,NEAST,SEAST,NWEST,SWEST,FIXED} Directions;

typedef struct pos{
    int x,y;
}Pos;

typedef struct hitbox{
    Pos topLeft;
    Pos botRight;
}Hitbox;

typedef struct entities{
    EntityType e;
    Directions d;
    Hitbox hb;
    int lives,color;
    pthread_t thr;
    _Bool destroyable,dead;
}Entity;

typedef struct entNode{
    struct entNode* prev;
    Entity data;
    struct entNode* next;
}entityNode;

typedef struct entList{
    entityNode* head;
    entityNode* tail;
    int len;
}entityList;

void* ship(void* param);
void* alien(void* param);
void* space(void* param);
void* bomba(void* param);
void* missile(void* param);
void* alienGenerator(void* param);

int game();
entityList* gameEnding(entityList *list);
void clearField();
void drawFieldBorder();
void printer(Entity* ent);
void bodyClearing(entityNode *body);
void adjustCourse(Entity *a, Entity *b);
void blast(entityList *list, Entity* ship);
void shipCollisions(entityList *list, entityNode *target);
void alienCollisions(entityList *list, entityNode *target);
void bombaCollisions(entityList *list, entityNode *target);
void projectileCollisions(entityList *list, entityNode *target);
void bombaDeployer(entityList *list, Entity *Alien, int attackChance);

_Bool verifyHitbox(Hitbox *a, Hitbox *b);
entityList* eraseEntity(entityNode *forDeletion, entityList *list);

entityNode *newEntity(Entity data);
entityNode *insert(Entity data, entityList *list);

entityList *initEntityList();

#endif