/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * login.c: Implementierung des Logins
 */

#include "common/util.h"
#include "login.h"
#include "rfc.h"
#include "clientthread.h"
#include "user.h"
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

/*
 * Login-Thread ließt Login-Anfrage von Clients ein
 * Spielerdaten Struktur getClientData() setClient() user.c
 * LOK rfc.c, LOK senden user.c
 * erzeugt Clientthreads clientthread.c
 */
int c_sockfd;
int s_sockfd;

void *loginThread(void* parameter) {

	s_sockfd = *(int*) parameter;

	socklen_t clientlen;
	struct sockaddr_in client_addr;
	char clientMessage[1000];
	int n;

	memset(clientMessage, '\0', sizeof(clientMessage));

	while (1) {
		clientlen = sizeof(client_addr);
		c_sockfd = accept(s_sockfd, (struct sockaddr *) &client_addr,
				&clientlen);

		if (c_sockfd < 0) {
			errorPrint("Fehler: Socket akzeptieren");
			return 0;
		}
    mutexLock(); //Mutex getPlayerCount
		//5.Client abweisen
		if (getPlayerCount() >= 4) { 
		   //Mutex freigeben
			//errorPrint("Spieler abweisen, es sind bereits 4 Spieler angemeldet.");
			//Fehler fatal
			sendERR("Login nicht möglich - Server voll",1,c_sockfd);	
			close(c_sockfd);
		}mutexUnlock(); 

		//Vom Socket lesen
		n = read(c_sockfd, clientMessage, sizeof(clientMessage));

		if (n < 0) {
			errorPrint("Fehler beim Lesen vom Socket");
			return 0;
		}

		//Eingehende Nachricht LRQ
		if (clientMessage[0] == 1) {
			//setClient() Clientdaten in Struktur setzen

			if (!setClient(clientMessage, c_sockfd)) { //LOK-Type 2 in user.c
				sendERR("Login nicht möglich - Name bereits vergeben",0,c_sockfd);	
				//errorPrint("Spieler abweisen, name schon vergeben.");					
			  close(c_sockfd);	  	
			}
	  mutexLock();//Mutex 
		//Spiel hat bereits begonnen
		if(getStartGame()==1){	
		  //Mutexende
		  sendERR("Spieler abweisen, Spiel hat bereits begonnen.",1,c_sockfd);		  
			//errorPrint("Spieler abweisen, Spiel hat bereits begonnen.");
			close(c_sockfd);				
		}mutexUnlock();
		//Clientthread erzeugen
		pthread_t* thread = getThread(getPlayerCount());
		if ((pthread_create(thread, NULL, clientThread, (void*) &c_sockfd))< 0) {
			errorPrint("Fehler beim client-Thread erzeugen");
		}
		debugPrint("client-Thread erfolgreich");

		//5.Client abweisen
    setPlayerCount(getPlayerCount() + 1); //erhöhen
		}
	}
	closeAllSockets();
	debugPrint("Login-Thread beendet.");
	pthread_exit(NULL);
}

void closeAllSockets() {
	for (int i = 0; i < getPlayerCount(); i++){
		close(getClientData(i).cSockfd);
	}
	close(c_sockfd);
	close(s_sockfd);
}

