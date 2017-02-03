/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.c: Implementierung der User-Verwaltung
 */

#include "user.h"
#include "rfc.h"
#include "common/util.h"
#include "main.h"
#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/select.h>
#include <sys/time.h>

/*
 * setClient() Spielerdaten anlegen, und LOK versenden
 * setPlayer() Spielerliste anlegen
 * getClientNo() Spieleranzahl ermitteln
 * getClientData() ClientStruktur zurückgeben
 * getPlayer() Spieler (Spielerliste)
 * isGameLeader() Spielleiter ermitteln
 */

//Globale Variablen mit Mutex Schützen
static clientdata client_data[MAX_CLIENTS];
static pthread_t client[MAX_CLIENTS];
static int playerCount = 0;
static int game = 0;
static int lok = 0;
static int currentQuestion[MAX_CLIENTS];

//Mutex - Schutz Kritischer Abschnitt
static pthread_mutex_t mutexData = PTHREAD_MUTEX_INITIALIZER;

pthread_t* getThread(int index) {
	return &client[index];
}

int setClient(char clientMessage[], int c_sockfd) {
	mutexLock();
	uint8_t clientNo = playerCount;

	char clientname[32];
	int a = 0;
	int j = 0;
	int length = clientMessage[3];
	//int length = ntohs(clientMessage[3]); //Umdrehen
	if(length > 32 && length == 0) {  //Namenslänge length prügen, Namenslänge 0
	    return 0;                            //Abweisen
  }
	for (int i = 4; i <= length; i++) {   //int 4 = 32bit
		clientname[j] = clientMessage[i];
		j++;
		if (a < clientNo) {
			a++;
		}
	}
	
	//Auf doppelte Namen prüfen.
	for(int i=0;i<=getPlayerCount();i++){
		debugPrint("Spieleranzahl:%d",getPlayerCount());
		if(strcmp(getClientData(i).player.name, clientname) == 0){
			return 0;
		}
	}  
  //ClientStruktur gefüllt
	strcpy(client_data[clientNo].player.name, clientname);
	client_data[clientNo].player.score = 0;
	client_data[clientNo].player.clientID = clientNo;
	client_data[clientNo].cSockfd = c_sockfd;	
  setLogin(1);
	mutexUnlock();
	//LOK
	sendLOK(c_sockfd, clientNo);
	//catalog catalogs = getCatalogs();
  //sendCCH(catalogs.aktCatalog);
	debugPrint("Type 2 LOK");
	return 1;
}

void mutexLock(void){
	//Mutex - Kritischer Abschnitt - Datenanlegen, Spieleranzahl erhöhen
	pthread_mutex_lock(&mutexData);
}
void mutexUnlock(void){
	//Mutex - Kritischer Abschnitt - Datenanlegen, Spieleranzahl erhöhen
	pthread_mutex_unlock(&mutexData);
}

void setPlayerCount(int count) {
	//Mutex - Schutz Kritischer Abschnitt
	mutexLock();
	playerCount = count;
	mutexUnlock();
}

int getPlayerCount() {
	//mutex
	return playerCount;
}

//StrukturArrayVariable zurückgeben
clientdata getClientData(int index) {
	return client_data[index];
}

void setCurrentQuestion(int count, int i) {
	//Mutex - Schutz Kritischer Abschnitt
	mutexLock();
	currentQuestion[count] = i;
	mutexUnlock();
}

int getCurrentQuestion(int i) {
	//Kein mutex da nur gelesen wird.
	return currentQuestion[i];
}

