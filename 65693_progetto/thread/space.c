#include "space.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

_Bool gamestate;
int score;
entityList* EL;
int wavecount = 0,publ_pl_lives;

// matrice giocatore
char DPBody [5][5] = 
{
    "c===>",
    " ]-| ",
    "<(D)>",
    " ]-| ",
    "c===>"
};

// matrice alieno lvl 1
char ORFISTBody [3][3] = 
{
    "(*)",
    "]|[",
    "^v^"
};

// matrice alieno lvl 2
char GRAVIDIABody [2][3] = 
{
    "<A>",
    "(T)"
};

// asterisco per i missili,zero per la bomba
char CRITICALBody = 'O';
char METEORBody = '0';

// funzioni per le chiamate da thread

void* ship(void* param){

    entityList *eL = (entityList*) param;
    
    Entity s;

    s.hb.topLeft.x = 1;
    s.hb.topLeft.y = (MAXY/2)-2;

    s.hb.botRight.x = s.hb.topLeft.x+(PLAYER_SIZE-1);
    s.hb.botRight.y = s.hb.topLeft.y+(PLAYER_SIZE-1);

    s.lives = 1;
    publ_pl_lives = s.lives;
    s.d = FIXED;
    s.e = DP;
    s.color = 1;
    s.thr = pthread_self();
    s.destroyable = false;

    entityNode *shipNode = insert(s,eL);

    char c;

    Hitbox *shipBox = &shipNode->data.hb;
    Entity *ship = &shipNode->data;

    int shots = MAX_SHOTS;

    // inizio loop attività...
    while(ship->lives > 0){ 

        c=(char)getchar();

        // ricezione input direzione
        switch(c){
            
            case UP:
                pthread_mutex_lock(&mutex);
                if(shipBox->topLeft.y > 1){
                    shipBox->topLeft.y -= 1;
                    shipBox->botRight.y = shipBox->topLeft.y+(PLAYER_SIZE-1);
                    ship->d = N;              
                }
                if(shots < MAX_SHOTS){;
                    shots++;  
                }
                pthread_mutex_unlock(&mutex); 
            break;

            case DOWN:
                pthread_mutex_lock(&mutex);
                if(shipBox->botRight.y < MAXY-1){       
                    shipBox->topLeft.y += 1;
                    shipBox->botRight.y = shipBox->topLeft.y+(PLAYER_SIZE-1);
                    ship->d = S;  
                }
                if(shots < MAX_SHOTS){;
                    shots++;  
                }    
                pthread_mutex_unlock(&mutex); 
            break;

            // bottone di sparo, generazione missili
            case 's':
                if(shots > 0){
                    blast(eL,ship);
                    shots--;  
                }       
            break;               
        }

        mvprintw(MAXY+2,0,"shots available:%d",shots);
    }

    gamestate = false;

    while(!ship->destroyable){
        usleep(10000);
    }

    return NULL;
}

void* missile(void* param){

    entityNode* m = (entityNode*) param;

    Hitbox *missileBox = &m->data.hb;
    Entity *missile = &m->data;

    missile->thr = pthread_self();

    // inizio attività, condizioni: se il missile 
    // è in volo e non ha superato il bordo destro
    while(missileBox->topLeft.x < MAXX-1 && missile->lives > 0 && gamestate){
   
        switch(missile->d){
        
            case NEAST:
                pthread_mutex_lock(&mutex);
                if(missileBox->topLeft.y < 0){
                    missile->d = SEAST;
                    missileBox->topLeft.y = missileBox->botRight.y += 1;
                }else {
                    missileBox->topLeft.y = missileBox->botRight.y -= 1;
                }

                missileBox->topLeft.x += 3;
                missileBox->botRight.x = missileBox->topLeft.x;
                pthread_mutex_unlock(&mutex);
            break;

            case SEAST:
                pthread_mutex_lock(&mutex);
                if(missileBox->topLeft.y > MAXY){
                   missile->d = NEAST;
                   missileBox->topLeft.y = missileBox->botRight.y -= 1;
                }else{
                    missileBox->topLeft.y = missileBox->botRight.y  += 1;
                }
                missileBox->topLeft.x += 3;
                missileBox->botRight.x = missileBox->topLeft.x;
                pthread_mutex_unlock(&mutex);
            break;
        }
        usleep(50000);
    }

    while(!missile->destroyable && gamestate){
        usleep(10000);
    }
    return NULL;
}

