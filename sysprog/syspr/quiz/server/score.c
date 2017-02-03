/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * score.h: Implementierung des Score-Agents
 */

#include "score.h"
#include "login.h"
#include "catalog.h"
#include "rfc.h"
#include "common/util.h"
#include "user.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

static sem_t scoreSemaphore;

void *scoreThread(void* parameter) {

  clientdata clientData;
	sem_init(&scoreSemaphore, 0, 0);

	while (1) {
		//Eintritt Score
	  sem_wait(&scoreSemaphore); //Warten (bei sem_post() hier los)
  		
		  
		  for(int i = 0; i < getPlayerCount();i++){
         clientData = getClientData(i);
      }
		  sortDataScore(clientData);
		  
		  //Anmeldung
      //Spielstart
      //Spielende
      //Punktestand ändert sich
		  sendLST();
		
	}
}

void unlockSem(){
  sem_post(&scoreSemaphore);
}


void destroySem(){
  sem_destroy(&scoreSemaphore); 
}

