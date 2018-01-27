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

#define PRINT_TARGET(iw, target) mvwprintw(iw, 0,0, "TARGET: /dev/%s\n", target)
#define PRINT_SE(iw, se) mvwprintw(iw, 1, 0, "SECURE_ERASE: %s\n", se)
#define PRINT_SN(iw, sn) mvwprintw(iw, 2, 0, "SERIAL: %s\n", sn)
#define PRINT_ET(iw, et) mvwprintw(iw, 3, 0, "ESTIMATED_TIME: %s\n", et)

#define PRINT_ER(iw, er) mvwprintw(iw, 5, 0, "ERROR: %s\n", er)

#define STATUS_RUNNING 0
#define STATUS_DONE 1
#define STATUS_ERROR 2

typedef struct WipeStatus WipeStatus;

struct WipeStatus {
	char target[8];
	char status_sn[64];
	char status_se[64];
	char status_et[64];
	char status_er[64];
	int status_pid;
	int status;
	long est_time;
	char *pbar;
	float progress;

	WINDOW *padwin;
	int pad_scroll;


};

typedef struct WipeWIN WipeWIN;

struct WipeWIN {
	int i;
	WINDOW *borderwin;
	WINDOW *infoborder;
	WINDOW *infowin;
};


int rows, cols;


void draw_proc(WipeWIN *w, WipeStatus *s, long elapsed)
{
	if (s->status == STATUS_RUNNING){
		wattron(w->infoborder, COLOR_PAIR(1));
		box(w->infoborder, 0,0);
		wattroff(w->infoborder, COLOR_PAIR(1));
	} else if (s->status == STATUS_DONE){
		wattron(w->infoborder, COLOR_PAIR(3));
		box(w->infoborder, 0,0);
		wattroff(w->infoborder, COLOR_PAIR(3));
	} else if (s->status == STATUS_ERROR) {
		wattron(w->infoborder, COLOR_PAIR(2));
		box(w->infoborder, 0,0);
		wattroff(w->infoborder, COLOR_PAIR(2));

		wattron(w->infowin, COLOR_PAIR(2));
		PRINT_ER(w->infowin, s->status_er);
		wattroff(w->infowin, COLOR_PAIR(2));

	} else {
		//Problem.
	}

	PRINT_TARGET(w->infowin, s->target);
	PRINT_SE(w->infowin, s->status_se);
	PRINT_SN(w->infowin, s->status_sn);
	PRINT_ET(w->infowin, s->status_et);




	if (elapsed > s->est_time){
		mvwprintw(w->infowin, 4,0, "T: +%02.lf:%02.lf:%02.lf      ", floor((elapsed - s->est_time) / (60l * 60l)),
			floor((elapsed - s->est_time)/60l), fmod(elapsed - s->est_time, 60l));
		//if (clone)
		//	pbars[i][5] = "CLONING!";
	} else {
		mvwprintw(w->infowin, 4,0, "T: -%02.lf:%02.lf:%02.lf      ", floor((s->est_time - elapsed) / (60l * 60l)),
			floor(fmod((s->est_time - elapsed)/60l, 60l)), fmod(s->est_time - elapsed, 60l));

		if (((float)elapsed / (float)s->est_time) > ((s->progress + 1) / (cols/4 - 3))){
			s->pbar[(int)s->progress+1] = '=';
			s->progress ++;

		}

	}
	wattron(w->infowin, COLOR_PAIR(4));	wattron(w->infowin, A_BOLD);
	mvwprintw(w->infowin, 5,0, s->pbar);
	wattroff(w->infowin, A_BOLD);	wattroff(w->infowin, COLOR_PAIR(4));

	wrefresh(w->infowin);


	wrefresh(w->infowin);


	prefresh(s->padwin, s->pad_scroll - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, w->i * cols/4 + 1,
					(rows - BORDER_DN  - 2), (w->i + 1) * cols/4 - 1);
}


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
			if (i == 1){
				execl("./pipe_test2.sh", "./pipe_test.sh", (char*) NULL);				
			} else {
				execl("./pipe_test.sh", "./pipe_test.sh", (char*) NULL);
			}
#else
			if (!clone){
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], (char*)0);
			} else {
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], clone, (char*)0);
			}
