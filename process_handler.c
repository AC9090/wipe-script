#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string.h>

#define MAX_PROCESSES 10

#define NUM_WINDOWS 4


//Definitions for window structuring
#define BORDER_UP 3
#define BORDER_DN 1


#define SECURE_ERASE "SE"
#define ERROR "ER"
#define ESTIMATED_TIME "ET"

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');

	wrefresh(local_win);
	delwin(local_win);
}

int main(int argc, char *argv[])
{
	printf("Begining...\n");
	

	int pcount = 0;

	char* targets[MAX_PROCESSES];
	char* clone;


	int i = 1;
	if ((argv[1][0] == '-')){ //this indicates a clone
		clone = &argv[1][1];
		i++;
	}
	for (i; i < argc; i ++){
		targets[pcount] = argv[i];
		pcount ++;
	}

	//Now we have the arguments, create a pipe for each process.


	int filedes[pcount][2]; //the pipe identifiers for each process.
	pid_t child_pids[pcount];

	for (i = 0; i < pcount; i++){
		if (pipe(filedes[i]) == -1){
			perror("fork");
			exit(1);
		}

		pid_t pid = fork();
		if (pid == -1){ 
			perror("fork");
			exit(1);
		}else if (pid == 0){
			close(filedes[i][0]);
			close(filedes[i][1]);
			sleep(5); //wait for ncurses to be set up
			if (!clone){
				printf("Target %s will be initialized with no clone.\n", targets[i]);
			} else {
				printf("Target %s will be initialized with clone %s.\n", targets[i], clone);
			}
			perror("execl");
			_exit(1); //dunno why the exit 1
		}
		child_pids[i] = pid;
		close(filedes[i][1]);
	}

	// The processes have started, Now it's time to start curses.

	char buffer[256];

	int rows, cols;

	initscr();
	getmaxyx(stdscr, rows, cols);

	cbreak(); //typed characers are not buffered
	keypad(stdscr, TRUE);

	int ch; //the character on the keyboard that has been pressed
    refresh();
	WINDOW *windows[pcount];

	for(i = 0; i < pcount; i++){
		windows[i] = create_newwin((rows - BORDER_UP - BORDER_DN), cols/4,
			BORDER_UP, i * cols/4);
	}

	while((ch = getch()) != KEY_F(1)){ //f1 to exit, may change this

		// switch(ch)
		// {
		// 	case KEY_LEFT:
		// }

		for (i = 0; i < pcount; i++){
			ssize_t count = read(filedes[i][0], buffer, sizeof(buffer));
			wprintw(windows[i], buffer);
		}
		refresh();
		sleep(1);

	}
	sleep(10);
}