void* alienGenerator(void* param){

    int prevmod,currentmod;

    while(!gamestate){
        usleep(20000);
    }

    entityList *list = (entityList*) param;

    // loop che genera le posizioni iniziali degli alieni e avvia un thread per alieno
    do{
        currentmod = prevmod = 0;

        for(int i = 0; i < ALIENS_ALLOWED; i++){

            Entity a;
            pthread_t alienT;

            a.lives = 1;
            a.e = ORFIST;
            a.destroyable = false;


            // condizione per impedire che vengano generati
            // 3 alieni con la stessa direzione di fila
            currentmod = rand()%3;

            if(prevmod == currentmod){
                if(currentmod - 1 < 0){
                    currentmod += 1;
                }else{
                    currentmod -=1;
                }
            }

            prevmod = currentmod;

            switch(currentmod){
                case 0:
                    a.d = NWEST; 
                    a.hb.topLeft.y = MAXY-2;
                    a.hb.topLeft.x = MAXX - ALIEN_SIZE;
                    a.color = 2;
                break;
                 
                case 1:
                    a.d = SWEST;
                    a.hb.topLeft.y = 0;
                    a.hb.topLeft.x = MAXX - ALIEN_SIZE; 
                    a.color = 5;
                break;
                 
                case 2:
                    a.d = FIXED;
                    a.hb.topLeft.y = (MAXY/2);
                    a.hb.topLeft.x = MAXX - ALIEN_SIZE;
                    a.color = 3;
                break;
            }

            a.hb.botRight.x = a.hb.topLeft.x + (ALIEN_SIZE-1);
            a.hb.botRight.y = a.hb.topLeft.y + (ALIEN_SIZE-1);

            entityNode *alienN = insert(a,list);

            pthread_create(&alienN->data.thr,NULL,&alien,(void*)alienN);
        }

        wavecount +=1;
        sleep(SPAWN_SPEED);
    }while(WAVES > wavecount);


    //una volta finita la generazione degli alieni 
    //aspetta la cancellazione insieme agli altri thread
    while(gamestate){
       usleep(20000); 
    }

    return NULL;
}

