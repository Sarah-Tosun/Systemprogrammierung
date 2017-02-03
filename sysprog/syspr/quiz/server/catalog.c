/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.c: Implementierung der Fragekatalog-Behandlung und Loader-Steuerung
 */

#include "common/server_loader_protocol.h"
#include "catalog.h"
#include "rfc.h"
#include "common/question.h"
#include "common/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>

static catalog catalogs;
int stdoutPipe[2];
int stdinPipe[2];
int shmRoom;
Question *shmData;
int shmLength;


int startLoader(char catpath[]) {

	if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
		perror("Pipe konnte nicht erzeugt werden");
		return 3;
	}

	pid_t pid = fork();
	if (pid < 0) {
		perror("forking nicht erfolgreich, kein Kindprozess");
		return 1;
	} else if (pid == 0) { //Kindprozess

		if (dup2(stdinPipe[0], STDIN_FILENO) == -1) {
			perror("dup2(stdinPipe[0], STDIN_FILENO)"); //Kindseite lesen
			return 4;
		}
		/* Umleitung der Standardausgabe */
		if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1) {
			perror("dup2(stdoutPipe[1], STDOUT_FILENO"); //Kindseite schreiben
			return 5;
		}
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		close(stdoutPipe[1]);

		if (execl("loader", "loader", "catalogs/", NULL) == -1) { //überschreibt
			perror("Der Loader wurde erfolgreich gestartet.");
			return 0;
		} else {
			perror("execl");
			return 2;
		}
	} else { //Elternprozess
		close(stdinPipe[0]);
		close(stdoutPipe[1]);
	}
	return 2;
}

catalog getCatalogs() {
	return catalogs;
}

void setCatalogs(catalog cat) {
	catalogs = cat;
}

int browseCatalogs() {
	write(stdinPipe[1], "BROWSE", 6);
	write(stdinPipe[1], "\n", 1);
	char buffer[128];
	int i = 0;
	do {
		catalogs.catName[i] = readLine(stdoutPipe[0]);
		strcpy(buffer, catalogs.catName[i]);
		//debugPrint("name %s count %d",buffer, catalogs.catalog_count);
		catalogs.catNo++;
		i++;

	} while (buffer[0] != '\0');

	debugPrint("end of browse");
	return 1;
}

int loadCatalog() {
  shm_unlink("/quiz_reference_implementation");
	char message[100];
	int firstDez;
	int secondDez;
	debugPrint("AktuellerKatalog: %s", catalogs.aktCatalog);
	write(stdinPipe[1], "LOAD", 4);
	write(stdinPipe[1], " ", 1);
	write(stdinPipe[1], catalogs.aktCatalog, strlen(catalogs.aktCatalog));
	write(stdinPipe[1], "\n", 1);
	read(stdoutPipe[0], message, sizeof(message));

	//Message ASCII
	if (message[16] - 48 < 0) {
		firstDez = message[15] - 48;
		catalogs.questionNo = firstDez;
	} else {
		firstDez = message[15] - 48;
		secondDez = message[16] - 48;
		catalogs.questionNo = (firstDez * 10) + secondDez;
	}

	shmLength = catalogs.questionNo * QUESTION_LENGTH;
	//char path[] = {"/quiz_referenz_implementation"};
	//debugPrint("pfad %s", path);
	shmRoom = shm_open("quiz_reference_implementation", O_RDWR, 0600);
	shmData = mmap(NULL, shmLength, PROT_READ, MAP_SHARED, shmRoom, 0);
	//perror("shm_open");
	// Warnung Katalog konnte nicht geladen werden
	if(!shmRoom){
	  sendERR("Katalog kann nicht geladen werden",0,128); //128 an alle
	  return 1;
	}else {
  
   DATA data;
	 for(int i = 0; i < catalogs.questionNo; i++) {
	  data = getQueData(i);
	  memset(&data, 0, sizeof(data));
		strcpy(data.questionText, shmData[i].question);
		strcpy(data.answerText1, shmData[i].answers[0]);
		strcpy(data.answerText2, shmData[i].answers[1]);
		strcpy(data.answerText3, shmData[i].answers[2]);
		strcpy(data.answerText4, shmData[i].answers[3]);
		data.timeLimit = shmData[i].timeout;
		data.bitmask = shmData[i].correct;
		setQueData(data, i);
		debugPrint("cataloc.c question %s", data.questionText);
	 }
	}return 0;
}


void closeAllCatalogs(){
  shm_unlink("/quiz_reference_implementation");
	close(shmRoom);
	close(stdinPipe[1]);
	close(stdoutPipe[0]);
}
