#include "space.h"

Gamestate game;

void ship(int pipewrite)
{
    char input;
    Entity player;

    player.hb.topLeft.x = 1;
    player.hb.topLeft.y = (MAXY/2)-2;

    player.hb.botRight.x = player.hb.topLeft.x + PLAYER_SIZE-1;
    player.hb.botRight.y = player.hb.topLeft.y + PLAYER_SIZE-1;

    player.lives = 1;
    player.d = FIXED;
    player.e = DP;
    player.pid = getpid();
    player.col = RED;

    while(true) {
        input=getch();
        switch(input) {
            case UP:
                if(player.hb.topLeft.y >= 2) {
                    player.hb.topLeft.y--;
                    player.d = N;
                }

                player.hb.botRight.x = player.hb.topLeft.x + PLAYER_SIZE-1;
                player.hb.botRight.y = player.hb.topLeft.y + PLAYER_SIZE-1;
            break;

            case DOWN:

                if(player.hb.botRight.y <= MAXY-2) {
                    player.hb.topLeft.y++;
                    player.d = S;
                }
                player.hb.botRight.x = player.hb.topLeft.x + PLAYER_SIZE-1;
                player.hb.botRight.y = player.hb.topLeft.y + PLAYER_SIZE-1;
            break;
            
            case 's':
                shoot(pipewrite,player.hb);
            break;
        }

        updateEntity(player,pipewrite);
    }
}

void shoot(int pipewrite, Hitbox pH){
    Entity missiles[SHOTS_ALLOWED];
 
    missiles[0].hb.topLeft.y = missiles[0].hb.botRight.y =  pH.topLeft.y+1;
    missiles[0].d = NEAST;
    missiles[0].col = BLUE;
    
    missiles[1].hb.topLeft.y = missiles[1].hb.botRight.y =  pH.botRight.y-1;
    missiles[1].d = SEAST;
    missiles[1].col = CYAN;

    for(int i = 0; i < SHOTS_ALLOWED; i++){
        missiles[i].hb.topLeft.x = missiles[i].hb.botRight.x = pH.topLeft.x + PLAYER_SIZE;
        missiles[i].lives = 1;
        missiles[i].e = CRITICAL;
        missiles[i].id = i;
        if(fork() == 0){
            moveProjectile(pipewrite, missiles[i]);
        }
    }
}

void moveProjectile(int pipewrite, Entity projectile){
    
    int steps = 0;
    projectile.pid = getpid();
    while (true) { // Main bullet loop
        if(projectile.e == CRITICAL) {
            switch(projectile.d) {
                case NEAST:
                    steps++;
                    if(projectile.hb.topLeft.x%4 == 0){
                        projectile.hb.topLeft.y--;
                    }
                    if(projectile.hb.topLeft.y < 1) {
                        projectile.d = SEAST;
                        projectile.hb.topLeft.y++;
                    }
                break;

                case SEAST:
                    steps++;
                    if(projectile.hb.topLeft.x%4 == 0){
                        projectile.hb.topLeft.y++;
                    }
                    if(projectile.hb.topLeft.y > MAXY-1) {
                        projectile.d = NEAST;
                        projectile.hb.topLeft.y--;
                    }
                break;
            }

            projectile.hb.topLeft.x += 1;

            if (projectile.hb.topLeft.x >= MAXX-1) {
                projectile.lives = 0;
                updateEntity(projectile,pipewrite);
                kill(projectile.pid, 1);
                return;
            }

            updateEntity(projectile,pipewrite);

            usleep(25000);

        } else {
            projectile.hb.topLeft.x = projectile.hb.botRight.x -=1;

            if (projectile.hb.topLeft.x <= 1) {
                projectile.lives = 0;
                updateEntity(projectile,pipewrite);
                kill(projectile.pid, 1);
                return;
            }

            updateEntity(projectile,pipewrite);

            usleep(25000);
        }    
    }
}

