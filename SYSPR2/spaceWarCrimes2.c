#include "spaceWarCrimes2.h"

/**
 * player:
 * - ha la pipe principale dove scrive la posizione aggiornata
 * - ha la pipe privata per il player, nella quale arrivano 
 *   aggiornamenti di stato"
 * - ha la pipe privata per i missili in modo da passarla ai
 *   missili creati durante lo sparo
 * */
void player(int leTube, int playerpipe, int missilepipe){
   Entity player, tempPlayer;

   // inizializzazione player...
   player.p.x = 0;
   player.p.y = (MAXY/2)-2;
   player.lives = 3;
   player.d = FIXED;
   player.e = DP;
   player.pid = getpid();
   player.color = 1;
   player.isTracked = false;

   pid_t ms;

   // scrittura iniziale del player...
   write(leTube,&player,sizeof(player));

   char c;
   
   // inizio loop attività...
   while(player.lives > 0){ 
      
      // leggi possibile aggiornamento dal filtro...
      read(playerpipe,&tempPlayer,sizeof(player));

      if(tempPlayer.lives != 0){
         player.lives += tempPlayer.lives;
         tempPlayer.lives = 0;
      }

      // ricezione input direzione...
      switch(c=getch()){
         case UP:
            if(player.p.y > 0){
               player.p.y--;
               player.d = N;
            }
         break;

         case DOWN:
            if(player.p.y < MAXY-(PLAYER_SIZE-1)){
               player.p.y++;
               player.d = S;
            }
         break;

         // bottone di autodistruzione, for testing uses only
         case 'q':
            player.lives = 0;
         break;

         // bottone di sparo, generazione missili
         case 'e':
            
            for(int i = 0; i < SHOTS_ALLOWED; i++){              
               if(fork()==0){
                  missile(leTube,player.p.y,player.p.x,missilepipe);
               }         
            }
         break;        
      } 

      // scrive al filtro le modifiche...
      write(leTube,&player,sizeof(player));
   }  

   printf("player ended...\n");
   exit(0);
}

/**
 * missile:
 * - ha la pipe principale dove scrive la posizione aggiornata
 * - ha una copia del player, per la generazione iniziale della 
 *   posizione dei missili
 * - ha la pipe privata per i missili in modo da ricevere 
 *   aggiornamenti di stato"
 * */
void missile(int leTube, int start_y, int start_x, int missilepipe){

   Entity missile,temp;

   // inizializzazione missili...
   missile.p.x = start_x + PLAYER_SIZE;
   missile.pid = getpid();
   missile.lives = 1;
   missile.e = CRITICAL;
   missile.isTracked = false;

   // decisione direzione iniziale missili...
   if(missile.pid%2 == 0){
      missile.p.y = start_y+1;
      missile.d = NEAST;
      missile.color = 6;
   }else{
      missile.p.y = start_y+3;
      missile.d = SEAST;
      missile.color = 4;
   }

   // scrittura iniziale...
   write(leTube,&missile,sizeof(missile));

   // inizio attività, condizioni: se il missile 
   // è in volo e non ha superato il bordo destro
   while(missile.p.x < MAXX && missile.lives > 0){


      // possibile aggiornamento del missile..
      read(missilepipe,&temp,sizeof(missile));

      // controllo pid per aggiornare correttamente i cambi...
      if(missile.pid == temp.pid){
         missile.lives += temp.lives;
         missile.d = temp.d;
         missile.isTracked = temp.isTracked;
      }

      // controllo direzione missile e rimbalzo coi bordi verticali...
      switch(missile.d){
         case NEAST:
            missile.p.y--;
            if(missile.p.y < 0){
               missile.d = SEAST;
               missile.p.y++;
            }
         break;

         case SEAST:
            missile.p.y++;
            if(missile.p.y > MAXY){
               missile.d = NEAST;
               missile.p.y--;
            }
         break;
      }

      missile.p.x += 3;

      // scrittura al filtro e breve pausa...
      write(leTube,&missile,sizeof(missile));
      usleep(50000);
   }

   exit(0);
}

/**
 * alien:
 * - ha la pipe principale dove scrive la posizione aggiornata
 * - pipe privata per gli alieni dove ricevono aggiornamenti di stato
 * */
