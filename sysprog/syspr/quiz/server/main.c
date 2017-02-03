/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * main.c: Hauptprogramm des Servers
 */

#include "../common/util.h"
#include "login.h"
#include "score.h"
#include "catalog.h"
#include "user.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/file.h>

/*
* Socket erstellen, bind(), listen() 
* Loader starten:  startLoader(); in catalog.c
* LoginThread erzeugen  login.c
* ScoreThread erzeugen  score.c
*
*/

static pthread_t login; 
static pthread_t score;
static pthread_t server;
static sigset_t signals;
static int sig;


void closeAll(){
   //Variable wird noch konstanten Aufruf 
   closeAllSockets(); 
   closeAllCatalogs();
   pthread_cancel(login);
   pthread_cancel(score); 
   pthread_join(login, NULL); 
   pthread_join(score, NULL); 
   destroySem();
   unlink("/tmp/my_program_running");
   exit(0);
   return; 
}
/*
void terminate(){
  pthread_kill(server, SIGTERM); //SIGNAL-HANDLER
  for(;;)
    pause();
}*/

void signalHandler(){
  sigemptyset(&signals);
  sigaddset(&signals, SIGHUP);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);
  sigaddset(&signals, SIGUSR1);
  sigaddset(&signals, SIGQUIT);
  pthread_sigmask(SIG_SETMASK, &signals, NULL);
}

int create_lockfile(void){
  // O_CREAT: create file, if it does not exist yet
  // O_EXCL:  fail with EEXIST, if the file already exists
  int fd = open("/tmp/my_program_running", O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if(fd < 0)
  {
    if(errno == EEXIST)
      return 0;    // file already exists, program is probably already running
    else
      return -1;   // error opening file
  }
  else
  {
    close(fd);
    return 1;      // file has been created sucessfully
  }
}

int main(int argc, char *argv[])
{   server = pthread_self(); //Thread ID
    signalHandler();
    //signal (SIGINT,closeAll);
    setProgName(argv[0]); 
    infoPrint("Start - Server Gruppe 18");
    int opt;
    int port = 1234;; //Standardport
    struct sockaddr_in server_addr;
    const int y = 1;
    
    //Socket
    int s_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    //eingegebener Port -p übernehmen  
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':port = atoi(optarg); //(argv[1])
          debugPrint("Server started on Port %d", port);
                break;
        default:
          errorPrint("Falsche Parameter");
          close(s_sockfd);
                break;
        }
    } 
    unlink("/tmp/my_program_running");
    // LockFile mehrere Instanzen verhindern 
    if(create_lockfile() == 0) {
		  errorPrint("Instanz läuft bereits");
		  close(s_sockfd);
	  }
    
    if(s_sockfd < 0){
       errorPrint("Fehler: Socket konnte nicht erstellt werden");
       return 0;
    }
    
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;            //IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    //Adressen
    server_addr.sin_port = htons(port);            //Port
    
    setsockopt(s_sockfd, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    
    if (bind(s_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        errorPrint("Fehler beim Binden an die Adresse");
        exit(0);
    }
    
    //4 Clients
  	if(listen(s_sockfd,4)){
  	  errnoPrint("Fehler beim Warten auf Clients");
  	  return 0;
  	} 
    
    //LoginThread erzeugen, server-socket übergeben
    if((pthread_create (&login, NULL, (void *)loginThread, (void*)&s_sockfd)) != 0){
        errorPrint("Fehler beim Login-Thread erzeugen");
    }
    debugPrint("Login-Thread erfolgreich");

    //Loader aufrufen in catalog.c
    startLoader("");
    browseCatalogs();
    
    //ScoreThread erzeugen
    if((pthread_create (&score, NULL, (void *)scoreThread, NULL)) != 0){
      errorPrint("Fehler beim Score-Thread erzeugen");
    }
    debugPrint("Score-Thread erfolgreich");
    
    sigwait(&signals, &sig);
    closeAll();
return 0;
}