void attack(int pipewrite, int attackChance, Hitbox aH)
{
    pid_t bombaPid;

    if (rand()%attackChance == 0){
        Entity bomba;

        bomba.hb.topLeft.x = bomba.hb.botRight.x = aH.topLeft.x-1;
        bomba.hb.topLeft.y = bomba.hb.botRight.y = aH.topLeft.y+1;
        bomba.pid = getpid();
        bomba.lives = 1;
        bomba.e = METEOR;
        bomba.id = 1945;
        bomba.d = FIXED;
        bomba.col = WHITE;

        if ((bombaPid = fork()) == 0) {
            moveProjectile(pipewrite,bomba);
        }
    }
}

void alienGenerator(int pipewrite){
    int prevmod,currentmod,lastOccupPosition,i;
    Entity aliensLVL1[ALIENS_ALLOWED*WAVES];
    prevmod = currentmod = 0;

    for (int i=0; i<(ALIENS_ALLOWED * WAVES); i+=1) {
        currentmod = rand()%3;
        if(prevmod == currentmod) {
            if(currentmod - 1 < 0) {
                currentmod += 1;
            } else {
                currentmod -=1;
            }
        }

        aliensLVL1[i].lives = 1;
        aliensLVL1[i].e = ORFIST;
        aliensLVL1[i].id = i;

        switch(currentmod) {
            case 0:
                aliensLVL1[i].d = NWEST;
                aliensLVL1[i].hb.topLeft.y = 25 + rand()%9;
                aliensLVL1[i].hb.topLeft.x = MAXX - ALIEN_SIZE;
                aliensLVL1[i].col = YELLOW;
            break;

            case 1:
                aliensLVL1[i].d = SWEST;
                aliensLVL1[i].hb.topLeft.y = 1 + rand()%9;
                aliensLVL1[i].hb.topLeft.x = MAXX - ALIEN_SIZE;
                aliensLVL1[i].col = MAGENTA;
            break;

            case 2:
                aliensLVL1[i].d = FIXED;
                aliensLVL1[i].hb.topLeft.y = MAXY/2;
                aliensLVL1[i].hb.topLeft.x = MAXX - ALIEN_SIZE;
                aliensLVL1[i].col = GREEN;
            break;
        }

        aliensLVL1[i].hb.botRight.x = aliensLVL1[i].hb.topLeft.x + (ALIEN_SIZE-1);
        aliensLVL1[i].hb.botRight.y = aliensLVL1[i].hb.topLeft.y + (ALIEN_SIZE-1);

        prevmod = currentmod;
    }

    lastOccupPosition = 0;

    while(lastOccupPosition < WAVES*ALIENS_ALLOWED){
        
        for(i = lastOccupPosition;i < ALIENS_ALLOWED+lastOccupPosition; i++){
            if(fork()== 0){
                alien(pipewrite,aliensLVL1[i]);
            }
            sleep(3);
        }
        lastOccupPosition = i;  
        sleep(10);
    }

    exit(0);
}

void alien(int pipewrite, Entity alien)
{
    srand(time(NULL) * alien.id);
    alien.pid = getpid();
    int midborder = MAXY/2;

    while (true){
        switch(alien.d){
            case NWEST:
                alien.hb.topLeft.x--;
                if(alien.hb.topLeft.y <= 1 || alien.hb.topLeft.y == midborder-1){
                    alien.d = SWEST;
                    alien.hb.topLeft.y++;
                } else {
                    alien.hb.topLeft.y--;
                }
                alien.hb.botRight.x = alien.hb.topLeft.x + ALIEN_SIZE-1;
                alien.hb.botRight.y = alien.hb.topLeft.y + ALIEN_SIZE-1;
            break;

            case SWEST:
                alien.hb.topLeft.x--;
                if(alien.hb.botRight.y >= MAXY-1 || alien.hb.botRight.y == midborder+1) {
                    alien.d = NWEST;
                    alien.hb.topLeft.y--;
                } else {
                    alien.hb.topLeft.y++;
                }
                alien.hb.botRight.x = alien.hb.topLeft.x + ALIEN_SIZE-1;
                alien.hb.botRight.y = alien.hb.topLeft.y + ALIEN_SIZE-1;
            break;

            case FIXED:
                alien.hb.topLeft.x--;
                alien.hb.botRight.x = alien.hb.topLeft.x + ALIEN_SIZE-1;
            break;
        }
        attack(pipewrite, ATK_CHANCE, alien.hb);
        updateEntity(alien,pipewrite); 
        usleep(600000); 
    }
}