void* alien(void* param){
    
    entityNode *a = (entityNode*) param;

    Hitbox *alienBox = &a->data.hb;
    Entity *alien = &a->data;

    alien->thr = pthread_self();

    // loop che termina solo se l'alieno è morto o ha raggiunto 
    // il bordo sinistro della mappa o è terminato il gioco
    while(alienBox->topLeft.x >= 0 && alien->lives > 0 && gamestate){

        switch(alien->d){
            case NWEST:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.y -= 1;
                alienBox->topLeft.x -= 1;
                
                if(alienBox->topLeft.y <= 1){
                    alien->d = SWEST;
                    alienBox->topLeft.y += 1;
                }

                alienBox->botRight.y = alienBox->topLeft.y + (ALIEN_SIZE-1);
                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE-1);    

                pthread_mutex_unlock(&mutex);
            break;

            case SWEST:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.y += 1;
                alienBox->topLeft.x -= 1;
                
                if(alienBox->botRight.y >= MAXY-1){
                   alien->d = NWEST;
                   alienBox->topLeft.y -= 1;
                }

                alienBox->botRight.y = alienBox->topLeft.y + (ALIEN_SIZE-1);
                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE-1);  

                pthread_mutex_unlock(&mutex);
            break;

            case FIXED:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.x -= 1;

                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE-1);  

                pthread_mutex_unlock(&mutex);
            break;
        } 

        bombaDeployer(EL,alien,ATK_CHANCE);

        if(alienBox->topLeft.x <= 0){
            gamestate = false;
            break;
        }

        usleep(MOVESPEED_LVL1);
    }

    // aspetta che le sue caratteristiche vengano aggiornate 
    // per l'evoluzione dalla funzione space 
    while(alien->lives <= 0){
        usleep(10000);
    }

    // stesse ccondizioni del loop precedente con l'aggiunta 
    // della condizione di essere un alieno di livello 2
    while(alienBox->topLeft.x >= 0 && alien->lives > 0 && alien->e == GRAVIDIA && gamestate){

        switch(alien->d){
            case NWEST:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.y -= 1;
                alienBox->topLeft.x -= 1;
                
                if(alienBox->topLeft.y <= 1){
                    alien->d = SWEST;
                    alienBox->topLeft.y += 1;
                }

                alienBox->botRight.y = alienBox->topLeft.y + (ALIEN_SIZE*2)-1;
                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE*2)+1;    

                pthread_mutex_unlock(&mutex);
            break;

            case SWEST:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.y += 1;
                alienBox->topLeft.x -= 1;
                
                if(alienBox->botRight.y >= MAXY-1){
                   alien->d = NWEST;
                   alienBox->topLeft.y -= 1;
                }

                alienBox->botRight.y = alienBox->topLeft.y + (ALIEN_SIZE*2)-1;
                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE*2)+1;  

                pthread_mutex_unlock(&mutex);
            break;

            case FIXED:
                pthread_mutex_lock(&mutex);
                alienBox->topLeft.x -= 1;
                alienBox->botRight.x = alienBox->topLeft.x + (ALIEN_SIZE*2)+1;  
                pthread_mutex_unlock(&mutex);
            break;
        } 

        bombaDeployer(EL,alien,ATK_CHANCE);

        if(alienBox->topLeft.x <= 0){
            gamestate = false;
            break;
        }

        usleep(MOVESPEED_LVL2);
    }


    
    while(!alien->destroyable && gamestate){
        usleep(10000);
    }    
}

void* bomba(void* param){
    entityNode* b = (entityNode*) param;

    Hitbox *bombaBox = &b->data.hb;
    Entity *bomba = &b->data;

    bomba->thr = pthread_self();

    // loop di movimento
    while(bombaBox->topLeft.x > 0 && bomba->lives > 0 && gamestate){
        pthread_mutex_lock(&mutex);
        bombaBox->topLeft.x = bombaBox->botRight.x -=1;
        pthread_mutex_unlock(&mutex);
        usleep(90000);
    } 

    while(!bomba->destroyable && gamestate){
        usleep(10000);
    }

    return NULL;
}

