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
#include <mysql.h>

#define MAX_PROCESSES 10

#define MAX_WINS 4


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

#define PRINT_ER(iw, er) mvwprintw(iw, 4, 0, "ERR: %s     \n", er)
#define PRINT_DONE(iw) mvwprintw(iw, 4, 0, "*****DONE*****          \n")


#define STATUS_RUNNING 0
#define STATUS_DONE 1
#define STATUS_ERROR 2

#define REFRESH_SELECTED 0b001
#define REFRESH_PAD 0b010
#define REFRESH_INFO_BORDER 0b100

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
long elapsed;

void draw_proc(WipeWIN *w, WipeStatus *s, bool selected, int refresh_flag)
{
	if (s->status == STATUS_RUNNING){
		wattron(w->infoborder, COLOR_PAIR(1));
		box(w->infoborder, 0,0);
		wattroff(w->infoborder, COLOR_PAIR(1));


		if (elapsed > s->est_time){
			mvwprintw(w->infowin, 4,0, "T: +%02.lf:%02.lf:%02.lf      ", floor((elapsed - s->est_time) / (60l * 60l)),
				floor(fmod((elapsed - s->est_time)/60l, 60l)), fmod(elapsed - s->est_time, 60l));
			//if (clone)
			//	wstat[i].pbar[5] = "CLONING!";
		} else {
			mvwprintw(w->infowin, 4,0, "T: -%02.lf:%02.lf:%02.lf      ", floor((s->est_time - elapsed) / (60l * 60l)),
				floor(fmod((s->est_time - elapsed)/60l, 60l)), fmod(s->est_time - elapsed, 60l));

			if (((float)elapsed / (float)s->est_time) > ((s->progress + 1) / (cols/MAX_WINS - 3))){
				s->pbar[(int)s->progress+1] = '=';
				s->progress ++;

			}

		}
	} else if (s->status == STATUS_DONE){
		wattron(w->infoborder, COLOR_PAIR(3));
		box(w->infoborder, 0,0);
		wattroff(w->infoborder, COLOR_PAIR(3));

		wattron(w->infowin, COLOR_PAIR(3));
		PRINT_DONE(w->infowin);
		wattroff(w->infowin, COLOR_PAIR(3));
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
	if (refresh_flag & REFRESH_INFO_BORDER)
		wrefresh(w->infoborder);

	if (selected){
		wattron(w->borderwin, COLOR_PAIR(5));
		box(w->borderwin, 0,0);
		wattron(w->borderwin, COLOR_PAIR(5));
	} else {
		wattron(w->borderwin, COLOR_PAIR(6));
		box(w->borderwin, 0,0);
		wattron(w->borderwin, COLOR_PAIR(6));
	}

	if (refresh_flag & REFRESH_SELECTED)
		wrefresh(w->borderwin);

	PRINT_TARGET(w->infowin, s->target);
	PRINT_SE(w->infowin, s->status_se);
	PRINT_SN(w->infowin, s->status_sn);
	PRINT_ET(w->infowin, s->status_et);

	wattron(w->infowin, COLOR_PAIR(4));	wattron(w->infowin, A_BOLD);
	mvwprintw(w->infowin, 5,0, s->pbar);
	wattroff(w->infowin, A_BOLD);	wattroff(w->infowin, COLOR_PAIR(4));

	wrefresh(w->infowin);

	if (refresh_flag & (REFRESH_PAD | REFRESH_SELECTED) )
		prefresh(s->padwin, s->pad_scroll - (rows - BORDER_UP - BORDER_DN - INFO_HT - 2), 0,  BORDER_UP + INFO_HT + 1, 
			(w->i) * cols/MAX_WINS + 1, (rows - BORDER_DN  - 2), (w->i + 1) * cols/MAX_WINS - 1);


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
	char* clone = NULL;
	char* parent = NULL;

	int pcount = 0;
	printf("argc %d, argv %s", argc, argv[1]);
	if (argc < 2){
		printf("Please give arguments in the following format:\n {-c clone_drive} {-p parent_computer} [drives]\n");
		exit(1);
	}
	int i = 1;
	for (i; i < argc; i ++){
		if ((argv[i][0] == '-') && (argv[i][1] == 'c')){ //this indicates a clone
			i++;
			clone = &argv[i][0];
		} else if ((argv[i][0] == '-') && (argv[i][1] == 'p')){
			i++;
			parent = &argv[i][0];
		}else {
			targets[pcount] = argv[i];
			pcount ++;
		}
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
			if ((!clone) && (!parent)){
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], "NONE", "NONE",(char*)0);
			} else if (!parent){
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], clone, "NONE", (char*)0);
			} else if (!clone){
				execl("./wipe-worker.sh", "./wipe-worker.sh", targets[i], "NONE", parent,(char*)0);
			} else {
				printf("something went wrong....\n");
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

	char *proc_status;

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
	cbreak(); // Input characers are not shown on screen.
	curs_set(0);
	keypad(stdscr, TRUE);

    refresh();

    mvprintw(rows - 1, 0, "Press CTRL^C to exit (not recommended).");



    WipeWIN *wwin;


    int num_wins = (pcount < MAX_WINS) ? pcount : MAX_WINS;
    wwin = malloc(num_wins * sizeof(WipeWIN));

	for(i = 0; i < num_wins; i++){ // Initialize the wipe window structures.
		wwin[i].i = i;

		wwin[i].borderwin = create_newwin((rows - BORDER_UP - BORDER_DN - INFO_HT), cols/MAX_WINS,
			BORDER_UP + INFO_HT, i * cols/MAX_WINS, true);

		wwin[i].infoborder = create_newwin(INFO_HT, cols/MAX_WINS,
			BORDER_UP, i * cols/MAX_WINS, true);
		
		wattron(wwin[i].infoborder, COLOR_PAIR(1));
		box(wwin[i].infoborder, 0,0);
		wattroff(wwin[i].infoborder, COLOR_PAIR(1));
		wrefresh(wwin[i].infoborder); //remember to update colors.

		wwin[i].infowin = create_newwin(INFO_HT -2 , cols/MAX_WINS - 2,
			BORDER_UP + 1, i * cols/MAX_WINS + 1, false);

		mvwprintw(wwin[i].infowin, 0,0, "TARGET: /dev/%s\n", targets[i]);
		wrefresh(wwin[i].infowin);
	}

	WipeStatus *wstat;

	wstat = malloc(pcount * sizeof(WipeStatus));

	for (i = 0; i < pcount; i++){ // Initialize the wipe status structure.
		wstat[i].pbar =  malloc(sizeof(char) * (cols/MAX_WINS -1));
		strcpy(wstat[i].target, targets[i]);
		wstat[i].padwin = newpad(1000, cols/MAX_WINS - 2);
		scrollok(wstat[i].padwin, TRUE);
		wstat[i].pad_scroll = 0;
		wstat[i].est_time = 0;
		wstat[i].status_pid = 0;
		wstat[i].progress = 0;
		wstat[i].status = STATUS_RUNNING;
		wstat[i].pbar[0] = '[';
		wstat[i].pbar[cols/MAX_WINS -3] = ']';
		wstat[i].pbar[cols/MAX_WINS - 2] = '\0';
		int j;
		for (j = 1; j < cols/MAX_WINS -3; j ++ )
			wstat[i].pbar[j]=' ';
	}

	mvprintw(0,0, "Wipe-Script Process Handler");
	time_t start, current;

	time(&start);

	int ch,y,x;  // The character to be taken for input and the x and y coordinates of the cursor.
	int wsel =0; // The currently selected / highlighted window.
	int wshow = 0; // The first window shown on screen.
	bool fin = false; // All processes are finished.
	int r_flag; // Which parts of the screen need refreshed to reduce flickering.
	while(1){
		r_flag = 0;

		time(&current);
		elapsed = difftime(current,start);
		mvprintw(1,0, "TIME ELAPSED: %02.lf hr %02.lf min %02.lf sec       ", floor(elapsed/(60l * 60l)),
			fmod(floor((elapsed)/60l), 60l), fmod(elapsed, 60l));

		// Print a diamond with a color corresponding to the status of the subprocess.
		for(i = 0; i < pcount; i ++){
			if (wstat[i].status == STATUS_RUNNING){
				attron(COLOR_PAIR(1));
				mvaddch(2,2*i, ACS_DIAMOND);
				attroff(COLOR_PAIR(1));
			} else if (wstat[i].status == STATUS_ERROR){
				attron(COLOR_PAIR(2));
				mvaddch(2,2*i, ACS_DIAMOND);
				attroff(COLOR_PAIR(2));
			} else if (wstat[i].status == STATUS_DONE){
				attron(COLOR_PAIR(3));
				mvaddch(2,2*i, ACS_DIAMOND);
				attroff(COLOR_PAIR(3));
			}
		}

		refresh();


		ch = getch(); // Get the next character from the input buffer.
		switch(ch) // Choose what to do based on the character input.
		{

			case KEY_HOME: // If all subprocesses are finished, press home to continue.
				if (fin) {
					
					sleep(1);
					goto finish;
				}
				break;

			case KEY_LEFT: // Move one window to the left.

				if (wsel > 0 && wsel > wshow){
					wsel --;
				} else if (wsel > 0 && wsel <= wshow){
					wsel --;
					wshow = wsel;
				}else {
					wsel += pcount -1;
					wshow = pcount - num_wins;
				}
				r_flag |= REFRESH_SELECTED | REFRESH_INFO_BORDER;

				break;
			
			case KEY_RIGHT: // Move one window to the right.

				if (wsel < pcount -1 && wsel < wshow + num_wins - 1){
					wsel ++;
				} else if (wsel < pcount - 1 && wsel >= wshow + num_wins - 1){
					wsel ++;
					wshow ++;
				} else {
					wsel = 0;
					wshow = 0;
				}

				r_flag |= REFRESH_SELECTED | REFRESH_INFO_BORDER;


				break;

			case KEY_UP:  // Scroll up within the pad of the selected window.
				if (wstat[wsel].pad_scroll > 0)
					wstat[wsel].pad_scroll --;

				r_flag |= REFRESH_PAD;
				break;

			case KEY_DOWN:

				getyx(wstat[wsel].padwin,y,x);
				if(wstat[wsel].pad_scroll < y )
					wstat[wsel].pad_scroll ++;

				r_flag |= REFRESH_PAD;
				break;

			default:
				break;
		}


		for (i = 0; i < pcount; i++){ // Loop through each subprocess.

			ssize_t count = read(filedes[i][0], buffer, sizeof(buffer));

			if (count > 0 && wstat[i].status == STATUS_RUNNING ){  // If the pipe buffer has data and the process is running.
				strncpy(text, buffer, count);
				char * line = text;
				char * end = line + count;
				while (line){  //Find all lines in buffer.

					char * nextline = strchr(line, '\n'); // Find the next newline character.
					if (nextline) *nextline = '\0'; // If it exists set it to endline.

					char test[3];
					strncpy(test, line, 2); // Special inputs are tagged by the first two characters.
					test[2] = '\0';

					// Test for all the special characters.
					if (strcmp(test, SECURE_ERASE) == 0){
						strcpy(wstat[i].status_se, &line[3]);
					} else if (strcmp(test, ESTIMATED_TIME) == 0){
						strcpy(wstat[i].status_et, &line[3]);

						char temp[5];
						int s = 0;
						while((wstat[i].status_et[s] <= '9' && wstat[i].status_et[s] >= '0' )){
							s++;		
						}

						if (s > 0){	
							strncpy(temp ,wstat[i].status_et, s);
							temp[s] = '\0';
							wstat[i].est_time = (long) atoi(temp) * 60;
						} else {
							wstat[i].est_time = 0;
						}

					} else if (strcmp(test, SERIAL_NUMBER) == 0){
						strcpy(wstat[i].status_sn, &line[3]);
					} else if (strcmp(test, ERROR) == 0){
						strcpy(wstat[i].status_er, &line[3]);
						wstat[i].status = STATUS_ERROR;

						r_flag |= REFRESH_INFO_BORDER;


					} else {
						wprintw(wstat[i].padwin, "%s\n", line);
						getyx(wstat[i].padwin,y,x);
						wstat[i].pad_scroll = y;

						r_flag |= REFRESH_PAD;

					}

					line = (nextline && nextline + 1 < end) ? (nextline + 1): NULL; // If there is a next line continue if not break.
					
				}
			}

			// if (fmod(s->est_time - elapsed, 60l)) == 0) // To be tested
			// 	r_flag = r_flag | REFRESH_PAD | REFRESH_INFO_BORDER | REFRESH_SELECTED;

			if (i >= wshow && i < wshow + num_wins)
				draw_proc(&wwin[i - wshow], &wstat[i], (i == wsel) ?true:false, r_flag);
 

		}

		fin = true; // Check if all child processes are finished.
		for (i=0; i < pcount; i ++){
			if (wstat[i].status == STATUS_DONE)
				continue;

			int stat = 0;
			if (wstat[i].status_pid == 0){
				wstat[i].status_pid = waitpid(child_pids[i], &stat, WNOHANG);
			}
			if (wstat[i].status_pid == -1){
				mvprintw(0,0, "return_pid == -1");
				refresh();
				sleep(10);
			} else if (wstat[i].status_pid == 0) {
				fin = false;
			} else if (wstat[i].status_pid == child_pids[i]){

				if (!(wstat[i].status == STATUS_ERROR))
					wstat[i].status = STATUS_DONE;

				// Update the info window one final time.
				if (i >= wshow && i < wshow + num_wins)
					draw_proc(&(wwin[i - wshow]), &(wstat[i]), (i == wsel) ?true:false, 0b111); 
			}
		}
		if (fin){
			attron(COLOR_PAIR(3));
			mvprintw(rows - 1, 0, "All processes are complete! Press HOME to continue...");
			attroff(COLOR_PAIR(3));
			refresh();
		}

		usleep(25000);
		// sleep(1);

	}
	finish:
	erase();
	endwin();
	printf("Exiting process_handler.\n\n");
	sleep(2);
	exit(0);
}
