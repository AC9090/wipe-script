// TO COMPILE gcc -o process-handler process_handler.c -lncurses -lm


#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
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
#define SERIAL_NUMBER "SN"






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
	if (argc < 2){
		printf("Please give arguments in the following format:\n {-clone drive} [drives]\n");
		exit(1);
	}
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
#ifdef TEST
			execl("./pipe_test.sh", "./pipe_test.sh", (char*) NULL);
#else
			if (!clone){
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], (char*)0);
			} else {
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], clone, (char*)0);
			}
#endif			
			perror("execl");
			_exit(1); //dunno why the exit 1
		}
		child_pids[i] = pid;
		close(filedes[i][1]);
		int retval = fcntl( filedes[i][0], F_SETFL, fcntl(filedes[i][0], F_GETFL) | O_NONBLOCK);
        printf("Ret from fcntl: %d\n", retval);
	}

	// The processes have started, Now it's time to start curses.

	char buffer[1024];
	char text[256];
	int rows, cols;

	initscr();

	if(has_colors() == FALSE)
		{	endwin();
			printf("Your terminal does not support color\n");
			exit(1);
		}
	start_color();			/* Start color 			*/

	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);

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
	WINDOW *infoborders[pcount];
	WINDOW *infowins[pcount];

	for(i = 0; i < pcount; i++){
		borderwin[i] = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT), cols/4,
			BORDER_UP + INFO_HT, i * cols/4, true);

		windows[i] = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT - 2), cols/4 - 2,
			BORDER_UP + INFO_HT + 1, i * cols/4 + 1, false);

		scrollok(windows[i], TRUE);
		
		infoborders[i] = create_newwin(INFO_HT, cols/4,
			BORDER_UP, i * cols/4, true);
		
		wattron(infoborders[i], COLOR_PAIR(1));
		box(infoborders[i], 0,0);
		wattroff(infoborders[i], COLOR_PAIR(1));
		wrefresh(infoborders[i]); //remember to update colors.

		infowins[i] = create_newwin(INFO_HT -2 , cols/4 - 2,
			BORDER_UP + 1, i * cols/4 + 1, false);

		mvwprintw(infowins[i], 0,0, "TARGET: /dev/%s\n", targets[i]);
		wrefresh(infowins[i]);
	}

	char status_sn[pcount][64];
	char status_se[pcount][64];
	char status_et[pcount][64];
	char status_er[pcount][64];

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

			if (count <= 0) continue;
			strncpy(text, buffer, count);
			char * line = text;
			char * end = line + count;
			while (line){  //Find all lines in buffer.

				char * nextline = strchr(line, '\n'); // Find the next newline character.
				if (nextline) *nextline = '\0'; // If it exists set it to endline.

				char test[3];
				strncpy(test, line, 2);
				test[2] = '\0';
				// printw("%d",strcmp(test, SECURE_ERASE));

				if (strcmp(test, SECURE_ERASE) == 0){
					strcpy(status_se[i], &line[3]);
					mvwprintw(infowins[i], 1, 0, "SECURE_ERASE: %s\n", status_se[i]);
					wrefresh(infowins[i]);
				} else if (strcmp(test, ESTIMATED_TIME) == 0){
					strcpy(status_et[i], &line[3]);
					mvwprintw(infowins[i], 2, 0, "ESTIMATED_TIME: %s\n", status_et[i]);
					wrefresh(infowins[i]);
				} else if (strcmp(test, SERIAL_NUMBER) == 0){
					strcpy(status_sn[i], &line[3]);
					mvwprintw(infowins[i], 3, 0, "SERIAL_NUM: %s\n", status_sn[i]);
					wrefresh(infowins[i]);
				} else if (strcmp(test, ERROR) == 0){
					wattron(infoborders[i], COLOR_PAIR(2));
					box(infoborders[i], 0,0);
					wattroff(infoborders[i], COLOR_PAIR(2));
					wrefresh(infoborders[i]);

					strcpy(status_er[i], &line[3]);

					wattron(infowins[i], COLOR_PAIR(2));
					mvwprintw(infowins[i], 4, 0, "ERROR: %s\n", status_er[i]);
					wattroff(infowins[i], COLOR_PAIR(2));

					mvwprintw(infowins[i], 0,0, "TARGET: /dev/%s\n", targets[i]);
					mvwprintw(infowins[i], 1, 0, "SECURE_ERASE: %s\n", status_se[i]);
					mvwprintw(infowins[i], 2, 0, "ESTIMATED_TIME: %s\n", status_et[i]);
					mvwprintw(infowins[i], 3, 0, "SERIAL_NUM: %s\n", status_sn[i]);

					wrefresh(infowins[i]);

				} else {
					wprintw(windows[i], "%s\n", line);
					wrefresh(windows[i]);
				}

				line = (nextline && nextline + 1 < end) ? (nextline + 1): NULL; // If there is a next line continue if not break.
				
			}
			// wrefresh(windows[i]);

		}
		time(&current);
		double elapsed = difftime(current,start);
		mvprintw(1,0, "TIME ELAPSED: %0.lf minutes, %0.lf seconds", floor(elapsed / 60l), fmod(elapsed, 60l));
		refresh();

		bool fin = true; // Check if all child processes are finished.
		for (i=0; i < pcount; i ++){
			int stat = 0;
			pid_t return_pid = waitpid(child_pids[i], &stat, WNOHANG);
			if (return_pid == -1){
				exit(1);
			} else if (return_pid == 0) {
				fin = false;
			} else if (return_pid == child_pids[i]){
				wattron(infoborders[i], COLOR_PAIR(3));
				box(infoborders[i], 0,0);
				wattroff(infoborders[i], COLOR_PAIR(3));

				wrefresh(infoborders[i]);

				wattron(infowins[i], COLOR_PAIR(3));
				mvwprintw(infowins[i], 4, 0, "****DONE****\n");
				wattroff(infowins[i], COLOR_PAIR(3));

				mvwprintw(infowins[i], 0,0, "TARGET: /dev/%s\n", targets[i]);
				mvwprintw(infowins[i], 1, 0, "SECURE_ERASE: %s\n", status_se[i]);
				mvwprintw(infowins[i], 2, 0, "ESTIMATED_TIME: %s\n", status_et[i]);
				mvwprintw(infowins[i], 3, 0, "SERIAL_NUM: %s\n", status_sn[i]);

				wrefresh(infowins[i]);


			}
		}
		if (fin) {
			sleep(20);
			break;
		}

		sleep(1);

	}
	endwin();
	printf("Exiting process_handler.\n\n");
	sleep(2);
}