/*
 * Systemprogrammierung
 * Multiplayer-Quiz
 *
 * Server
 * 
 * catalog.h: Header für die Katalogbehandlung und Loader-Steuerung
 */

#ifndef CATALOG_H
#define CATALOG_H
#define QUESTION_LENGTH 770;
#include "rfc.h"

int startLoader(char catpath[]);

#pragma pack(push, 1)
typedef struct{
	char* catName[32];
	int catNo;
	char aktCatalog[32];
	int questionNo;
}catalog;
#pragma pack(pop)

int start_loader(char* path);

catalog getCatalogs();
void setCatalogs(catalog);
int browseCatalogs();
int loadCatalog();
void closeAllCatalogs();

#endif
