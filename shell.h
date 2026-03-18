#include "list.h"

// Constants
#define MAX_INPUT 513
#define MAX_ALIASES 10
#define MAX_HISTORY 20

// Alias structure
typedef struct {
    char* name;
    char* command;
} alias;

// Helpers
void startTasks(char** originalPath, int* historyCount);
void exitTasks(char* originalPath, int historyCount);
int  getInput(char* input);
void trim(char *s);
void processInput(char* input, int* historyCount);
void externalCommand(char** tokens);

// Built in commands
void getpath(char** tokens);
void setpath(char** tokens);
void changeDirectory(char** tokens);

// History
void printHistory(int historyCount);
void invokeHistory(char** tokens, int* historyCount);
void saveHistory(int historyCount);
void loadHistory(int* historyCount);

// Aliases
void loadAliases();
void saveAliases();
void printAliases();
void addAlias(char** tokens);
void deleteAlias(char** tokens);
int  checkAlias(char* input, List aliases_used);