void alien(int leTube, int alienpipe){

   Entity alien,tempAlien;

   alien.e = ORFIST;
   alien.lives = 1;
   alien.pid = getpid();
   alien.isTracked = false;

   int i = alien.pid%3;

   switch(i){
      case 0:
         alien.d = NWEST; 
         alien.p.y = MAXY;
         alien.p.x = MAXX - ALIEN_SIZE;
         alien.color = 2;
      break;
         
      case 1:
         alien.d = SWEST;
         alien.p.y = 0;
         alien.p.x = MAXX - ALIEN_SIZE; 
         alien.color = 5;
      break;
         
      case 2:
         alien.d = FIXED;
         alien.p.y = (MAXY/2);
         alien.p.x = MAXX - ALIEN_SIZE;
         alien.color = 3;
      break;
   }

   write(leTube,&alien,sizeof(alien));

   while(alien.p.x > MINX && alien.lives > 0){
        
      read(alienpipe,&tempAlien,sizeof(tempAlien));
      if(tempAlien.pid == alien.pid){
         alien.lives += tempAlien.lives;
         alien.d = tempAlien.d;
         alien.isTracked = tempAlien.isTracked;
      }  

      switch(alien.d){
         case NWEST:
             alien.p.y--;
             alien.p.x--;
             if(alien.p.y <= 0  || alien.p.y == (MAXY/2)+(ALIEN_SIZE-1)){
                 alien.d = SWEST;
                 alien.p.y++;
             }
         break;

         case SWEST:
             alien.p.y++;
             alien.p.x--;
             if(alien.p.y >= MAXY - (ALIEN_SIZE-1) || alien.p.y == (MAXY/2)-(ALIEN_SIZE-1)){
                alien.d = NWEST;
                alien.p.y--;
             }
         break;

         case FIXED:
             alien.p.x--;
         break;
      }

      write(leTube,&alien,sizeof(alien));
      sleep(1);
   }

   //inizia trasformazione in

  exit(0);
}

/*
 * alienGenerator:
 * - pipe principale per la scrittura da passare agli alieni
 * - pipe privata per gli alieni dove ricevono aggiornamenti di stato
 * - SENZA QUESTA NON FUNZIONANO GLI ALIENI
 */
void alienGenerator(int leTube, int alienpipe){

   while(1){
      for(int i = 0; i < ALIENS_ALLOWED; i++){
         if(fork()==0){
            alien(leTube,alienpipe);
         }
      }
      sleep(30);
   }
}

/**
 * printer:
 * - ha l'entità passatagli dalla funzione filtro da stampare
 * */
void printer(Entity ent){

   Position aux;

   // matrice giocatore...
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
      "]M[",
      "^v^"
   };

   // matrice alieno lvl 2
   char GRAVIDIABody [3][3] = 
   {
      "<A>",
      "{0}",
      "/|\\"
   };

   // asterisco per i missili,zero per la bomba
   char CRITICALBody = '*';
   char METEORBody = '0';

   // individuazione entità...
   switch(ent.e){

      // giocatore
      case DP:
         attron(COLOR_PAIR(ent.color));
         for(int i = 0; i<PLAYER_SIZE;i++){
            for(int j = 0;j <PLAYER_SIZE;j++){
               mvaddch(ent.p.y+i,ent.p.x+j, DPBody[i][j]);
            }
         }
         attroff(COLOR_PAIR(ent.color));
      break;

      // alieno lvl 1
      case ORFIST:
         attron(COLOR_PAIR(ent.color));
         for(int i = 0; i<ALIEN_SIZE;i++){
            for(int j = 0;j <ALIEN_SIZE;j++){
               mvaddch(ent.p.y+i,ent.p.x+j, ORFISTBody[i][j]);
            }
         }  
         attroff(COLOR_PAIR(ent.color));
      break;

      // alieno lvl2
      case GRAVIDIA:
      aux = ent.p;

      for(int k = 0; k<=4; k+2){
         for(int i = 0; i<ALIEN_SIZE;i++){
            for(int j = 0;j <ALIEN_SIZE;j++){
               mvaddch(aux.y+i, aux.x+j, GRAVIDIABody[i][j]);
            }
         }
         if(k%2 == 0){
            aux.x +=3;
         }else{
           aux.x -=3;
           aux.y +=3; 
         }        
      }      
      break;

      // bomba
      case METEOR:

         mvaddch(ent.p.y,ent.p.x,METEORBody);
      break;

      // missili
      case CRITICAL:
         if(ent.p.x < MAXX && ent.p.y > 0 && ent.p.y <MAXY){
            attron(COLOR_PAIR(ent.color));
            mvaddch(ent.p.y,ent.p.x,CRITICALBody); 
            attroff(COLOR_PAIR(ent.color));
         }         
      break;
   }
}

