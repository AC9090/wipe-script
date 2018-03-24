#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <time.h>

void finish_with_error(MYSQL *con)
{
  printf("%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}

void print_to_csv(FILE * f, MYSQL mysql, char * table, MYSQL_RES *result)
{



	unsigned int res_count = mysql_num_rows(result);
	unsigned int num_fields = mysql_num_fields(result);
	unsigned int i;
	MYSQL_ROW row;
	
	if (res_count == 0) {
		printf("No unsynced records were found for table %s\n", table);
		return;
	}

	fprintf(f, "%s\n", table);
	while ((row = mysql_fetch_row(result)))
	{
	   	unsigned long *lengths;
	   	lengths = mysql_fetch_lengths(result);
	   	for(i = 0; i < num_fields - 1; i++)
	   	{
	       fprintf(f, "%.*s|", (int) lengths[i], 
	              row[i] ? row[i] : "NULL");
	       printf("%.*s|", (int) lengths[i], 
	              row[i] ? row[i] : "NULL");
	   	}
       	fprintf(f, "%.*s", (int) lengths[i], 
    		row[i] ? row[i] : "NULL");
	  	printf("%.*s", (int) lengths[i], 
    		row[i] ? row[i] : "NULL");
	  	fprintf(f, "\n");
	  	printf("\n");
	}
	char * query = malloc(sizeof(char) * 512);

	printf("Saved %d rows from %s to file.\n", res_count, table);
	sprintf(query,"UPDATE %s SET  sync_time=CURRENT_TIMESTAMP, synced=1 WHERE synced=0;", table);
	
	if (mysql_query(&mysql, query))
    	finish_with_error(&mysql);


}



int main(int argc, char *argv[])
{
	time_t current;
  	struct tm * timeinfo;
	time(&current);
	
  	timeinfo = localtime(&current);

	char * filename = malloc(sizeof(char) * 128);
	char * temp = asctime(timeinfo);
	temp[strlen(temp) - 1] = '\0';
	sprintf(filename, "WIPEDB MODIFIED %s.csv", temp);
		

	

	MYSQL mysql;


	if(mysql_init(&mysql)==NULL)
	{
		printf("\nFailed to initate MySQL connection");
		exit(1);
	}


	if (!mysql_real_connect(&mysql,"192.168.0.1","wipe","wipepw","wipedb",0,NULL,0))
	{ 

	    printf( "Failed to connect to MySQL: Error: %s\n", mysql_error(&mysql)); 

	    exit(1);

   	}

	FILE *f = fopen(filename, "w");
	if (f == NULL)
	{
	    printf("Error opening file!\n");
	    exit(1);
	}
	printf("Using file %s \n", filename);


	char * query = malloc(sizeof(char) * 512);

	sprintf(query,"SELECT * FROM disk d1 WHERE wiped = (SELECT max(wiped) FROM disk d3 WHERE d3.disk_serial = d1.disk_serial) and synced = 0;");
	
	if (mysql_query(&mysql, query))
    	finish_with_error(&mysql);
  
	MYSQL_RES *result = mysql_store_result(&mysql);
			  
	if (result == NULL)
    	finish_with_error(&mysql);

	print_to_csv(f, mysql, "disk", result);

	sprintf(query, "SELECT * FROM computer WHERE synced = 0");

	if (mysql_query(&mysql, query))
    	finish_with_error(&mysql);
  
	result = mysql_store_result(&mysql);
			  
	if (result == NULL)
    	finish_with_error(&mysql);

	print_to_csv(f, mysql, "computer", result);
	fclose(f);


}