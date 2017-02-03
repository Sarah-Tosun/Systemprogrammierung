/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 *
 * rfc.h: Definitionen für das Netzwerkprotokoll gemäß dem RFC
 */

#ifndef RFC_H
#define RFC_H
//#define MAX_QU 50;


#include "common/question.h"
#include "user.h"
#include "catalog.h"

#pragma pack(push, 1)
typedef struct Header{
  uint8_t type;
  uint16_t length;
}header; 

// Type 1
typedef struct LoginRequest{
  header header; 
  uint8_t rfcVersion;
  char name[MAX_NAME];
}LRQ; 

// Type 2

typedef struct LoginResponseOk{
  header header; 
  uint8_t rfcVersion;
  uint8_t maxPlayers;
  uint8_t clientID;
}LOK;

// Type 3
typedef struct CatalogRequest{
  header header;
}CRQ;

// Type 4
typedef struct CatalogResponse{
  header header; 
  char fileName[60]; //nicht 0 terminiert oder leer
}CRE; 

// Type 5
typedef struct CatalogChange{
  header header; 
  char fileName[60]; //nicht 0 terminiert oder leer
}CCH;

// Type 6
typedef struct LST{
  header header; 
  player player[MAX_CLIENTS];
}LST;

// Type 7
typedef struct StartGame{
  header header; 
  char fileName[60]; //nicht 0 terminiert oder leer
}STG;

// Type 8
typedef struct QuestionRequest{
  header header;
}QRQ;

typedef struct Data{
      char questionText[256]; //0-terminiert
      char answerText1[128];
      char answerText2[128];
      char answerText3[128];
      char answerText4[128];
      uint8_t timeLimit;
      uint8_t bitmask;
}DATA;

// Type 9
typedef struct Question{
  header header;   // 769 oder 0
  DATA data;
}QUE;


// Type 10
typedef struct QuestionAnswered{
  header header; 
  uint8_t bitmask; //ersten 4 Stellen 0 (Werte 1-16 niedrigste Bit Frage 1 gewählt)
}QAN;

// Type 11
typedef struct QuestionResult{
  header header; 
  uint8_t bitmask; //aber bei bitmask höchste bit gesetzt bei Zeitlimitüberschreitung, sonst analog
}QRE; 
 
// Type 12
typedef struct GameOver{
  header header; 
  uint8_t rank; // rank => 4 und rank <= 0 (Rang der Spieler)
  uint32_t score; //Endgültiger Score
}GOV;

// Type 13
typedef struct ErrorWarning{
  header header; 
  uint8_t subtype; // 0 Warnung 1 fatal
  char errorMessage[200];
}ERR;

#pragma pack(pop)

DATA getQueData(int index); 
void setQueData(DATA que, int i);
  
LOK getLOK(int id);
CRE getCRE(char catName[]);
CCH getCCH();
LST getLST();
STG getSTG();
QUE getQUE(int i);
QRE getQRE(uint8_t bitmask);
GOV getGOV(uint32_t score);
ERR getERR(char* message, uint8_t subtype);


void sendLOK(int c_sockfd, int clientNo);
void sendCRE(int c_sockfd);
void sendCCH(char clientMessage[]);
void sendSTG(char clientMessage[]);
void sendLST();
void sendQUE(int c_sockfd, int currentQuestion);
void sendQRE(int c_sockfd, uint8_t bitmask);
void sendGOV();
void sendERR(char* message, uint8_t subtype, int c_sockfd);


#endif