void moveInCollisionMap(int y, int x, mapCell map[y][x], Entity tempEntity){

   int size;

   switch(tempEntity.e){

     // player
      case DP:
         size = PLAYER_SIZE;
      break;

      // missili
      case GRAVIDIA:
         size = ALIEN_SIZE*2;
      break;
         
      //alieni primo livello
      case ORFIST:
        size = ALIEN_SIZE;
      break;

      default:
         size = 1;
      break;

   }

   for(int i = tempEntity.p.y; i<size;i++){
      for(int j = tempEntity.p.x;j <size;j++){
         map[i][j].pid = tempEntity.pid;
         map[i][j].d = tempEntity.d;
         map[i][j].e = tempEntity.e;
         map[i][j].p.y = tempEntity.p.y;
         map[i][j].p.x = tempEntity.p.x;
      }
   }  
}

void deleteInCollisionMap(int y, int x, mapCell map[y][x], Entity tempEntity){

   int size;


   switch(tempEntity.e){

      // player
      case DP:
         size = PLAYER_SIZE;
      break;

      // missili
      case GRAVIDIA:
         size = ALIEN_SIZE*2;
      break;
         
      //alieni primo livello
      case ORFIST:
        size = ALIEN_SIZE;
      break;

      default:
         size = 1;
      break;

   }
   
   for (int i = tempEntity.p.y; i < size+tempEntity.p.y; i++)
   {
      for (int j = tempEntity.p.x; j < size+tempEntity.p.x; j++)
      {
            map[i][j].pid = 0;
            map[i][j].d = HALT;
            map[i][j].e = BLANK;
            map[i][j].p.y = 0;
            map[i][j].p.x = 0;
      }
   }
}

void greatFilter(int leTube, int playerpipe, int missilepipe, int alienpipe){

   //variabili
   bool gamestate = true;
   Entity tempEntity;
   int midborder = MAXY/2;
   int adj;

   //inizializza mappa
   mapCell collisionMap[MAXY][MAXX];
   for(int i = 0;i<MAXY;i++){
      for (int j = 0; j <MAXX; j++)
      {
         collisionMap[i][j].e = BLANK;
         collisionMap[i][j].pid = 0;
         collisionMap[i][j].d = HALT;
         collisionMap[i][j].p.y = collisionMap[i][j].p.x = 0;
      }
   }  

   // inizio loop...
   do{

      // legge la prima entità dipsonibile...
      read(leTube,&tempEntity,sizeof(tempEntity));

      if(!tempEntity.isTracked){
         moveInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);
         tempEntity.isTracked = true;
      }

      if(tempEntity.lives > 0){

         if(!verifyCollision(tempEntity,MAXY,MAXX,collisionMap,playerpipe,alienpipe,missilepipe)){
            deleteInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);
            moveInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);
         }

         // identificazione entità...
         switch(tempEntity.e){

            // player
            case DP:

               mvprintw(0,(MAXX/2)-10,"player lives: %d player pid:%d player y:%d",tempEntity.lives,tempEntity.pid,tempEntity.p.y); 
                         
               switch(tempEntity.d){
                  case N:
                     adj = 1;
                  break;

                  case S:
                     adj = -1;
                  break;
               }

               for(int i = 0; i<PLAYER_SIZE;i++){
                  mvprintw(tempEntity.p.y+i+adj,tempEntity.p.x,"     ");
               }    

               break;

               // missili
               case CRITICAL:

                  switch(tempEntity.d){
                     case NEAST:
                        mvprintw(tempEntity.p.y+1,tempEntity.p.x-3," ");
                     break;

                     case SEAST:
                        mvprintw(tempEntity.p.y-1,tempEntity.p.x-3," ");
                     break;
                  }

                  //collisioni col bordo destro, suicidio se condizione viene soddisfatta
                  if(tempEntity.p.x >= MAXX && tempEntity.lives > 0){  
                     deleteInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);    
                     kill(tempEntity.pid,SIGKILL);
                  }

               break;

               
               //alieni primo livello
               case ORFIST:

                  switch(tempEntity.d){
                     case NWEST:

                        if(tempEntity.p.y == midborder){
                           tempEntity.d = SWEST;
                           write(alienpipe,&tempEntity,sizeof(tempEntity));
                        }

                        for(int i = 0;i < ALIEN_SIZE;i++){
                           mvprintw(tempEntity.p.y+i+1,tempEntity.p.x+1,"   ");              
                        }

                     break;

                     case SWEST:

                        if(tempEntity.p.y == midborder){
                           tempEntity.d = NWEST;
                           write(alienpipe,&tempEntity,sizeof(tempEntity));
                        }

                        for(int i = 0;i< ALIEN_SIZE;i++){
                           mvprintw(tempEntity.p.y+i-1,tempEntity.p.x+1,"   ");             
                        }

                     break;

                     case FIXED:

                        for(int i = 0; i <ALIEN_SIZE;i++){
                           mvprintw(tempEntity.p.y+i,tempEntity.p.x+1,"   ");     
                        }

                     break;
                  }

                  if(tempEntity.p.x <= 0 || tempEntity.lives <= 0){      
                     deleteInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);
                     kill(tempEntity.pid,SIGKILL); 
                     gamestate = false;
                  }

               break;

            }

            // stampa l'entità letta...
            printer(tempEntity);

      }else{

         if(tempEntity.e == DP){
            gamestate = false;
         }

         deleteInCollisionMap(MAXY,MAXX,collisionMap,tempEntity);
         kill(tempEntity.pid,SIGKILL);
      }

      refresh();
   }while(gamestate);

   // TODO: kill everything function goes here

   exit(0);
}

