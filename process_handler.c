#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string.h>
#include <time.h>
#include <math.h>

#define MAX_PROCESSES 10

#define NUM_WINDOWS 4


//Definitions for window structuring
#define BORDER_UP 3
#define BORDER_DN 1

#define INFO_HT 8

#define SECURE_ERASE "SE"
#define ERROR "ER"
#define ESTIMATED_TIME "ET"

WINDOW *create_newwin(int height, int width, int starty, int startx, bool border)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	if(border){
		box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	}
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
	


	char* targets[MAX_PROCESSES];
	char* clone;

	int pcount = 0;

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
			while ((dup2(filedes[i][1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
			close(filedes[i][0]);
			close(filedes[i][1]);
			sleep(2); //wait for ncurses to be set up
			// if (!clone){
				// printf("Target %s will be initialized with no clone.\n", targets[i]);
			// } else {
				// printf("Target %s will be initialized with clone %s.\n", targets[i], clone);
			// }
			execl("./pipe_test.sh", "./pipe_test.sh", (char*) NULL);
			perror("execl");
			_exit(1); //dunno why the exit 1
		}
		child_pids[i] = pid;
		close(filedes[i][1]);
	}

	// The processes have started, Now it's time to start curses.

	char buffer[256];
	char text[64];
	int rows, cols;

	initscr();
	getmaxyx(stdscr, rows, cols);
	noecho();
	cbreak(); //typed characers are not buffered
	curs_set(0);
	keypad(stdscr, TRUE);

	int ch; //the character on the keyboard that has been pressed
    refresh();

    mvprintw(rows - 1, 0, "Press CTRL^C to exit (not recommended).");


    WINDOW *borderwin[pcount];
	WINDOW *windows[pcount];
	WINDOW *infowins[pcount];

	for(i = 0; i < pcount; i++){
		borderwin[i] = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT), cols/4,
			BORDER_UP + INFO_HT, i * cols/4, true);

		windows[i] = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT - 2), cols/4 - 2,
			BORDER_UP + INFO_HT + 1, i * cols/4 + 1, false);

		scrollok(windows[i], TRUE);
		
		infowins[i] = create_newwin(INFO_HT, cols/4,
			BORDER_UP, i * cols/4, true);
	}

	mvprintw(0,0, "Wipe-Script Process Handler");

	time_t start, current;

	time(&start);

	while(1){ 

		// switch(ch)
		// {
		// 	case KEY_LEFT:
		// }

		for (i = 0; i < pcount; i++){
			ssize_t count = read(filedes[i][0], buffer, sizeof(buffer));
			strncpy(text, buffer, count);

			char test[3];
			strncpy(test, text, 2);
			test[2] = '\0';
			// printw("%d",strcmp(test, SECURE_ERASE));

			if (strcmp(test, SECURE_ERASE) == 0){
				mvwprintw(infowins[i], 1, 1, "SECURE_ERASE: %s", text);
				wrefresh(infowins[i]);
			} 

			if (count != 0){
				wprintw(windows[i], text);
			}
			wrefresh(windows[i]);

		}
		time(&current);
		double elapsed = difftime(current,start);
		mvprintw(1,0, "TIME ELAPSED: %0.lf minutes, %0.lf seconds", floor(elapsed / 60l), fmod(elapsed, 60l));		refresh();
		sleep(1);

	}
	sleep(10);
}