void* space(void* param){

    entityList* entities = (entityList*) param;

    entityNode *iter;
    gamestate = false;
    int aliencounter = 0;

    // rimane in attesa se la lista delle entità è vuota
    // una volta riempita il gioco può iniziare
    while(!gamestate){
        usleep(10000);
        if(entities->len >= 1){
            gamestate = true;
        } 
    }

    do{
        pthread_mutex_lock(&mutex);

        clearField();

        aliencounter = 0;
        iter = entities->head;
       
        // loop di scorrimento della lista
        while(iter != NULL && gamestate){

            // a seconda del tipo di entità puntata verrà chiamata la rispettiva 
            // funzione di controllo delle collisioni

            if(!iter->data.destroyable){
                switch(iter->data.e){
                    case DP:
                        if(iter->data.lives == 0){
                            gamestate = false;
                            EL = gameEnding(EL);
                            mvprintw(MAXY/2,MAXX/2-16,"you lose, the planet is doomed");
                            break;
                        }else{
                          shipCollisions(entities,iter);  
                        }  
                    break;

                    case ORFIST:
                        aliencounter +=1;
                        alienCollisions(entities,iter);                  
                    break;

                    case GRAVIDIA:
                        aliencounter +=1;
                        alienCollisions(entities,iter);
                    break;

                    case CRITICAL:
                        if(iter->data.hb.topLeft.x >= MAXX-1){
                            iter->data.lives = 0;
                        }else{
                            projectileCollisions(entities,iter);  
                        }
                    break;

                    case METEOR:
                        if(iter->data.hb.topLeft.x <= 1){
                            iter->data.lives = 0;
                        }else{
                            bombaCollisions(entities,iter);
                        }
                    break;
                }

                //sezione di stampa / cancellazione in caso di morte
                if(iter->data.lives <= 0){
                    if(iter->data.e == ORFIST){
                        // sezione per l'evoluzione degli alieni di primo livello
                        bodyClearing(iter);
                        iter->data.e = GRAVIDIA;
                        iter->data.lives = 4;
                    } else {
                        EL = eraseEntity(iter,entities); 
                    }
                }else{
                    printer(&iter->data);
                }

            }else if(iter->data.e == DP){
                gamestate = false;
                break;
            }
            else{
                pthread_join(iter->data.thr,NULL);
            }
            iter = iter->next;   
        }

        drawFieldBorder();
        
        if((wavecount >= WAVES-1) && (aliencounter == 0)&& publ_pl_lives > 0){
            gamestate = false;
            EL = gameEnding(EL);
            mvprintw(MAXY/2,MAXX/2-14,"you win, the planet is saved");
            break;
        }

        refresh();

        pthread_mutex_unlock(&mutex);
        
        usleep(20000);
    }while(gamestate);

    return NULL;
}

// funzioni per regolare la lista

entityList* initEntityList(){
    entityList *list;
    list->head = NULL;
    list->tail = NULL;
    list-> len = 0;
    return list;
}

entityNode *insert(Entity data, entityList *list){
    entityNode *ent;
    ent = malloc(sizeof(entityNode));

    ent->next = NULL;
    ent->prev = NULL;

    ent->data = data;

    if (list->head == NULL) 
    {
        list->head = ent;
        list->tail = ent;
        ent->prev = NULL;
        ent->next = NULL;
    }
    else
    {
        // Setting ent data
        ent->prev= list->tail;
        ent->next = NULL;

        // Setting list data
        list->tail->next = ent;
        list->tail = ent;
    }

    list->len +=1;

    return ent;
}

entityNode *newEntity(Entity data){
    entityNode *ent;
    ent = malloc(sizeof(entityNode));
    ent->next = NULL;
    ent->prev = NULL;

    ent->data = data;

    return ent;
}

entityList* eraseEntity(entityNode *forDeletion, entityList *list){

    entityNode *ent;

    if (list->head != NULL) // If the list is not empty
    {
        ent = list->head;
        while (ent != NULL)
        {
            if (ent == forDeletion)
            {
                entityNode *predecessor, *successor;
                predecessor = ent->prev;
                successor = ent->next;
                
                if (list->head != list->tail)
                {
                    if (ent != list->head)
                    {
                        if (ent != list->tail)
                        {
                            predecessor->next = successor;
                            successor->prev = predecessor;
                        }
                        else
                        {
                            predecessor->next = NULL;
                            list->tail = predecessor;
                        }
                    }
                    else
                    {
                        list->head = successor;
                        successor->prev = NULL;
                    }
                }
                else
                {  
                    list->head = NULL;
                    list->tail = NULL;
                }
            } 
            ent = ent->next;         
        }
    }

    list->len -=1;

    bodyClearing(forDeletion);
    forDeletion->data.destroyable = true;

    return list;
}

// funzioni di controllo collisioni

void shipCollisions(entityList *list, entityNode *target){
    entityNode *obj;

    obj = list->head->next;
    while(obj != NULL){
        if(obj->data.thr != target->data.thr){
            if(verifyHitbox(&target->data.hb,&obj->data.hb)){
                if(obj->data.e != CRITICAL && obj->data.e != DP){
                    target->data.lives -= 1;
                    obj->data.lives -= 1;
                }
            }
        }
        obj = obj->next;
    }
}

