#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>

#define PRINTARGS printf("Please give arguments as:\n -c asset_no service_tag model processor for computer) \n -d model serial size security_erase enhanced_erase source_drive(NONE) parent(NONE) (for disk)\n");

void finish_with_error(MYSQL *con)
{
  printf("%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}


int main(int argc, char *argv[])
{
	printf("Begining...\n");

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
   	if (strlen(argv[1]) < 2)
   		PRINTARGS

   	char * query = (char*) malloc(512 * sizeof(char));

   	if (argv[1][0] == '-' && argv[1][1] == 'd'){
	   	if(mysql_select_db(&mysql,"wipedb")==0){
	    	printf( "Database 'disk' Selected\n");
	    	if (argc < 9)
	    		PRINTARGS
	    	sprintf(query,"INSERT INTO disk VALUES('%s','%s','%s','%s','%s','%s','%s')",
	    		argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8]);
	    	if (mysql_query(&mysql, query))
     	 		finish_with_error(&mysql);
		}else
	    	printf( "Failed to connect to Database: Error: %s\n", mysql_error(&mysql));
	} else if (argv[1][0] == '-' && argv[1][1] == 'c') {
		if(mysql_select_db(&mysql,"wipedb")==0){
	    	printf( "Database 'computer' Selected\n");

	    	if (argc < 6)
	    		PRINTARGS

	    	 sprintf(query, "INSERT INTO computer VALUES('%s','%s','%s','%s')",
	    		argv[2],argv[3],argv[4],argv[5]);
	    	if (mysql_query(&mysql, query))
     	 		finish_with_error(&mysql);
		}
		else
	    	printf( "Failed to connect to Database to select: Error: %s\n", mysql_error(&mysql));
	} else {
		PRINTARGS
	}
	printf("Done\n");
	mysql_close(&mysql);

	exit(0);


}