void alien2(int pipewrite, Entity alien)
{
    srand(time(NULL) * alien.id);
    alien.pid = getpid();
    int midborder = MAXY/2-2;

    while (true){ 
        switch(alien.d) {
            case NWEST:
                alien.hb.topLeft.x--;
                if(alien.hb.topLeft.y <= 1 || alien.hb.topLeft.y == midborder+2) {
                    alien.d = SWEST;
                    alien.hb.topLeft.y++;
                } else {
                    alien.hb.topLeft.y--;
                }
                alien.hb.botRight.y = alien.hb.topLeft.y + (ALIEN_SIZE*2)-1;
                alien.hb.botRight.x = alien.hb.topLeft.x + (ALIEN_SIZE*2)+1;
            break;

            case SWEST:
                alien.hb.topLeft.x--;
                if(alien.hb.botRight.y >= MAXY-1 || alien.hb.topLeft.y == midborder-2) {
                    alien.d = NWEST;
                    alien.hb.topLeft.y--;
                } else {
                    alien.hb.topLeft.y++;
                }
                alien.hb.botRight.y = alien.hb.topLeft.y + (ALIEN_SIZE*2)-1;
                alien.hb.botRight.x = alien.hb.topLeft.x + (ALIEN_SIZE*2)+1;
            break;

            case FIXED:
                alien.hb.topLeft.x--;
                alien.hb.botRight.x = alien.hb.topLeft.x + (ALIEN_SIZE*2)+1;
            break;
        }
        attack(pipewrite, ATK_CHANCE, alien.hb);
        updateEntity(alien,pipewrite);             
        usleep(600000); 
    }
}

