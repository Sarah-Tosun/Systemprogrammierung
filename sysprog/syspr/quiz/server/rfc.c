/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 *
 * rfc.c: Implementierung der Funktionen zum Senden und Empfangen von
 * Datenpaketen gemäß dem RFC
 */

#include "rfc.h"
#include "user.h"
#include "catalog.h"
#include "common/util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h> 
#include <errno.h>
#include <unistd.h>

static DATA queData[30];  //MAX_QU];
static 	uint8_t rang = 0;

DATA getQueData(int index){
  return queData[index];
}

void setQueData(DATA que, int i){ 
  queData[i] = que;
}

LOK getLOK(int id){
  LOK lok;
  lok.header.type = 2;
  lok.header.length = htons(3);
  lok.rfcVersion = 9;
  lok.maxPlayers = 4;
  lok.clientID = id;
  return lok;
}

//Anzahl Kataloge jeder Catalog eigenes Paket!!!
CRE getCRE(char catName[]){
  CRE cre;
  cre.header.type = 4;
  memset(cre.fileName, '\0' , strlen(catName));
  strcpy(cre.fileName, catName);
  cre.header.length = htons(strlen(cre.fileName));
  return cre;
}

//Katalogauswahl Spielleiter
CCH getCCH(){
  catalog catalogs = getCatalogs();
  CCH cch;
  cch.header.type = 5;
  cch.header.length = htons(strlen(catalogs.aktCatalog));
  strcpy(cch.fileName, catalogs.aktCatalog);
  return cch;
}

//Spielerliste
LST getLST(){
	LST lst; 
	lst.header.type = 6;
  lst.header.length = htons((getPlayerCount()*37)); //((1+players)*37)

	for(int i = 0; i < getPlayerCount(); i++){
	  memset(lst.player[i].name, '\0' , sizeof(lst.player[i].name));
		strcpy(lst.player[i].name, getClientData(i).player.name);
		lst.player[i].score = htonl(getClientData(i).player.score);
		lst.player[i].clientID = getClientData(i).player.clientID;
		debugPrint("NAMELST %s", lst.player[i].name);
		debugPrint("SCORELST %d", htonl(lst.player[i].score));
		debugPrint("IDLST %d", lst.player[i].clientID);
	}
	  debugPrint("CountLSt %d", (getPlayerCount()*37));
	return lst;
}

STG getSTG(){
  catalog catalogs = getCatalogs();
  STG stg;
  stg.header.type = 7;
  stg.header.length = htons(sizeof(catalogs.aktCatalog));
  memset(stg.fileName, '\0' , sizeof(catalogs.aktCatalog));
  strcpy(stg.fileName, catalogs.aktCatalog);
  return stg;  
}

QUE getQUE(int i){
  QUE que;
  que.header.type = 9;
  que.header.length = htons(769);  
  strcpy(que.data.questionText, queData[i].questionText);
  strcpy(que.data.answerText1, queData[i].answerText1);
	strcpy(que.data.answerText2, queData[i].answerText2);
	strcpy(que.data.answerText3, queData[i].answerText3);
	strcpy(que.data.answerText4, queData[i].answerText4);
	que.data.timeLimit = queData[i].timeLimit;
  return que;
}

QRE getQRE(uint8_t bitmask){
  QRE qre;
  qre.header.type = 11;
  qre.header.length = htons(1);
  //Bitmaske Antworten
  qre.bitmask = bitmask;
  return qre;
}

GOV getGOV(uint32_t score){
  GOV gov;
	gov.header.type = 12;
	gov.header.length = htons(5);
	gov.score = (htonl(score));
	rang++;
	gov.rank = rang;
	return gov;
}

ERR getERR(char* message, uint8_t subtype){
	ERR rfcErr;
	rfcErr.header.type = 255;
	rfcErr.header.length = htons((strlen(message))+1);
	rfcErr.subtype = subtype;
	memset(rfcErr.errorMessage, '\0', sizeof(rfcErr.errorMessage));
	strcpy(rfcErr.errorMessage, message);
	return rfcErr;
}


