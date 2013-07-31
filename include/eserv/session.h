#ifndef __ESERV_SESSION_H__
#define __ESERV_SESSION_H__

#include <crypt.h>
#include <time.h>
#include "hash.h"

#define SESSION_MAX_BYTES 2048
#define SESSION_ID "eserv_session"

static char pool[SESSION_MAX_BYTES];
static ex_mpool mpool;
static ex_hashmap session;

void ex_session_init();
char* sessionCreate(const char*, const void*);
void* sessionCheck(const char*);
char* sessionFromHeader(char*);

#endif
