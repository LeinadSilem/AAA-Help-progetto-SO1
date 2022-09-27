#ifndef GATCHASHOOTER_H
#define GATCHASHOOTER_H

#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define MAXY 38
#define MAXX 116
#define UP 65
#define DOWN 66
#define ALIEN_SIZE 3
#define PLAYER_SIZE 5
#define ALIENS_ALLOWED 3
#define SHOTS_ALLOWED 2
#define WAVES 2
#define ATK_CHANCE 30
#define MOVESPEED_LVL1 600000
#define MOVESPEED_LVL2 300000
#define SPAWN_CD 10.00

typedef enum {DP, ORFIST, GRAVIDIA, METEOR, CRITICAL} EntityType;
typedef enum {N,S,NEAST,SEAST,NWEST,SWEST,FIXED} Directions;
typedef enum {WHITE, RED, YELLOW, GREEN, CYAN, BLUE, MAGENTA} Color;

typedef struct positions{
    int x;
    int y;
} Pos;

typedef struct hitbox {
    Pos topLeft;
    Pos botRight;
} Hitbox;

typedef struct entities {
    Hitbox hb;
    EntityType e;
    Directions d;
    Color col;
    int lives,id;
    pid_t pid;
} Entity;

typedef struct gamestate {
    Entity player;
    Entity aliensLVL1[ALIENS_ALLOWED*WAVES];
    Entity aliensLVL2[ALIENS_ALLOWED*WAVES];
    _Bool running;
    int aliensLvl1Alive;
    int aliensLvl2Alive;
} Gamestate;

void ship(int pipewrite);
void shoot(int pipewrite, Hitbox pH);
void moveProjectile(int pipewrite, Entity projectile);
void attack(int pipewrite, int attackChance, Hitbox aH);
void alien(int pipewrite, Entity alien);
void alien2(int pipewrite, Entity alien);
void alienGenerator(int pipewrite);
void space(int piperead, int pipewrite);
void projectileCollisions(Entity currentMissile, int pipewrite);
void shipCollisions(Entity temp);
void checkBorderProximity(int topLeftx);
_Bool verifyHitbox(Hitbox a, Hitbox b);
void updateEntity(Entity temp, int pipewrite);
void initializeData();
void drawFieldBorder();
void printer(Entity ent);
void bodyClearing(Entity ent);

#endif