//Senden
void sendLOK(int c_sockfd, int clientNo){
	LOK lok;	
	lok = getLOK(clientNo);
  write(c_sockfd, &lok, sizeof(lok));
}

void sendCRE(int c_sockfd){
  catalog catalogs = getCatalogs();
		for(int i = 0; i < catalogs.catNo; i++){
			debugPrint("catalog_list inhalt: %s",catalogs.catName[i]);
			CRE cre = getCRE(catalogs.catName[i]);
			write(c_sockfd, &cre, (strlen(cre.fileName)+3));
			memset(&cre,0,sizeof(cre));
		}   
}

void sendLST(){
   LST lst;
   lst = getLST();
   infoPrint("Versende Liste an: %d", getPlayerCount());
   for(int i = 0; i<getPlayerCount();i++) {
     write(getClientData(i).cSockfd, &lst, (ntohs(lst.header.length))+3);
  }
}

void sendCCH(char clientMessage[]){
  catalog catalogs = getCatalogs();
	int length = clientMessage[2];
	CCH cch;
	for (int i = 0; i <= length; i++) {
		if(i == length){
		  catalogs.aktCatalog[i] = '\0'; //letzter Katalog
		}else {
		  	catalogs.aktCatalog[i] = clientMessage[i + 3];	
		}
	}
	setCatalogs(catalogs);
	cch = getCCH();
  debugPrint("Versende aktuellen Katalog an: %d", getPlayerCount());
  for(int i = 0; i<getPlayerCount();i++) {
    write(getClientData(i).cSockfd, &cch, (ntohs(cch.header.length))+3);
  }
}

void sendSTG(char clientMessage[]){
  catalog catalogs = getCatalogs();
	int length = clientMessage[2];
	STG stg;
	for (int i = 0; i <= length; i++) {
		if(i == length){
		  catalogs.aktCatalog[i] = '\0'; //letzter Katalog
		}else {
		  	catalogs.aktCatalog[i] = clientMessage[i + 3];	
		}
	}
	setCatalogs(catalogs);
	stg = getSTG();
  debugPrint("Versende aktuellen Katalog an: %d", getPlayerCount());
  for(int i = 0; i<getPlayerCount();i++) {
  write(getClientData(i).cSockfd, &stg, (ntohs(stg.header.length))+3);
  }
}

void sendQUE(int c_sockfd, int currentQuestion){
  QUE que;
  //Leer für Ende
  catalog catalogs = getCatalogs();
  if(currentQuestion == catalogs.questionNo){
    que.header.type = 9;
    que.header.length = 0;
    memset(&que.data, 0, sizeof(que.data));
    write(c_sockfd, &que, 3);
    debugPrint("leere Frage");
  }else {
    que = getQUE(currentQuestion); 
    write(c_sockfd, &que, (ntohs(que.header.length))+3);
  }
}

void sendQRE(int c_sockfd, uint8_t bitmask){ // ??? Aktuelle Frage index
  QRE qre;
  qre = getQRE(bitmask);
  write(c_sockfd, &qre, (ntohs(qre.header.length))+3);
}

void sendGOV(){
  //wenn letzte Frage leer, dann ende
  //sind alle an dieser Stelle
  GOV gov;
  uint32_t score;
  for(int i = 0; i <= getPlayerCount();i++) {
	  score = getClientData(i).player.score;
    gov = getGOV(score);
    write(getClientData(i).cSockfd, &gov, (ntohs(gov.header.length)+3));
  }
}

void sendERR(char* message, uint8_t subtype, int c_sockfd){
  ERR rfcErr;
  rfcErr = getERR(message, subtype);

  if(c_sockfd == 128){
    //an alle
    for(int i = 0; i<getPlayerCount();i++) {
      write(getClientData(i).cSockfd, &rfcErr, (ntohs(rfcErr.header.length))+3);
    }
  }else{
    write(c_sockfd, &rfcErr, (ntohs(rfcErr.header.length))+3);
  }

}








