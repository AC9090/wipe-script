
CC=gcc
CFLAGS=-m32

processhandler: process_handler.c
	$(CC) $(CFLAGS) -o process-handler process_handler.c -lncurses -lm