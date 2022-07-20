#ifndef GATCHASHOOTER_H
#define GATCHASHOOTER_H

#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>

#define MAXY 24
#define MAXX 80
#define MINX 0
#define MINY 0
#define UP 65 
#define DOWN 66
#define ALIEN_SIZE 3
#define PLAYER_SIZE 5
#define SHOTS_ALLOWED 2
#define ALIENS_ALLOWED 2

typedef enum {DP, ORFIST, GRAVIDIA, METEOR, CRITICAL,BLANK} EntityType;
typedef enum {N,S,NEAST,SEAST,NWEST,SWEST,FIXED,HALT} Directions;

typedef struct positions{
    int x;
    int y;
}Position;

typedef struct entities{
    Position p;
    EntityType e;
    Directions d;
    int lives;
    int color;
    bool isTracked;
    pid_t pid;
}Entity;

typedef struct mapcell{
    pid_t pid;
    EntityType e;
    Directions d;
    Position p;
}mapCell;


// entities
void player(int leTube, int playerpipe, int missilepipe);

void missile(int leTube, int start_y, int start_x, int missilepipe);

void alien(int leTube, int alienpipe);

void alienGenerator(int leTube, int alienpipe);

void printer(Entity ent);

void moveInCollisionMap(int y, int x, mapCell map[y][x], Entity tempEntity);

void deleteInCollisionMap(int y, int x, mapCell map[y][x], Entity tempEntity);

bool verifyCollision(Entity tempEntity, int x, int y, mapCell map[y][x], int playerpipe,int alienpipe,int missilepipe);

void greatFilter(int leTube,int playerpipe, int missilepipe, int alienpipe);


#endif