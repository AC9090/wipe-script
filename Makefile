
CC=gcc
CFLAGS=-m32 -D TEST

processhandler: process_handler.c
	$(CC) $(CFLAGS) -o process-handler process_handler.c -lncurses -lm

sqlhandler: sql_handler.c
	$(CC) $(CFLAGS) -o sql-handler sql_handler.c -lmysqlclient