#endif			
			perror("execl");
			//exit(0);
			_exit(1); //dunno why the exit 1
		}
		child_pids[i] = pid;
		close(filedes[i][1]);
		int retval = fcntl( filedes[i][0], F_SETFL, fcntl(filedes[i][0], F_GETFL) | O_NONBLOCK);
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
	init_pair(4, COLOR_YELLOW, COLOR_BLUE);
	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(6, COLOR_WHITE, COLOR_BLACK);

	getmaxyx(stdscr, rows, cols);
	timeout(0);
	noecho();
	cbreak(); //typed characers are not buffered
	curs_set(0);
	keypad(stdscr, TRUE);

    refresh();

    mvprintw(rows - 1, 0, "Press CTRL^C to exit (not recommended).");



    WipeWIN *wwin;

    wwin = malloc(NUM_WINDOWS * sizeof(WipeWIN));

    int a = (pcount < NUM_WINDOWS) ? pcount : NUM_WINDOWS;

	for(i = 0; i < a; i++){
		wwin[i].i = i;

		wwin[i].borderwin = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT), cols/4,
			BORDER_UP + INFO_HT, i * cols/4, true);

		wwin[i].infoborder = create_newwin(INFO_HT, cols/4,
			BORDER_UP, i * cols/4, true);
		
		wattron(wwin[i].infoborder, COLOR_PAIR(1));
		box(wwin[i].infoborder, 0,0);
		wattroff(wwin[i].infoborder, COLOR_PAIR(1));
		wrefresh(wwin[i].infoborder); //remember to update colors.

		wwin[i].infowin = create_newwin(INFO_HT -2 , cols/4 - 2,
			BORDER_UP + 1, i * cols/4 + 1, false);

		mvwprintw(wwin[i].infowin, 0,0, "TARGET: /dev/%s\n", targets[i]);
		wrefresh(wwin[i].infowin);
	}

	char (*status_sn)[64];
	status_sn = malloc(pcount * sizeof(char[64]));
	char (*status_se)[64];
	status_se = malloc(pcount * sizeof(char[64]));
	char (*status_et)[64];
	status_et = malloc(pcount * sizeof(char[64]));
	char (*status_er)[64];
	status_er = malloc(pcount * sizeof(char[64]));

	long (*est_time);
	est_time = malloc(pcount * sizeof(long));

	int *status_pid;
	status_pid = malloc(sizeof(int) * pcount);


	char (*pbars)[cols/4 -2];
	pbars = malloc(pcount * sizeof(char) * (cols/4 -1));
	float *progress = malloc(pcount * sizeof(float));

	int *pad_scroll;
	WINDOW *padwins[pcount];
	pad_scroll = malloc(pcount * sizeof(int));

	int j;
	for (j = 0; j < pcount; j++){

		padwins[j] = newpad(1000, cols/4 - 2);
		scrollok(padwins[j], TRUE);
				

		est_time[j] = 0;
		status_pid[j] = 0;
		progress[j] = 0;
		pbars[j][0] = '[';
		pbars[j][cols/4 -3] = ']';
		pbars[j][cols/4 - 2] = '\0';
		for (i = 1; i < cols/4 -3; i ++ )
			pbars[j][i]=' ';
	}

	mvprintw(0,0, "Wipe-Script Process Handler");
	time_t start, current;

	time(&start);

	int ch, wsel,y,x;
	wsel =0;
	while(1){ 


		time(&current);
		double elapsed = difftime(current,start);
		mvprintw(1,0, "TIME ELAPSED: %02.lf hr %02.lf min %02.lf sec       ", floor(elapsed/(60l * 60l)),
			floor((elapsed)/60l), fmod(elapsed, 60l));
		refresh();


		ch = getch();
		switch(ch)
		{
			case KEY_LEFT:
				wattron(wwin[wsel].borderwin, COLOR_PAIR(6));
				box(wwin[wsel].borderwin, 0,0);
				wattron(wwin[wsel].borderwin, COLOR_PAIR(6));
				wrefresh(wwin[wsel].borderwin);

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
					(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);

				if (wsel > 0){
					wsel --;
				} else {
					wsel += pcount -1;
				}

				wattron(wwin[wsel].borderwin, COLOR_PAIR(5));
				box(wwin[wsel].borderwin, 0,0);
				wattron(wwin[wsel].borderwin, COLOR_PAIR(5));
				wrefresh(wwin[wsel].borderwin);

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
					(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);

				break;
			
			case KEY_RIGHT:
				wattron(wwin[wsel].borderwin, COLOR_PAIR(6));
				box(wwin[wsel].borderwin, 0,0);
				wattron(wwin[wsel].borderwin, COLOR_PAIR(6));
				wrefresh(wwin[wsel].borderwin);

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
					(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);

				if (wsel < pcount -1){
					wsel ++;
				} else {
					wsel = 0;
				}

				wattron(wwin[wsel].borderwin, COLOR_PAIR(5));
				box(wwin[wsel].borderwin, 0,0);
				wattron(wwin[wsel].borderwin, COLOR_PAIR(5));
				wrefresh(wwin[wsel].borderwin);

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
					(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);


				break;

			case KEY_UP:
				if (pad_scroll[wsel] > 0)
					pad_scroll[wsel] --;

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
					(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);

				break;

			case KEY_DOWN:

				getyx(padwins[wsel],y,x);
				if(pad_scroll[wsel] < y )
					pad_scroll[wsel] ++;

				prefresh(padwins[wsel], pad_scroll[wsel] - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, wsel * cols/4 + 1,
									(rows - BORDER_DN  - 2), (wsel+1) * cols/4 - 1);

				break;

			default:
				break;
		}


		for (i = 0; i < pcount; i++){ // Loop through each subprocess.

			if (elapsed > est_time[i]){
				mvwprintw(wwin[i].infowin, 4,0, "T: +%02.lf:%02.lf:%02.lf      ", floor((elapsed - est_time[i]) / (60l * 60l)),
					floor((elapsed - est_time[i])/60l), fmod(elapsed - est_time[i], 60l));
				//if (clone)
				//	pbars[i][5] = "CLONING!";
			} else {
				mvwprintw(wwin[i].infowin, 4,0, "T: -%02.lf:%02.lf:%02.lf      ", floor((est_time[i] - elapsed) / (60l * 60l)),
					floor(fmod((est_time[i] - elapsed)/60l, 60l)), fmod(est_time[i] - elapsed, 60l));

				if (((float)elapsed / (float)est_time[i]) > ((progress[i] + 1) / (cols/4 - 3))){
					pbars[i][(int)progress[i]+1] = '=';
					progress[i] ++;

				}

			}
			wattron(wwin[i].infowin, COLOR_PAIR(4));	wattron(wwin[i].infowin, A_BOLD);
			mvwprintw(wwin[i].infowin, 5,0, pbars[i]);
			wattroff(wwin[i].infowin, A_BOLD);	wattroff(wwin[i].infowin, COLOR_PAIR(4));

			wrefresh(wwin[i].infowin);

			// Process information passed through each pipe.

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
					PRINT_SE(wwin[i].infowin, status_se[i]);
					wrefresh(wwin[i].infowin);
				} else if (strcmp(test, ESTIMATED_TIME) == 0){
					strcpy(status_et[i], &line[3]);
					PRINT_ET(wwin[i].infowin, status_et[i]);
					wrefresh(wwin[i].infowin);

					char temp[5];
					int s = 0;
					while((status_et[i][s] <= '9' && status_et[i][s] >= '0' )){
						s++;		
					}

					if (s > 0){	
						strncpy(temp ,status_et[i], s);
						temp[s] = '\0';
						est_time[i] = (long) atoi(temp) * 60;
					} else {
						est_time[i] = 0;
					}

				} else if (strcmp(test, SERIAL_NUMBER) == 0){
					strcpy(status_sn[i], &line[3]);
					PRINT_SN(wwin[i].infowin, status_sn[i]);
					wrefresh(wwin[i].infowin);
				} else if (strcmp(test, ERROR) == 0){
					wattron(wwin[i].infoborder, COLOR_PAIR(2));
					box(wwin[i].infoborder, 0,0);
					wattroff(wwin[i].infoborder, COLOR_PAIR(2));
					wrefresh(wwin[i].infoborder);

					strcpy(status_er[i], &line[3]);

					wattron(wwin[i].infowin, COLOR_PAIR(2));
					PRINT_ER(wwin[i].infowin, status_er[i]);
					wattroff(wwin[i].infowin, COLOR_PAIR(2));

					PRINT_TARGET(wwin[i].infowin, targets[i]);
					PRINT_SE(wwin[i].infowin, status_se[i]);
					PRINT_SN(wwin[i].infowin, status_sn[i]);
					PRINT_ET(wwin[i].infowin, status_et[i]);

					wrefresh(wwin[i].infowin);

				} else {
					wprintw(padwins[i], "%s\n", line);
					getyx(padwins[i],y,x);
					pad_scroll[i] = y;
					prefresh(padwins[i], y - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, i * cols/4 + 1,
					(rows - BORDER_DN  - 2), (i+1) * cols/4 - 1);
					// wrefresh(windows[i]);

				}

				line = (nextline && nextline + 1 < end) ? (nextline + 1): NULL; // If there is a next line continue if not break.
				
			}
			// wrefresh(windows[i]);

		}

		bool fin = true; // Check if all child processes are finished.
		for (i=0; i < pcount; i ++){
			int stat = 0;
			if (status_pid[i] == 0){
				status_pid[i] = waitpid(child_pids[i], &stat, WNOHANG);
			}
			if (status_pid[i] == -1){
				mvprintw(0,0, "return_pid == -1");
				refresh();
				sleep(10);
			} else if (status_pid[i] == 0) {
				fin = false;
			} else if (status_pid[i] == child_pids[i]){
				wattron(wwin[i].infoborder, COLOR_PAIR(3));
				box(wwin[i].infoborder, 0,0);
				wattroff(wwin[i].infoborder, COLOR_PAIR(3));

				wrefresh(wwin[i].infoborder);

				wattron(wwin[i].infowin, COLOR_PAIR(3));
				mvwprintw(wwin[i].infowin, 5, 0, "****DONE****\n");
				wattroff(wwin[i].infowin, COLOR_PAIR(3));

				PRINT_TARGET(wwin[i].infowin, targets[i]);
				PRINT_SE(wwin[i].infowin, status_se[i]);
				PRINT_SN(wwin[i].infowin, status_sn[i]);
				PRINT_ET(wwin[i].infowin, status_et[i]);

				wrefresh(wwin[i].infowin);


			}
		}
		if (fin) {
			
			attron(COLOR_PAIR(3));
			mvprintw(rows - 1, 0, "All processes are complete! Press any key to continue...");
			attroff(COLOR_PAIR(3));
			refresh();
			fseek(stdin,0,SEEK_END); // Empty stdin.
			sleep(1);
			getchar();
			break;
		}

		usleep(10000);

	}
	endwin();
	printf("Exiting process_handler.\n\n");
	sleep(2);
	exit(0);
}