void space(int piperead, int pipewrite)
{
    initializeData();
    Entity tempEntity, tempMissile, tempBomba;

    drawFieldBorder();
    
    while(game.running)
    {   
        drawFieldBorder();
        read(piperead, &tempEntity, sizeof(Entity));

        if(tempEntity.e == GRAVIDIA || tempEntity.e == ORFIST || tempEntity.e == METEOR){
            mvprintw(0,MAXX+1,"[x:%d|y:%d] [pid:%d|type:%d|vector id:%d]\t",tempEntity.hb.topLeft.x,tempEntity.hb.topLeft.y, tempEntity.pid,tempEntity.e,tempEntity.id);
            mvprintw(1,MAXX+1,"[direction:%d|color:%d|lives:%d]",tempEntity.d,tempEntity.col,tempEntity.lives);
        }
        
        switch (tempEntity.e)
        {
            case DP:
                bodyClearing(game.player);
                game.player.hb.topLeft.x = tempEntity.hb.topLeft.x;
                game.player.hb.topLeft.y = tempEntity.hb.topLeft.y;
                game.player.hb.botRight.x = tempEntity.hb.botRight.x;
                game.player.hb.botRight.y = tempEntity.hb.botRight.y;
                game.player.pid = tempEntity.pid;
                if(game.player.lives > 0){
                    printer(tempEntity);
                }
            break;

            case ORFIST:
                bodyClearing(game.aliensLVL1[tempEntity.id]);
                game.aliensLVL1[tempEntity.id].hb.topLeft.x = tempEntity.hb.topLeft.x;
                game.aliensLVL1[tempEntity.id].hb.topLeft.y = tempEntity.hb.topLeft.y;
                game.aliensLVL1[tempEntity.id].hb.botRight.x = tempEntity.hb.botRight.x;
                game.aliensLVL1[tempEntity.id].hb.botRight.y = tempEntity.hb.botRight.y;
                game.aliensLVL1[tempEntity.id].d = tempEntity.d;
                game.aliensLVL1[tempEntity.id].col = tempEntity.col;
                game.aliensLVL1[tempEntity.id].pid = tempEntity.pid;                              
                shipCollisions(game.aliensLVL1[tempEntity.id]);
                checkBorderProximity(game.aliensLVL1[tempEntity.id].hb.topLeft.x);               
                if(game.aliensLVL1[tempEntity.id].lives > 0){
                    printer(game.aliensLVL1[tempEntity.id]);
                }
            break;
            
            case GRAVIDIA:
                bodyClearing(game.aliensLVL2[tempEntity.id]); 
                game.aliensLVL2[tempEntity.id].hb.topLeft.x = tempEntity.hb.topLeft.x;
                game.aliensLVL2[tempEntity.id].hb.topLeft.y = tempEntity.hb.topLeft.y;
                game.aliensLVL2[tempEntity.id].hb.botRight.x = tempEntity.hb.botRight.x;
                game.aliensLVL2[tempEntity.id].hb.botRight.y = tempEntity.hb.botRight.y;
                game.aliensLVL2[tempEntity.id].d = tempEntity.d;
                game.aliensLVL2[tempEntity.id].col = tempEntity.col;
                game.aliensLVL2[tempEntity.id].pid = tempEntity.pid;                
                shipCollisions(game.aliensLVL2[tempEntity.id]);
                checkBorderProximity(game.aliensLVL2[tempEntity.id].hb.topLeft.x);
                if(game.aliensLVL2[tempEntity.id].lives > 0){
                    printer(game.aliensLVL2[tempEntity.id]);
                }
            break;
        
            case METEOR:
                bodyClearing(tempEntity);
                tempBomba.hb.topLeft.x = tempBomba.hb.botRight.x = tempEntity.hb.topLeft.x;
                tempBomba.hb.topLeft.y = tempBomba.hb.botRight.y = tempEntity.hb.botRight.y;
                tempBomba.pid = tempEntity.pid;
                tempBomba.d = tempEntity.d;
                tempBomba.id = tempEntity.id;
                tempBomba.e = tempEntity.e;                            
                shipCollisions(tempBomba);
                if(tempBomba.hb.topLeft.x > 1){
                    printer(tempBomba);
                }
                                
            break;

            case CRITICAL:
                bodyClearing(tempEntity);
                tempMissile.hb.topLeft.x = tempMissile.hb.botRight.x = tempEntity.hb.topLeft.x;
                tempMissile.hb.topLeft.y = tempMissile.hb.botRight.y = tempEntity.hb.botRight.y;
                tempMissile.pid = tempEntity.pid;
                tempMissile.col = tempEntity.col;
                tempMissile.d = tempEntity.d;
                tempMissile.id = tempEntity.id;
                tempMissile.e = tempEntity.e;
                projectileCollisions(tempMissile,pipewrite);
                if(tempMissile.lives > 0 && tempMissile.hb.topLeft.x < MAXX-1 && tempMissile.hb.topLeft.y > 1 && tempMissile.hb.topLeft.y < MAXY-1){
                    printer(tempMissile);
                }               
            break;
        }
        
        refresh();

        if ((game.aliensLvl1Alive+game.aliensLvl2Alive) <= 0 && game.running && game.player.lives > 0)
        {
            game.running = false;
            mvprintw(MAXY/2,MAXX/2-15, "You win! Press any key to exit...");
            refresh();
            getchar();
        }  
    }
}

void projectileCollisions(Entity currentMissile, int pipewrite)
{
    for (int i = 0; i < ALIENS_ALLOWED*WAVES ; i++) {
        if (game.aliensLVL1[i].lives > 0) {
            if (verifyHitbox(game.aliensLVL1[i].hb,currentMissile.hb)) {

                pid_t alien2p;

                game.aliensLVL2[i].col = game.aliensLVL1[i].col;
                game.aliensLVL2[i].d = game.aliensLVL1[i].d;
                game.aliensLVL2[i].hb.topLeft.x = game.aliensLVL1[i].hb.topLeft.x-2;
                game.aliensLVL2[i].hb.topLeft.y = game.aliensLVL1[i].hb.topLeft.y-1;
                game.aliensLVL2[i].hb.botRight.x = game.aliensLVL1[i].hb.botRight.x+2;
                game.aliensLVL2[i].hb.botRight.y = game.aliensLVL1[i].hb.botRight.y+2;
                game.aliensLvl2Alive++;

                game.aliensLvl1Alive--;
                game.aliensLVL1[i].lives = 0;
                bodyClearing(currentMissile);
                bodyClearing(game.aliensLVL1[i]);
                mvprintw(MAXY+1,MAXX/2,"orfist with [pid:%d|vector id:%d] died",game.aliensLVL1[i].pid,game.aliensLVL1[i].id);
                kill(currentMissile.pid, 1);
                kill(game.aliensLVL1[i].pid, 1);

                if((alien2p = fork()) == 0){
                    alien2(pipewrite, game.aliensLVL2[i]);
                }
                return;
            }
        }
    }

    for (int i = 0; i < ALIENS_ALLOWED*WAVES; i++) {
        if (game.aliensLVL2[i].lives > 0) {
            if (verifyHitbox(game.aliensLVL2[i].hb, currentMissile.hb)) {

                game.aliensLVL2[i].lives-=1;
                bodyClearing(currentMissile);
                kill(currentMissile.pid, 1);
                
                if (game.aliensLVL2[i].lives <= 0) {
                    game.aliensLvl2Alive--;
                    bodyClearing(game.aliensLVL2[i]);
                    mvprintw(MAXY+2,MAXX/2,"gravidia with [pid:%d|vector id:%d] died",game.aliensLVL2[i].pid,game.aliensLVL2[i].id);                     
                    kill(game.aliensLVL2[i].pid, 1);                    
                }
                return;               
            }
        }
    }
}

