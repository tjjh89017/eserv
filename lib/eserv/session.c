#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"

void ex_session_init(){
	
	ex_mpool_init(&mpool, pool, sizeof(pool));
	ex_hash_init(&session, &mpool, 97);
}

/*
 * *str for seesion_id
 * *session_data for a struct to store what program want seesion to store
 */


char* sessionCreate(const char *str, const void *session_data){

	// TODO find a better way to create session_id
	static char salt[] = "$1$AsAfCVgf";
	// encrypt 'str' and current time try to make session_id unique
	char *buf = malloc(strlen(str) + 20);
	sprintf(buf, "%s%d", str, (int)time(NULL));

	char *session_id = crypt(buf, salt);
	// register session_id into session
	ex_hash_add(&session, session_id, session_data);

	return session_id;
}

/*
 * For cgi to check if session_id exists
 * exist return data pointer
 * if not return NULL
 * session_id for find the  session data
 */
void* sessionCheck(const char* session_id){

	return ex_hash_find(&session, session_id);
}

/*
 * Get session_id for HTTP Header Cookie value
 * header_str
 */

// TODO don't work at all

char* sessionFromHeader(char *header_str){

	char *ret;
	char *delim = " =";
	
	ret = strtok(header_str, delim);
	while(ret != NULL){
		if(strcmp(ret, "eserv_session") != 0){
			ret = strtok(NULL, delim);
			break;
		}
		ret = strtok(NULL, delim);
	}

	return ret;
}
