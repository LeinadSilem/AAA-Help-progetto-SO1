#include "space.h"

void initialize();
void gameStart();
void gameEnd();

int main()
{
	initialize();
	gameStart();
	gameEnd();
	printf("Bye!");
	return 0;
}

void initialize()
{
	/** Initial settings: screen and colours **/
	initscr();
	noecho();
	curs_set(0);
	erase();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);

	srand(time(NULL)); // Rand initialization
}

void gameEnd()
{

	/** Final settings: screen **/
	erase();
	curs_set(1);
	endwin();
}


void gameStart(){

	int communicationPipe[2];
	pid_t pidShip, pidAliens;

	if (pipe(communicationPipe) == -1)
	{
		perror("Error in the initialization of communication pipes!");
		exit(-1);
	}

	
	switch(fork()){
		case -1:
			perror("eee la fin de la pipe");
			exit(-1);
		break;

		case 0:
			close(communicationPipe[0]); // Closing reading pipe
			ship(communicationPipe[1]);
			exit(1);
		break;
		
		default:
			switch(fork()){
				case -1:
					perror("eee la fin de la pipe");
					exit(-1);
				break;

				case 0:
					close(communicationPipe[0]);
					alienGenerator(communicationPipe[1]);
					exit(1);
				break;

				default:
					space(communicationPipe[0], communicationPipe[1]);
				break;
			}
		break;
	}
}

/*
void gameStart()
{
	int communicationPipe[2];
	pid_t pidShip, pidAliens;

	if (pipe(communicationPipe) == -1)
	{
		perror("Error in the initialization of communication pipes!");
		exit(-1);
	}

	pidShip = fork();

	if (pidShip == 0) // This process is ship process
	{
		close(communicationPipe[0]); // Closing reading pipe
		ship(communicationPipe[1]);
		exit(1);
	}

	pidAliens = fork();

	if(pidAliens == 0){
		close(communicationPipe[0]);
		alienGenerator(communicationPipe[1]);
		exit(1);
	}

	// This is the main thread
	space(communicationPipe[0], communicationPipe[1]);
}
*/