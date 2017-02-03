/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Header für den Score Agent
 */

#ifndef SCORE_H
#define SCORE_H

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void *scoreThread(void* parameter);

void unlockSem();
void destroySem();


#endif
