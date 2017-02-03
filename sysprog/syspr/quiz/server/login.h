/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.h: Header für das Login
 */
#include "rfc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h> 
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#ifndef LOGIN_H
#define LOGIN_H 

void closeAll();
void closeAllSockets();
void *loginThread(void* parameter);

#endif
