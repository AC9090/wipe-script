#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>

#define SERVER_IP "192.168.0.1"

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


// Set key vaule pair from source that is of format key=value.
void set_kvp(char* key, char* value, const char* src)
{
	char * temp = malloc(sizeof(char) * 128);
	strcpy(temp, src);
	char * token = strtok(temp, "=");
	strcpy(key, token);
	token = strtok(NULL, "=");
	if (! token){
		strcat(value, "NULL");

	} else {
		strcat(value, "\"");
		strcat(value, token);
		strcat(value, "\"");
		
	}

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


	if (!mysql_real_connect(&mysql,SERVER_IP, "wipe","wipepw","wipedb",0,NULL,0))
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
	   	} else if (argv[1][0] == '-' && argv[1][1] == 'u'){ //upsert
	   		mode = 2;
	   	} else {
	   		PRINTARGS
	   		exit(1);
	   	}



	    int i;
   		char ** values = malloc(sizeof(char * ) * (argc-3)); 
   		char ** keys = malloc(sizeof(char * ) * (argc-3));
   		for (i = 0; i < argc - 3; i ++){
   			values[i] = malloc(sizeof(char) * 64);
   			values[i][0] = '\0';
   			keys[i] = malloc(sizeof(char) * 64);
   			keys[i][0] = '\0';
   			
   		}

	   	if (argv[2][0] == '-' && argv[2][1] == 'd'){
	   		printf ("Database 'disk' selected.\n");

	   		char * disk_serial = malloc(sizeof(char) * 128); // Keep track of the primary key for use later.

	   		char * temp = malloc(sizeof(char) * 128);
	   		for (i = 0; i < argc - 3; i ++) {
	   			expand_escapes(temp, argv[i + 3]);

	   			// Having each field in a separate conditional allows for easier
	   			// manipulation of different types of fields.

	   			if(!strncmp("disk_model", temp, strlen("disk_model"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("disk_serial", temp, strlen("disk_serial"))){
	   				set_kvp(keys[i], values[i], temp);
   					char * token = strtok(temp, "=");
					token = strtok(NULL, "=");
					strcpy(disk_serial, token);
	   			} else if (!strncmp("disk_size", temp, strlen("disk_size"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("security_erase", temp, strlen("security_erase"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("enhanced_erase", temp, strlen("enhanced_erase"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("source_drive", temp, strlen("source_drive"))){
	   				set_kvp(keys[i], values[i], temp);
				} else if (!strncmp("parent", temp, strlen("parent"))){
	   				set_kvp(keys[i], values[i], temp);
   				} else if (!strncmp("wiped", temp, strlen("wiped"))){
   					// set_kvp(keys[i], values[i], temp);
   					keys[i] = "wiped";
   					values[i] = "CURRENT_TIMESTAMP";
   				} else if (!strncmp("transport", temp, strlen("transport"))){
	   				set_kvp(keys[i], values[i], temp);
				} else if (!strncmp("firmware", temp, strlen("firmware"))){
   					set_kvp(keys[i], values[i], temp);
   				} else if (!strncmp("form_factor", temp, strlen("form_factor"))){
	   				set_kvp(keys[i], values[i], temp);
   				} else if (!strncmp("rpm", temp, strlen("rpm"))){
	   				set_kvp(keys[i], values[i], temp);
   				} else if (!strncmp("health", temp, strlen("health"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else {
	   				printf("Error, argument not supported: %s\n", temp);
	   				PRINTARGS
	   				exit(1);
	   			}


	   		}	

	   		// Before deciding whether to insert or update we must check if the entry exists already.
	   		sprintf(query,"SELECT * FROM disk WHERE disk_serial=\"%s\" AND wiped='0';",
		    		disk_serial);
	   		
	   		if (mysql_query(&mysql, query))
			    finish_with_error(&mysql);
			  
			MYSQL_RES *result = mysql_store_result(&mysql);
						  
			if (result == NULL)
			    finish_with_error(&mysql);

			unsigned int res_count = mysql_num_rows(result);

			if (res_count == 0) { // If an entry doesn't exist for that disk insert it.

		   		char * values_str = malloc(sizeof(char) * 512);
		   		char * keys_str = malloc(sizeof(char) * 512);
		   		values_str[0] = '\0';
		   		keys_str[0] = '\0';
		   		for (i = 0; i < argc - 3; i++){
					strcat(values_str, values[i]);
					strcat(keys_str, keys[i]);
					if (i != argc - 4){
						strcat(values_str, ", ");
						strcat(keys_str, ", ");
					}
		   				
		   		}

			    sprintf(query,"INSERT INTO disk (%s) VALUES(%s);",
			    		keys_str, values_str);

		    	if (mysql_query(&mysql, query))
	     	 		finish_with_error(&mysql);

			} else if (res_count == 1) { // If the entry does exist, update it.

				sprintf(query, "UPDATE disk SET synced = 0, ");

				for (i = 0; i < argc - 4; i++) {
					strcat(query, keys[i]);
					strcat(query, " = ");
					strcat(query, values[i]);
					strcat(query, ", ");
				}

				strcat(query, keys[i]);
				strcat(query, " = ");
				strcat(query, values[i]);

				sprintf(temp, " WHERE disk_serial = \"%s\" AND wiped = 0 ;", disk_serial);
				strcat(query, temp);

				if (mysql_query(&mysql, query))
	     	 		finish_with_error(&mysql);

			} else { // Should not happen
				printf("%d\n", res_count);
			}

		    //printf("%s\n", query);
		    //printf("%s\n",disk_serial);
			
		} else if (argv[2][0] == '-' && argv[2][1] == 'c') {
	    	printf( "Database 'computer' Selected\n");

	    	char * asset_no = malloc(sizeof(char) * 128);

	   		char * temp = malloc(sizeof(char) * 128);
	   		for (i = 0; i < argc - 3; i ++) {
	   			expand_escapes(temp, argv[i + 3]);

	   			if(!strncmp("asset_no", temp, strlen("asset_no"))){
	   				set_kvp(keys[i], values[i], temp);
   					char * token = strtok(temp, "=");
					token = strtok(NULL, "=");
					strcpy(asset_no, token);
	   			} else if (!strncmp("service_tag", temp, strlen("service_tag"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("model", temp, strlen("model"))){
	   				set_kvp(keys[i], values[i], temp);
	   			} else if (!strncmp("processor", temp, strlen("processor"))){
	   				set_kvp(keys[i], values[i], temp);
   				} else if (!strncmp("is_laptop", temp, strlen("is_laptop"))){
   					set_kvp(keys[i], values[i], temp);
	   			} else {
	   				PRINTARGS
	   				printf("%s\n", temp);
	   				exit(1);
	   			}
	   				
	   		}


	   		// Before deciding whether to insert or update we must check if the entry exists already.
	   		sprintf(query,"SELECT * FROM computer WHERE asset_no=\"%s\";",
		    		asset_no);
	   		
	   		if (mysql_query(&mysql, query))
			    finish_with_error(&mysql);
			  
			MYSQL_RES *result = mysql_store_result(&mysql);
						  
			if (result == NULL)
			    finish_with_error(&mysql);

			unsigned int res_count = mysql_num_rows(result);

			if (res_count == 0) { // If an entry doesn't exist for that disk insert it.



		   		char * values_str = malloc(sizeof(char) * 512);
		   		char * keys_str = malloc(sizeof(char) * 512);
		   		values_str[0] = '\0';
		   		keys_str[0] = '\0';
		   		for (i = 0; i < argc - 3; i++){
					strcat(keys_str, keys[i]);
					strcat(values_str, values[i]);
					if (i != argc - 4){
						strcat(values_str, ", ");
						strcat(keys_str, ", ");
					}
				}
			    sprintf(query,"INSERT INTO computer (%s) VALUES(%s);",
			    		keys_str, values_str);

			    	if (mysql_query(&mysql, query))
		     	 		finish_with_error(&mysql);
			} if (res_count == 1){

				if (mode == 0)
					exit(2);

				sprintf(query, "UPDATE computer SET synced = 0, ");

				for (i = 0; i < argc - 4; i++) {
					strcat(query, keys[i]);
					strcat(query, " = ");
					strcat(query, values[i]);
					strcat(query, ", ");
				}

				strcat(query, keys[i]);
				strcat(query, " = ");
				strcat(query, values[i]);

				sprintf(temp, " WHERE asset_no = \"%s\";", asset_no);
				strcat(query, temp);

				if (mysql_query(&mysql, query))
	     	 		finish_with_error(&mysql);

			}
		    //printf("%s\n", query);

		} else {
			PRINTARGS
		}
	

	}else
    	printf( "Failed to connect to Database: Error: %s\n", mysql_error(&mysql));

	printf("Sql handler done.\n");
	mysql_close(&mysql);

	exit(0);


}