void shipCollisions(Entity temp)
{

    if (verifyHitbox(game.player.hb, temp.hb)){
        game.player.lives = 0;
        bodyClearing(game.player);
        bodyClearing(temp);        
        kill(game.player.pid, 1);
        kill(temp.pid, 1);      
        game.running = false;
        mvprintw(MAXY/2,MAXX/2-15, "You lose, now Earth is doomed... \n\t\t\t\t\t Press any key to exit...");
        refresh();
        getchar();
    }
}

void checkBorderProximity(int topLeftx)
{
    if (topLeftx <= 1) { //Ufo has reached the end of the area, the player has lost
        game.running = false;
        mvprintw(MAXY/2,MAXX/2-15, "You lose, invaders reached Earth... \n\t\t\t\t\t Press any key to exit...");
        refresh();
        getchar();
    }
}

_Bool verifyHitbox(Hitbox a, Hitbox b)
{

    if(((a.topLeft.x >= b.topLeft.x && a.topLeft.x <= b.botRight.x) || (a.botRight.x >= b.topLeft.x && a.botRight.x <= b.botRight.x)) &&
       ((a.topLeft.y >= b.topLeft.y && a.topLeft.y <= b.botRight.y) || (a.botRight.y >= b.topLeft.y && a.botRight.y <= b.botRight.y))) {
        return true;
    } else {
        return false;
    }
}

// data manipulation

void updateEntity(Entity temp, int pipewrite)
{
    Entity current;

    if(temp.e == ORFIST || temp.e == GRAVIDIA || temp.e == DP) {
        current.hb.topLeft.x = temp.hb.topLeft.x;
        current.hb.topLeft.y = temp.hb.topLeft.y;
        current.hb.botRight.x = temp.hb.botRight.x;
        current.hb.botRight.y = temp.hb.botRight.y;
    } else {
        current.hb.topLeft.x = current.hb.botRight.x = temp.hb.topLeft.x;
        current.hb.topLeft.y = current.hb.botRight.y = temp.hb.topLeft.y;
    }

    current.lives = temp.lives;
    current.e = temp.e;
    current.col = temp.col;
    current.d = temp.d;
    current.pid = temp.pid;
    current.id = temp.id;

    write(pipewrite, &current, sizeof(Entity));
}