void alienCollisions(entityList *list, entityNode *target){
    entityNode  *obj;

    obj = list->head;
    while(obj != NULL){  
        if(obj->data.thr != target->data.thr){
            if(verifyHitbox(&target->data.hb,&obj->data.hb)){
                if(obj->data.e != METEOR){
                    switch(obj->data.e){
                        case DP:
                            target->data.lives = -1;
                            publ_pl_lives = obj->data.lives -= 1;
                        break;

                        case CRITICAL:
                            target->data.lives -= 1;
                            obj->data.lives -= 1;
                        break;

                        default:
                            adjustCourse(&target->data,&obj->data);
                        break;
                    }
                }  
            }
        }
        obj = obj->next;
    }
}

void bombaCollisions(entityList *list, entityNode *target){
    entityNode *obj;

    obj = list->head;
    while(obj != NULL){
        if(obj->data.thr != target->data.thr){
            if(verifyHitbox(&target->data.hb,&obj->data.hb)){
                if(obj->data.e == DP){
                    target->data.lives -= 1;
                    publ_pl_lives = obj->data.lives -= 1;   
                }
            }
        }
        obj = obj->next;   
    }
}

void projectileCollisions(entityList *list, entityNode *target){
    entityNode *obj;

    obj = list->head;
    while(obj != NULL){
        if(obj->data.thr != target->data.thr){
            if(verifyHitbox(&target->data.hb,&obj->data.hb)){
                if(obj->data.e != DP){
                    target->data.lives -= 1;
                    obj ->data.lives -= 1;
                } 
            }
        }
        obj = obj->next;   
    }
}

void adjustCourse(Entity *a, Entity *b){
    switch(a->d){
        case NWEST:
            switch(b->d){
                case SWEST:
                    a->d = SWEST;
                    //+1 y
                    a->hb.topLeft.y += 1;
                    a->hb.botRight.y += 1;

                    b->d = NWEST;
                    //-1 y
                    b->hb.topLeft.y -= 1;
                    b->hb.botRight.y -= 1;
                break;

                case FIXED:
                    a->d = FIXED;
                    //+1 y
                    a->hb.topLeft.y += 1;
                    a->hb.botRight.y += 1;
                    
                    b->d = NWEST;
                    //-1 y
                    b->hb.topLeft.y -= 1;
                    b->hb.botRight.y -= 1;
                break;

                case NWEST:
                    a->hb.topLeft.y -= 2;
                    a->hb.botRight.y -= 2;

                    b->hb.topLeft.y +=1;
                    b->hb.botRight.y +=1;
                break;
            }
        break;

        case SWEST:
            switch(b->d){
                case NWEST:
                    a->d = NWEST;
                    //-1 y
                    a->hb.topLeft.y -= 1;
                    a->hb.botRight.y -= 1;
                    
                    b->d = SWEST;
                    //+1 y
                    b->hb.topLeft.y +=1;
                    b->hb.botRight.y +=1;
                break;

                case FIXED:
                    a->d = FIXED;
                    //-1 y
                    a->hb.topLeft.y -= 1;
                    a->hb.botRight.y -= 1;

                    b->d = SWEST;
                    //+1 y
                    b->hb.topLeft.y += 1;
                    b->hb.botRight.y += 1;
                break;

                case SWEST:
                    a->hb.topLeft.y += 2;
                    a->hb.botRight.y += 2;

                    b->hb.topLeft.y -=1;
                    b->hb.botRight.y -=1;

                break;
            }
        break;

        case FIXED:
            switch(b->d){
                case NWEST:
                    a->d = NWEST;
                    //-1 y
                    a->hb.topLeft.y -= 1;
                    a->hb.botRight.y -= 1;
                    
                    b->d = FIXED;
                    //+1 y
                    b->hb.topLeft.y += 1;
                    b->hb.botRight.y += 1;
                break;

                case SWEST:
                    a->d = SWEST;
                    //+1 y
                    a->hb.topLeft.y += 1;
                    a->hb.botRight.y += 1;
                    
                    b->d = FIXED;
                    //-1 y
                    b->hb.topLeft.y -= 1;
                    b->hb.botRight.y -= 1;
                break;

                case FIXED:
                    a->hb.topLeft.x += 2;
                    b->hb.botRight.x -=1;
                break;
            }
        break;
    }
}

