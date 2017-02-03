/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * user.h: Header für die User-Verwaltung
 */

#ifndef USER_H
#define USER_H
#define MAX_CLIENTS 4
#define MAX_NAME 32

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
#include <pthread.h>

#pragma pack(push, 1)
//Spieler
typedef struct{
	char name[MAX_NAME];
	uint32_t score;
	uint8_t clientID;
}player;

typedef struct{
	player player;
	int cSockfd;
	int done;
}clientdata;
#pragma pack(pop)

pthread_t* getThread(int index);

int setClient(char clientMessage[], int c_sockfd);
void mutexLock(void);
void mutexUnlock(void);

clientdata getClientData(int index);
void setPlayerCount(int count);
int getPlayerCount();
int leaveClient(int c_sockfd);
int isGameLeader(int c_sockfd);
int startGame(int c_sockfd);
void setStartGame(int index);
int getStartGame();
uint32_t scoreForTimeLeft(unsigned long timeLeft,unsigned long timeout);
uint8_t setBitmask(int status, int currentQuestion, char message[], int cSockfd, long diff);
int sortDataScore(clientdata clientData);
void setCurrentQuestion(int count, int i);
int getCurrentQuestion(int i);
int isGOV();
void setDone(int i);
void setLogin(int i);
int getLogin();

#endif
