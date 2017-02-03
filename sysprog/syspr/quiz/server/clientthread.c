/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * clientthread.c: Implementierung des Client-Threads
 */

#include "clientthread.h"
#include "login.h"
#include "rfc.h"
#include "common/util.h"
#include "catalog.h"
#include "user.h"
#include "score.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

/*
 * Nachrichtenaufbau, Versand rfc.c
 * Eingehende Nachrichten auswerten
 * 3 Katalog anfordern, 5 Katalogauswahl, 7 Starte Spiel, 8 Anfrage Frage, 10 Antwort
 * Nachrichten an Client senden, sendXXX in rfc.c
 * Spielerdaten in user.c
 * Prüfung verlässt der Spielleiter das Spiel oder ein anderer in user.c
 * Score unlockSem();
 * Prüfen auf genügend Spieler, Spielvorbereitung
 */

void *clientThread(void* parameter) {

	int c_sockfd = *(int*) parameter;
	int id;
	int currentQuestion = 0;
	struct timespec time = {0,0};
	struct timespec start = {0,0};
	struct timespec ende = {0,0};

	while (1) {
		char clientMessage[200];
		int n = 0;
		
		memset(clientMessage, 0, sizeof(clientMessage));
		
		n = read(c_sockfd, clientMessage, sizeof(clientMessage));
    debugPrint("n %d", n);
		if (n < 0) {
			debugPrint("Fehler beim Lesen vom Socket_Clientthread");
			closeAll();
		}
		//ID des akt Spielers
		for(int i = 0; i < getPlayerCount();i++){
       if(getClientData(i).cSockfd == c_sockfd){
          id = getClientData(i).player.clientID;
       }
    }
    debugPrint("ID spieler %d", id);

		//Wird das Spiel verlassen hat n den Wert 0
		if (n == 0) {
			//Verlässt der Spielleiter das Spiel - ENDE
			if (isGameLeader(c_sockfd)) {
			  sendERR("Spielleiter verlässt den Server",1, 128); //an alle
				closeAll();
			} else {
				//Welcher Client ist gegangen ?
				leaveClient(c_sockfd);
				//continue;
			}
			//closeAll();
		}
    catalog catalogs = getCatalogs();
    
    if(getLogin() == 1){
      	//Spielerliste versenden
        // LST versenden bei An-/Abmelden, Spielstart, QRE wenn Score "akt" anders Score "alt"
	      unlockSem(); //Anmelden LST senden im Score	
    }
    
    //Client input
		switch (clientMessage[0]) {
		case 3:
		  //Katalogliste
			sendCRE(c_sockfd);
			debugPrint("Type 4 CRE");
			break;
		case 5:
			debugPrint("Type 5 CCH");
			// Spielleiter wählt Katalog
			sendCCH(clientMessage); //Nur Spielleiter prüfen			
			//Warning 0
			if(getPlayerCount()==1){
        sendERR("Spiel kann nicht gestartet werden, weil noch zu wenig Teilnehmer",0,c_sockfd);
      }
			break;
		case 7:
			// Start Game
			if(startGame(c_sockfd)){
			   debugPrint("Type 7 STG");
			   sendSTG(clientMessage);
			   //Spielbeginn setzen
			     if (isGameLeader(c_sockfd)) {
			       setStartGame(1);  //Prüfung Spielleiter
			     }
			   debugPrint("Aktueller Katalog 1: %s",catalogs.aktCatalog);
			   loadCatalog(catalogs.aktCatalog);
			   unlockSem(); //starte Score sendLST();
			   break;
			}else {
			  //Fehler fatal zu wenig Spieler	  
			  sendERR("Spielabbruch wegen weniger als 2 Teilnehmern in der Spielphase",1, c_sockfd);
			  closeAll();			  
			  break;
			}		
		case 8://Bitte Frage Client
			debugPrint("Type 9 QUE Frage gesendet");
			sendQUE(c_sockfd, currentQuestion);
		  
		  fd_set fds;
	    FD_ZERO(&fds);
	    FD_SET(c_sockfd, &fds);
	
	     if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){ //Startzeit
	       perror("clock_gettime ");	
	     }
	     time.tv_sec = getQueData(currentQuestion).timeLimit;  //Zeitlimit
	     int retSelect = pselect(c_sockfd+1, &fds, NULL, NULL, &time, NULL); 
	     
	     if(clock_gettime(CLOCK_MONOTONIC, &ende) == -1){ //Endzeit
	       perror("clock_gettime ");	
	     }
	     //Kein 10 erhalten QAN, Zeit ist abgelaufen
	     catalog catalogs = getCatalogs();
	     if(currentQuestion < catalogs.questionNo){
			   if(retSelect == 0){  
           debugPrint("Zeit abgelaufen!");
			     uint8_t bitmask = setBitmask(1, currentQuestion, clientMessage, c_sockfd, 0);
			     sendQRE(c_sockfd, bitmask);
			     //Mutex
           mutexLock();
			     currentQuestion += 1; //Fragen erhöhen, solange noch nicht am Ende
           mutexUnlock();
			   }else if(retSelect == 1){
			     continue;
			   }
			  }else { //leere Frage Ende
			    for(int i = 0; i <= getPlayerCount(); i++){ //Am Ende für diesen Done auf 1 setzten
            if(getClientData(i).cSockfd == c_sockfd){
              setDone(i);
            }
          }
          //12 S-->C
			    //sind alle an dieser Stelle, fertig
			    //wenn letzte Frage leer, dann ende
          if(isGOV() == 1){
            sendGOV();
			      debugPrint("Ende");
			      closeAll();
            break;
          }else {
          }
			  }
			break;
		case 10: //Antwort Client
			debugPrint("Type 11 QRE Antwort");
			long diff = ende.tv_sec - start.tv_sec;
			uint8_t bitmask = setBitmask(0, currentQuestion, clientMessage, c_sockfd, diff);
			
			//Auswertung/Ergebnis
			sendQRE(c_sockfd, bitmask);
			
			//Frageergebnis Punktestand ändert sich -- Score
			//Punktestand ändert sich
			unlockSem();

      mutexLock();
			currentQuestion += 1; //Fragen erhöhen, solange noch nicht am Ende
			mutexUnlock();
 	   break;
	   //255 S-->C
	   sendERR("Fehler",0, c_sockfd);
	}
}
}