//Spielleiter ermittelt
int isGameLeader(int c_sockfd) {
	for (int i = 0; i < 4; i++) {
		if (client_data[i].cSockfd == c_sockfd) {
			if (client_data[i].player.clientID == 0) {
				//Beenden
				return 1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}

//Ein Client verlässt das Spiel (nicht der Spielleiter)     
int leaveClient(int c_sockfd) {
	for (int i = 1; i < MAX_CLIENTS; i++) {
		if (client_data[i].cSockfd == c_sockfd) {
			//Struktur neu ordnen 0 bleibt //Sortieren
			for(int j = 1; j < getPlayerCount() && (getPlayerCount() >= 3);j++) {		
				if(client_data[j + 1].player.name != 0){ //tauschen
				  //MUTEX
          mutexLock();
          strncpy(client_data[j].player.name, client_data[j + 1].player.name, strlen(client_data[j].player.name));
				  client_data[j].cSockfd = client_data[j + 1].cSockfd;
				  client_data[j].player.score = client_data[j + 1].player.score;
          //löschen				  
				  memset(&client_data[j + 1].player.name, 0, strlen(client_data[j+1].player.name));
				  client_data[j + 1].player.score = 0;			  	
				  //Mutex Ende
				  mutexUnlock();			
				 }
				}
				if(getPlayerCount() == 1){
			     sendERR("Client hat das spiel verlassen, zu wenig Spieler!",1, 128); //128 an alle
		       closeAll();
		    }
			}
			setPlayerCount(getPlayerCount() - 1);
			unlockSem(); //sendLST();		
		  return 1;
		}
		return 0;
}

//Mindestens 2 Spieler
int startGame(int c_sockfd){
	for (int i = 0; i < 4; i++) {
		if (client_data[i].cSockfd == c_sockfd) {
			if (client_data[i].player.clientID == 0 && client_data[i+1].player.clientID == 1) {
				//Starte Spiel wenn Spielleiter + 1 Spieler
				return 1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}

//Spielstand
void setStartGame(int index){
  game = index;
}
int getStartGame(){
  return game;
}

uint8_t setBitmask(int status, int currentQuestion, char message[], int cSockfd, long diff){
  DATA data;
  int id;

  for(int i = 0; i < getPlayerCount();i++){
      if(getClientData(i).cSockfd == cSockfd){
         id = getClientData(i).player.clientID;
      }
  }

  data = getQueData(currentQuestion);         
	uint8_t bitmask = data.bitmask; 	
	uint8_t firstBit; //char
	if(status == 0){ //0 richtig - 1 Zeitlimit abgelaufen
		firstBit = 0;
			if(((uint8_t)message[3])==bitmask){ //

				debugPrint("Richtige Antwort!");
				
				uint32_t score = scoreForTimeLeft(diff, data.timeLimit);

				//Mutex
        mutexLock();
               
				client_data[id].player.score = client_data[id].player.score+score;
				
				//Mutex Ende
			  mutexUnlock();
			  
			  bitmask = bitmask + firstBit;
			  return bitmask;

			}else{
				debugPrint("Falsche Antwort!");
			  bitmask = bitmask + firstBit;
			  return bitmask;
			}
			
	}else{
		debugPrint("Zeit abgelaufen!");
		//Zeitlimit abgelaufen
		firstBit = (1 << 7);
		bitmask = firstBit + bitmask; //^
		return bitmask;
	}
}

int sortDataScore(clientdata clientData){ 
  char tmpName;
	int tmpSocket;
	uint32_t tmpScore;
	
  for (int j = 0; j < getPlayerCount(); j++) {
			//Struktur neu ordnen nach Score
			for (int i = 0; i < getPlayerCount(); i++) {		  
			  if (client_data[i].player.score < client_data[i + 1].player.score) { //erster Score < zweiter Score = tauschen
				  strcpy(&tmpName, client_data[i].player.name);
				  tmpSocket = client_data[i].cSockfd;
				  tmpScore = client_data[i].player.score;
				  
				  strncpy(client_data[i].player.name, client_data[i + 1].player.name, strlen(client_data[i + 1].player.name));
				  client_data[i].cSockfd = client_data[i + 1].cSockfd;
				  client_data[i].player.score = client_data[i + 1].player.score;
				  
				  strcpy(client_data[i + 1].player.name, &tmpName);
				  client_data[i + 1].cSockfd = tmpSocket;
				  client_data[i + 1].player.score = tmpScore;
          return 1;         
			}
			}
		}
	
	return 0;
}

uint32_t scoreForTimeLeft(unsigned long timeLeft,unsigned long timeout){
   uint32_t score = (timeLeft * 1000UL)/timeout; /* auf 10er-Stellen runden */
   score = ((score+5UL)/10UL)*10UL;
   return score;
}

int isGOV(){
  for(int i = 0; i < getPlayerCount(); i++){
		if(client_data[i].done != 1){	//noch warten bis alle fertig sind
			return 0;
		}
	}
	debugPrint("Alle fertig");
	return 1; //alle fertig		
}

void setDone(int i){
  	//Mutex
    mutexLock();
		client_data[i].done = 1;//auf Done
		mutexUnlock();
}

void setLogin(int i){
  lok = i;
}

int getLogin(){
  return lok;
}