bool verifyCollision(Entity tempEntity, int x, int y, mapCell map[y][x], int playerpipe,int alienpipe,int missilepipe){
   
   int size;
   Entity auxEntity;

   switch(tempEntity.e){
      case DP:
      size = PLAYER_SIZE+2;
      break;

      case ORFIST:
      size = PLAYER_SIZE;
      break;

      case GRAVIDIA:
      size = (ALIEN_SIZE*2)+1;
      break;

      default:
      size = ALIEN_SIZE;
      break;
   }


   for (int i = tempEntity.p.y; i < size+tempEntity.p.y; ++i)
   {
      for (int j = tempEntity.p.x; j < size+tempEntity.p.x; ++j)
      {
         if(map[i][j].pid != BLANK || map[i][j].pid != tempEntity.pid){
            
            auxEntity.pid = map[i][j].pid;
            auxEntity.d = map[i][j].d;
            auxEntity.e = map[i][j].e;
            auxEntity.p.y = map[i][j].p.y;
            auxEntity.p.x  = map[i][j].p.x;
            
            break;
         }
      }
   }

   if(auxEntity.pid != 0){

      switch(tempEntity.e){
         case DP:

            switch(auxEntity.e){
               case METEOR:
                  //write to bomba
               break;

               case ORFIST:
                  auxEntity.lives = -1;
                  write(alienpipe,&auxEntity,sizeof(auxEntity));
               break;

               case GRAVIDIA:
                  auxEntity.lives = -1;
                  write(alienpipe,&auxEntity,sizeof(auxEntity));
               break;
            }

            tempEntity.lives-=1;
            write(playerpipe,&tempEntity,sizeof(tempEntity));
         break;

         case METEOR:

            if(auxEntity.e == DP){
               auxEntity.lives = -1;
               write(playerpipe,&auxEntity,sizeof(auxEntity));
               
               //tempEntity.lives -= 1;
               //write(bombapipe,&tempEntity,sizeof(tempEntity));
            }
            
         break;

         case CRITICAL:

            if(auxEntity.e == ORFIST || auxEntity.e == GRAVIDIA){
               auxEntity.lives = -1;
               write(alienpipe,&auxEntity,sizeof(auxEntity));
               
               tempEntity.lives -= 1;
               write(missilepipe,&tempEntity,sizeof(tempEntity));
            }

         break;

         default:

            switch(auxEntity.e){
               case DP:
                  auxEntity.lives = -1;
                  write(playerpipe,&auxEntity,sizeof(auxEntity));
               break;

               case CRITICAL:
                  auxEntity.lives = -1;
                  write(missilepipe,&auxEntity,sizeof(auxEntity));
               break;
            }

            tempEntity.lives -= 1;
            write(alienpipe,&tempEntity,sizeof(tempEntity));
         break;
      }
      return true;
   } 
     
   return false; 
}