_Bool verifyHitbox(Hitbox *a, Hitbox *b){

    if(((a->topLeft.x >= b->topLeft.x && a->topLeft.x <= b->botRight.x) || (a->botRight.x >= b->topLeft.x && a->botRight.x <= b->botRight.x)) && 
       ((a->topLeft.y >= b->topLeft.y && a->topLeft.y <= b->botRight.y) || (a->botRight.y >= b->topLeft.y && a->botRight.y <= b->botRight.y)))
    {
        return true;
    }else{
        return false;
    }
}

// funzioni d'avvio proiettili

void blast(entityList *list, Entity* ship){ 

    Entity r1,r2;
    pthread_t m1,m2;

    r1.hb.topLeft.x = r1.hb.botRight.x = ship->hb.botRight.x+1;
    r1.hb.topLeft.y = r1.hb.botRight.y = ship->hb.topLeft.y+1;
    r1.lives = 1;
    r1.e = CRITICAL;
    r1.d = NEAST;
    r1.color = 6;
    r1.destroyable = false;
     
    r2.hb.topLeft.x = r2.hb.botRight.x = ship->hb.botRight.x+1;
    r2.hb.topLeft.y = r2.hb.botRight.y = ship->hb.botRight.y-1;
    r2.lives = 1;
    r2.e = CRITICAL;
    r2.d = SEAST;
    r2.color = 4;
    r2.destroyable = false;

    entityNode *r1n = insert(r1,list); 
    entityNode *r2n = insert(r2,list);

    pthread_create(&m1,NULL,&missile,(void*)r1n);
    pthread_create(&m2,NULL,&missile,(void*)r2n);
}

void bombaDeployer(entityList *list, Entity *Alien, int attackChance){
    if(rand()%attackChance == 0){
        Entity b;
        pthread_t bt;

        b.hb.topLeft.x = b.hb.botRight.x = Alien->hb.topLeft.x-1;
        b.hb.topLeft.y = b.hb.botRight.y = Alien->hb.topLeft.y+1;
        b.lives = 1;
        b.e = METEOR;
        b.d = FIXED;
        b.color = 7;
        b.destroyable = false;
          
        entityNode *bn = insert(b,list);

        pthread_create(&bt,NULL,&bomba,(void*)bn);   
    }
}

// funzioni per lo schermo

