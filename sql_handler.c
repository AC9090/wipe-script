#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>


void expand_escapes(char* dest, const char* src)  //Tyler McHenry on stack exchange.
{
  char c;

  while (c = *(src++)) {
    switch(c) {
      // case '\a': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'a';
      //   break;
      // case '\b': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'b';
      //   break;
      // case '\t': 
      //   *(dest++) = '\\';
      //   *(dest++) = 't';
      //   break;
      // case '\n': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'n';
      //   break;
      // case '\v': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'v';
      //   break;
      // case '\f': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'f';
      //   break;
      // case '\r': 
      //   *(dest++) = '\\';
      //   *(dest++) = 'r';
      //   break;
      // case '\\': 
      //   *(dest++) = '\\';
      //   *(dest++) = '\\';
      //   break;
      case '\"': 
        *(dest++) = '\\';
        *(dest++) = '\"';
        break;
      default:
        *(dest++) = c;
     }
  }

  *dest = '\0'; /* Ensure nul terminator */
}



// These are the possible arguments for this program.
// For a disk: "disk_model" "disk_serial" "disk_size" "security_erase" "enhanced_erase" "source_drive" "parent" "wiped"
// For a computer: "asset_no" "service_tag" "model" "processor"
// -i for insert, -s for select (check)
#define PRINTARGS printf("Please give arguments as:\n -i to insert of -u to upsert \n-c asset_no service_tag model processor for computer) \n -d model serial size security_erase enhanced_erase source_drive(NONE) parent(NONE) (for disk)\n");

void finish_with_error(MYSQL *con)
{
  printf("%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}


int main(int argc, char *argv[])
{
	printf("Sql-handler...\n");

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
   	if(mysql_select_db(&mysql,"wipedb")==0){
   		int mode;
	   	if (argv[1][0] == '-' && argv[1][1] == 'i'){ //insert
	   		mode = 0;
	   	}else if (argv[1][0] == '-' && argv[1][1] == 's'){ //select
	   		mode = 1;
	   	} else if (argv[1][0] == '-' && argv[1][1] == 'u'){ //update
	   		mode = 2;
	   	} else {
	   		PRINTARGS
	   		exit(1);
	   	}

	   	if (argv[2][0] == '-' && argv[2][1] == 'd'){
	   		printf ("Database 'disk' selected.\n");

			int i;
	   		char * values = malloc(sizeof(char) * 512);
	   		char * keys = malloc(sizeof(char) * 512);
	   		char * disk_serial = malloc(sizeof(char) * 64);
	   		values[0] = '\0';
	   		keys[0] = '\0';

	   		char * temp = malloc(sizeof(char) * 128);
	   		for (i = 3; i < argc; i ++) {
	   			expand_escapes(temp, argv[i]);

	   			if(!strncmp("disk_model", temp, strlen("disk_model"))){
	   				strcat(keys, "disk_model");
	   			} else if (!strncmp("disk_serial", temp, strlen("disk_serial"))){
	   				strcat(keys, "disk_serial");
	   				// char * token = strtok(temp, "=");
	   				// token = strtok(NULL, "=");
	   				//strcpy(disk_serial, token);
	   			} else if (!strncmp("disk_size", temp, strlen("disk_size"))){
	   				strcat(keys, "disk_size");
	   			} else if (!strncmp("security_erase", temp, strlen("security_erase"))){
	   				strcat(keys, "security_erase");
	   			} else if (!strncmp("enhanced_erase", temp, strlen("enhanced_erase"))){
	   				strcat(keys, "enhanced_erase");
	   			} else if (!strncmp("source_drive", temp, strlen("source_drive"))){
	   				strcat(keys, "source_drive");
	   			} else if (!strncmp("parent", temp, strlen("parent"))){
	   				strcat(keys, "parent");
   				} else if (!strncmp("wiped", temp, strlen("wiped"))){
   					strcat(keys, "wiped");
	   			} else {
	   				printf("Error, argument not supported: %s\n", temp);
	   				PRINTARGS
	   				exit(1);
	   			}

	   			char * token = strtok(temp, "=");
	   			token = strtok(NULL, "=");
	   			strcat(values, "\"");
   				strcat(values, token);
	   			strcat(values, "\"");

   				if (i != argc - 1){
   					strcat(values, ", ");
   					strcat(keys, ", ");
   				}
	   				
	   		}
	  //  		sprintf(query,"SELECT * FROM disk WHERE disk_serial=\"%s\");",
		 //    		disk_serial);

	   		
	  //  		if (mysql_query(&mysql, query))
			// {
			//     finish_with_error(&mysql);
			// }
			  
			// MYSQL_RES *result = mysql_store_result(&mysql);
			  
			// if (result == NULL) 
			// {
			//     finish_with_error(&mysql);
			// }


		    sprintf(query,"INSERT INTO disk (%s) VALUES(%s);",
		    		keys, values);


	    	if (mysql_query(&mysql, query))
     	 		finish_with_error(&mysql);
			
		} else if (argv[2][0] == '-' && argv[2][1] == 'c') {
	    	printf( "Database 'computer' Selected\n");

		    int i;
	   		char * values = malloc(sizeof(char) * 512);
	   		char * keys = malloc(sizeof(char) * 512);
	   		values[0] = '\0';
	   		keys[0] = '\0';

	   		char * temp = malloc(sizeof(char) * 128);
	   		for (i = 3; i < argc; i ++) {
	   			expand_escapes(temp, argv[i]);

	   			if(!strncmp("asset_no", temp, strlen("asset_no"))){
	   				strcat(keys, "asset_no");
	   			} else if (!strncmp("service_tag", temp, strlen("service_tag"))){
	   				strcat(keys, "service_tag");
	   			} else if (!strncmp("model", temp, strlen("model"))){
	   				strcat(keys, "model");
	   			} else if (!strncmp("processor", temp, strlen("processor"))){
	   				strcat(keys, "processor");
	   			} else {
	   				PRINTARGS
	   				printf("%s\n", temp);
	   				exit(1);
	   			}
	   			char * token = strtok(temp, "=");
	   			token = strtok(NULL, "=");
	   			strcat(values, "\"");
   				strcat(values, token);
	   			strcat(values, "\"");

   				if (i != argc - 1){
   					strcat(values, ", ");
   					strcat(keys, ", ");
   				}
	   				
	   		}
		    sprintf(query,"INSERT INTO computer (%s) VALUES(%s);",
		    		keys, values);

		    printf("%s\n", query);
		    	if (mysql_query(&mysql, query))
	     	 		finish_with_error(&mysql);

		} else {
			PRINTARGS
		}
	

	}else
    	printf( "Failed to connect to Database: Error: %s\n", mysql_error(&mysql));

	printf("Done\n");
	mysql_close(&mysql);

	exit(0);


}