void initializeData()
{
    init_pair(WHITE,COLOR_WHITE,COLOR_BLACK);
    init_pair(RED,COLOR_RED,COLOR_BLACK);
    init_pair(YELLOW,COLOR_YELLOW,COLOR_BLACK);
    init_pair(GREEN,COLOR_GREEN,COLOR_BLACK);
    init_pair(BLUE,COLOR_BLUE,COLOR_BLACK);
    init_pair(MAGENTA,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(CYAN,COLOR_CYAN,COLOR_BLACK);

    game.player.lives = 1;
    game.player.id = 42;
    game.aliensLvl1Alive = ALIENS_ALLOWED*WAVES;
    game.aliensLvl2Alive = 0;
    game.running = true;

    for(int i = 0; i < ALIENS_ALLOWED*WAVES; i++){
        game.aliensLVL1[i].lives = 1;
        game.aliensLVL1[i].id = i;
        game.aliensLVL1[i].e = ORFIST;
        game.aliensLVL2[i].lives = 4;
        game.aliensLVL2[i].id = i;
        game.aliensLVL2[i].e = GRAVIDIA;
    }
}
    
// printing

void drawFieldBorder()
{
    int i = 0;
    int j = 0;
    for (j; j <= MAXY; j++) {
        i = 0;
        if (j >= 1 && j < MAXY) {
            mvaddch(j, i, ACS_VLINE);
            mvaddch(j, MAXX, ACS_VLINE);

        } else if (j == 0 || j == MAXY) {
            for (i = 0; i < MAXX; i++) {
                mvaddch(j, i, ACS_HLINE);
                mvaddch(j, MAXX, ACS_HLINE);
            }
        }
    }
    mvaddch(0, 0, ACS_ULCORNER);
    mvaddch(0, MAXX, ACS_URCORNER);
    mvaddch(MAXY, MAXX, ACS_LRCORNER);
    mvaddch(MAXY, 0, ACS_LLCORNER);
}

void printer(Entity ent)
{
    // matrice giocatore...
    char DPBody [5][5] = {
        "c===>",
        " ]-| ",
        "<(D)>",
        " ]-| ",
        "c===>"
    };

    // matrice alieno lvl 1
    char ORFISTBody [3][3] = {
        "(*)",
        "]M[",
        "^v^"
    };

    // matrice alieno lvl 2
    char GRAVIDIABody [2][3] = {
        "<A>",
        "(T)"
    };

    // asterisco per i missili,zero per la bomba
    char CRITICALBody = 'O';
    char METEORBody = '0';

    
    // individuazione entità...
    switch(ent.e) {
    // giocatore
    case DP:
        attron(COLOR_PAIR(ent.col));
        mvprintw(2,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
        for(int i = 0; i<PLAYER_SIZE; i++) {
            for(int j = 0; j<PLAYER_SIZE; j++) {
                mvaddch(ent.hb.topLeft.y+i,ent.hb.topLeft.x+j, DPBody[i][j]);
            }
        }
        attroff(COLOR_PAIR(ent.col));
        break;

    // alieno lvl 1
    case ORFIST:
        attron(COLOR_PAIR(ent.col));
        switch(ent.col){
            case YELLOW:
                mvprintw(3,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;

            case GREEN:
                mvprintw(4,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;

            case MAGENTA:
                mvprintw(5,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;
        }       
        for(int i = 0; i<ALIEN_SIZE; i++) {
            for(int j = 0; j <ALIEN_SIZE; j++) {
                mvaddch(ent.hb.topLeft.y+i,ent.hb.topLeft.x+j, ORFISTBody[i][j]);
            }
        }
        attroff(COLOR_PAIR(ent.col));
        break;

    // alieno lvl2
    case GRAVIDIA:
        attron(COLOR_PAIR(ent.col));
        int x,y;

        switch(ent.col){
            case YELLOW:
                mvprintw(6,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;

            case GREEN:
                mvprintw(7,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;

            case MAGENTA:
            mvprintw(8,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            break;
        }

        for (int i = 0; i < ent.lives; ++i) {
            switch(i) {
            case 0:
                y = ent.hb.topLeft.y;
                x = ent.hb.topLeft.x;
                break;

            case 1:
                x += 4;
                break;

            case 2:
                y += 3;
                x -= 4;
                break;

            case 3:
                x += 4;
                break;
            }

            for (int j = 0; j < ALIEN_SIZE-1; j++) {
                for (int k = 0; k < ALIEN_SIZE; k++) {
                    mvaddch(y+j,x+k,GRAVIDIABody[j][k]);
                }
            }
        }

        attroff(COLOR_PAIR(ent.col));
        break;

    // bomba
    case METEOR:
        mvprintw(9,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
        mvaddch(ent.hb.topLeft.y,ent.hb.topLeft.x,METEORBody);
        break;

    // missili
    case CRITICAL:
        if(ent.hb.topLeft.x < MAXX && ent.hb.topLeft.y > 0 && ent.hb.topLeft.y <MAXY) {
            attron(COLOR_PAIR(ent.col));
            if(ent.col == CYAN){
                mvprintw(10,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            }else{
                mvprintw(11,MAXX+1,"printing entity [x:%d y:%d] [type:%d|pid:%d|lives:%d|vector id:%d]",ent.hb.topLeft.x, ent.hb.topLeft.y,ent.e,ent.pid,ent.lives,ent.id);
            }            
            mvaddch(ent.hb.topLeft.y,ent.hb.topLeft.x,CRITICALBody);
            attroff(COLOR_PAIR(ent.col));
        }
        break;
    }
}

void bodyClearing(Entity ent){
    int size,x,y,adjy,adjx;

    y = ent.hb.topLeft.y;
    x = ent.hb.topLeft.x;
    adjy = adjx = 0;

    switch(ent.e) {
        case DP:            
            size = PLAYER_SIZE;
        break;

        case ORFIST:
            size = ALIEN_SIZE;
        break;

        case GRAVIDIA:
            size = ALIEN_SIZE*2+1;
        break;

        case CRITICAL:
            size = 1;
            adjx -=1;
            if(ent.d == NEAST){
                if((x-1)%4 == 0){
                   adjy +=1; 
                }       
            } else {
                if((x-1)%4 == 0){
                   adjy -=1; 
                }
            }
            
        break;

        case METEOR:
            size = 1;
            adjx +=1;
        break;
    }

    for(int i = 0; i < size; i++){
        for(int j = 0 ; j < size ; j++){
            mvaddch(y+i+adjy,x+j+adjx,' ');
        }
    }
}

/*

    void shoot(int pipewrite, Hitbox pH)
    {
        pid_t missilePid;
        Entity missile;

        missile.hb.topLeft.x = missile.hb.botRight.x = pH.topLeft.x + PLAYER_SIZE;
        missile.pid = getpid();
        missile.lives = 1;
        missile.e = CRITICAL;

        missile.hb.topLeft.y = missile.hb.botRight.y =  pH.topLeft.y+1;
        missile.d = NEAST;
        missile.col = BLUE;

        missilePid = fork();
        if (missilePid == 0) {
            moveProjectile(pipewrite, missile);
        }

        missile.hb.topLeft.y = missile.hb.botRight.y =  pH.botRight.y-1;
        missile.d = SEAST;
        missile.col = CYAN;

        missilePid = fork();
        if (missilePid == 0) {
            moveProjectile(pipewrite, missile);
        }
    }

    void bodyClearing(Entity ent)
    {

        int size,x,y,adjx,adjy;

        adjx = adjy = 0;

        y = ent.hb.topLeft.y;
        x = ent.hb.topLeft.x;

        switch(ent.e) {
        case DP:
            size = PLAYER_SIZE;
            if(ent.d == N) {
                adjy +=1;
            } else {
                adjy -=1;
            }
            break;

        case ORFIST:
            size = ALIEN_SIZE;
            if(ent.d == NWEST) {
                adjy +=1;
            } else {
                adjy -=1;
            }
            adjx +=1;
            break;

        case GRAVIDIA:
            size = ALIEN_SIZE*2;
            if(ent.d == NWEST) {
                adjy +=1;
            } else {
                adjy -=1;
            }
            adjx +=1;
            break;

        case CRITICAL:
            size = 1;
            if(ent.d == NEAST) {
                adjy +=1;
            } else {
                adjy -=1;
            }
            adjx -=3;
            break;

        case METEOR:
            size = 1;
            adjx +=1;
            break;
        }

        for(int i = 0; i < size; i++) {
            for(int j = 0 ; j < size ; j++) {
                mvaddch(y+i+adjy,x+j+adjx,' ');
            }
        }
    }

    void alienGenerator(int pipewrite)
    {
        for(int i = game.lastOccupPosition;i < ALIENS_ALLOWED+game.lastOccupPosition; i++){
            if(fork()==0){
                alien(pipewrite,game.aliensLVL1[i]);
            }
        }    

        game.lastOccupPosition += ALIENS_ALLOWED;
    }
*/
