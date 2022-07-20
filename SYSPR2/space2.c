#include "spaceWarCrimes2.h"

void main(){

    initscr();
    start_color();
    init_pair(2,COLOR_YELLOW,COLOR_BLACK); 
    init_pair(5,COLOR_MAGENTA,COLOR_BLACK);
    init_pair(3,COLOR_GREEN,COLOR_BLACK);
    init_pair(1,COLOR_RED,COLOR_BLACK);
    init_pair(4,COLOR_BLUE,COLOR_BLACK);
    init_pair(6,COLOR_CYAN,COLOR_BLACK);
    noecho();
    curs_set(0);
    srand(time(NULL));

    int mainpipe[2];
    int f2p[2];
    int f2m[2];
    int f2a[2];

    if(pipe(mainpipe) == -1){
        perror("eeee la fin de la mainpipe");
    }

    if(pipe(f2p) == -1 && fcntl(f2p[0],F_SETFL,O_NONBLOCK) == -1){
      perror("eee la fin de la pipe de el misil");
   }

   if(pipe(f2m) == -1 && fcntl(f2m[0],F_SETFL,O_NONBLOCK) == -1){
      perror("eee la fin de la pipe de el misil");
   }

   if(pipe(f2a) == -1 && fcntl(f2a[0],F_SETFL,O_NONBLOCK) == -1){
      perror("eee la fin de la pipe de el misil");
   }


    pid_t Pl,Al;

    refresh();
    
    Al = fork();

    switch(Al){
        case -1:
            perror("eeeee la fin de PL");
        break;

        case 0:
            close(mainpipe[0]);
            close(f2a[0]);
            alienGenerator(mainpipe[1],f2a[0]);
        break;

       default:

       Pl = fork();

            switch(Pl){
                case -1:
                    perror("eeeee la fin de AL");
                break;

                case 0:
                    close(mainpipe[0]);
                    close(f2p[0]);
                    close(f2m[0]);
                    player(mainpipe[1],f2p[0],f2m[0]);                  
                break;

                default:
                    close(mainpipe[1]);
                    close(f2p[0]);
                    close(f2m[0]);
                    close(f2a[0]);
                    greatFilter(mainpipe[0],f2p[1],f2m[1],f2a[1]);                
                break;
            }
       break;
    } 
    
    endwin();
    printf("the game ended\n");
    exit(0);
}