void printer(Entity* ent){
    Entity aux;
    int adj = 0;

    // individuazione entità...
    switch(ent->e){

      // giocatore
        case DP:
            attron(COLOR_PAIR(ent->color));
            for(int i = 0; i<PLAYER_SIZE;i++){
                for(int j = 0;j <PLAYER_SIZE;j++){
                    mvaddch(ent->hb.topLeft.y+i,ent->hb.topLeft.x+j, DPBody[i][j]);
                }
            }
            attroff(COLOR_PAIR(ent->color));
        break;

      // alieno lvl 1
        case ORFIST:
            attron(COLOR_PAIR(ent->color));
            for(int i = 0; i<ALIEN_SIZE;i++){
                for(int j = 0;j <ALIEN_SIZE;j++){
                    mvaddch(ent->hb.topLeft.y+i,ent->hb.topLeft.x+j, ORFISTBody[i][j]);
                }
            }  
            attroff(COLOR_PAIR(ent->color));
        break;

        case GRAVIDIA:
            attron(COLOR_PAIR(ent->color));

            int x,y;

            for (int i = 0; i < ent->lives; ++i)
            {
                switch(i){
                    case 0:
                        y = ent->hb.topLeft.y;
                        x = ent->hb.topLeft.x;
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

                for (int j = 0; j < ALIEN_SIZE-1; j++)
                {
                    for (int k = 0; k < ALIEN_SIZE; k++)
                    {
                        mvaddch(y+j,x+k,GRAVIDIABody[j][k]);
                    }
                }
            }

            attroff(COLOR_PAIR(ent->color));
        break;
  
        // missili
        case CRITICAL:
            if(ent->hb.topLeft.x < MAXX && (ent->hb.topLeft.y > 0 && ent->hb.topLeft.y < MAXY)){
               attron(COLOR_PAIR(ent->color));
               mvaddch(ent->hb.topLeft.y,ent->hb.topLeft.x,CRITICALBody); 
               attroff(COLOR_PAIR(ent->color));
            }         
        break;

        case METEOR:
            if(ent->hb.topLeft.x > 0){
               attron(COLOR_PAIR(ent->color));
               mvaddch(ent->hb.topLeft.y,ent->hb.topLeft.x,METEORBody); 
               attroff(COLOR_PAIR(ent->color));
            }         
        break;
    }
}

void bodyClearing(entityNode *body){

    int size,x,y;

    y = body->data.hb.topLeft.y;
    x = body->data.hb.topLeft.x;

    switch(body->data.e){
        case DP:
            size = PLAYER_SIZE; 
        break;

        case ORFIST:
            size = ALIEN_SIZE;
        break;

        case GRAVIDIA:
            size = ALIEN_SIZE*2;
        break;

        default:
            size = 1;
        break;
    }


    for(int i = 0; i < size;i++){
        for(int j = 0 ;j < size ;j++){
            mvaddch(y+i,x+j,' ');
        }
    }
}

void drawFieldBorder(){
    /**area**/
    int i = 0;
    int j = 0;
    for (j; j <= MAXY; j++)
    {
        i = 0;
        if (j >= 1 && j < MAXY)
        {
            mvaddch(j, i, ACS_VLINE);
            mvaddch(j, MAXX, ACS_VLINE);
        }
        else if (j == 0 || j == MAXY)
        {
            for (i = 0; i < MAXX; i++)
            {
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

void clearField(){
    for (int i = 0; i < MAXY+3; i++){
        for (int j = 0; j < MAXX+3; j++){
            mvaddch(i,j,' ');
        }
    }
} 

// funzioni per avvio e termine del gioco

int game(){
    srand(time((time_t*)NULL));
    initscr();
    start_color();
    init_pair(2,COLOR_YELLOW,COLOR_BLACK); 
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(3,COLOR_GREEN,COLOR_BLACK);
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);
    init_pair(6,COLOR_CYAN,COLOR_BLACK);
    init_pair(7,COLOR_WHITE,COLOR_BLACK);
    noecho();
    curs_set(0);

    EL = initEntityList();

    pthread_t plThread, alThread, spThread;

    pthread_mutex_init(&mutex,NULL);

    pthread_create(&plThread,NULL,&ship,(void*)EL);
    pthread_create(&spThread,NULL,&space,(void*)EL);
    pthread_create(&alThread,NULL,&alienGenerator,(void*)EL);

    pthread_join(alThread,NULL);
    pthread_join(spThread,NULL);
    pthread_mutex_destroy(&mutex);
    
    mvprintw((MAXY/2)+1,(MAXX/2)-16,"game over,type 'up' to get out"); 
    refresh();

    while(getch() != 65){
        usleep(1);
    }
    endwin();    
    return 0;
}

entityList* gameEnding(entityList *list){
    entityNode* temp;
    entityNode* head = list->head;

    move(0,MAXX);

    while(head != NULL){
        temp = head;
        head = head->next;
        free(temp);
    }

    clearField();

    return list;
}  

// dove non si trova il problema 

