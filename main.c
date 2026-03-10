#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void processInput(char* input, int count);
void getpath(char** tokens);
void setpath(char** tokens);
void changeDirectory(char** tokens);
void externalCommand(char** tokens);
void printHistory(int count);
void invokeHistory(char** tokens, int count);
void saveHistory(int count);
void loadHistory(int* count);
void trim(char *s);
void checkAlias(char* input);
void addAlias(char** tokens);
void deleteAlias(char** tokens);
void printAliases();
void saveAliases();
void loadAliases();

typedef struct {
    char* name;
    char* command;
} alias;

alias* aliases[10] = {NULL};
char* history[20] = {NULL};

int main(void) {

    char input[513];
    int exit = 0;
    int count = 0;

    // Save original PATH
    char* originalPath = getenv("PATH");

    // Set current directory to HOME
    char* userHomeDir = getenv("HOME");
    chdir(userHomeDir);

    // Load history from file
    loadHistory(&count);

    // Load aliases
    loadAliases();

    while (!exit) {
        printf("$ ");

        // If fgets returns NULL, Ctrl-D has been pressed
        if (fgets(input, 513, stdin) == NULL) {
            printf("\n");
            exit = 1;
            break;
        }

        // Clear any unread characters from stdin
        setbuf(stdin, NULL);

        // Remove leading whitespace
        trim(input);

        //
        if (strcmp(input, "\n") == 0 || strcmp(input, " \n") == 0) {
            continue;
        }

        // Remove newline from input
        input[strcspn(input, "\n")] = 0;

        // Add command to history
        if (strcspn(input, "!") != 0) {
            if (history[count] != NULL) {
                free(history[count]);
            }
            history[count] = strdup(input);
            count = (count + 1) % 20;
        }

        // Check if input is "exit", if it is then quit the shell
        if (strcmp(input, "exit") == 0) {
            exit = 1;
            break;
        }

        processInput(input, count);
    }

    saveHistory(count);
    saveAliases();

    // Clear history memeory allocations
    for (int i = 0; i < 20; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }

    // Restore and print original PATH
    setenv("PATH", originalPath, 1);
    printf("Restored PATH:\n");
    getpath(NULL);

    return 0;
}

void trim(char *s) {

    // Pointer to the beginning of the trimmed string
    char *ptr = s;

    // Skip leading spaces
    while (*s == ' ') s++;

    // Shift remaining characters to the beginning
    while ((*ptr++ = *s++));
}

void checkAlias(char* input) {
    // Check alias
    for (int i = 0; i < 10; i++) {
        if (aliases[i] != NULL) {
            int aliasLen = strlen(aliases[i]->name);

            // Check if input starts with alias name
            if (strncmp(input, aliases[i]->name, aliasLen) == 0) {

                // Check alias name is full token and not just start of one
                if (input[aliasLen] == ' ' || input[aliasLen] == '\0') {
                    char newInput[513];
                    snprintf(newInput, sizeof(newInput), "%s%s", aliases[i]->command, input + aliasLen);
                    strcpy(input, newInput);
                    break;
                }
            }
        }
    }
}

void processInput(char* input, int count) {
    // Check if input is alias
    //checkAlias(input);
    char prev[513];
    do {
        strcpy(prev, input);
        checkAlias(input);
    } while (strcmp(prev, input) != 0);

    // Split input string into tokens
    int i = 0;
    char* tokens[50] = {NULL};
    char* tok = strtok(input, " \t|><&;");
    while (tok != NULL) {
        tokens[i++] = tok;
        tok = strtok(NULL, " \t|><&;");
    }
    tokens[i] = NULL;

    //
    if (strcmp(tokens[0], "getpath") == 0) {
        getpath(tokens);
    }
    else if (strcmp(tokens[0], "setpath") == 0) {
        setpath(tokens);
    }
    else if (strcmp(tokens[0], "cd") == 0){
        changeDirectory(tokens);
    }
    else if (strcmp(tokens[0], "history") == 0) {
        printHistory(count);
    }
    else if (strcspn(input, "!") == 0) {
        invokeHistory(tokens, count);
    }
    else if (strcmp(tokens[0], "alias") == 0) {
        if (tokens[1] != NULL) {
            addAlias(tokens);
        }
        else {
            printAliases();
        }
    }
    else if (strcmp(tokens[0], "unalias") == 0) {
        deleteAlias(tokens);
    }
    else {
        externalCommand(tokens);
    }
}

void getpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens != NULL && tokens[1] != NULL) {
        printf("Error: Too many arugments. getpath takes no parameters\n");
        return;
    }

    // Get and print the current value of PATH
    char* currentPath = getenv("PATH");
    printf("%s\n", currentPath);
}

void setpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens[2] != NULL) {
        printf("Error: Too many arguments. setpath takes exactly 1 parameter\n");
        return;
    }
    else if (tokens[1] == NULL) {
        printf("Error: Not enough arguments. setpath requires 1 parameter (the path)\n");
        return;
    }

    // Set the value of PATH to tokens[1]
    setenv("PATH", tokens[1], 1);
}

void changeDirectory(char** tokens){
    // Check if too many parameters have been passed
    if (tokens[2] != NULL) {
        printf("Error: Too many arguments. cd takes either 0 or 1 parameters\n");
        return;
    }

    if(tokens[1] != NULL){
        if (chdir(tokens[1]) != 0) {
            // Error changing directories
            char errMsg[200] = "Directory change failed: ";
            strcat(errMsg, tokens[1]);
            perror(errMsg);
        }
    }
    else{
        chdir(getenv("HOME"));
    }
}

void externalCommand(char** tokens) {
    // Create fork
    pid_t p = fork();
    if (p < 0) {
        // Error when forking
        printf("Error forking\n");
    }
    else if (p == 0) {
        // We're the child process
        if (execvp(tokens[0], tokens) == -1) {
            // Exec error
            char errMsg[200] = "Error running command: ";
            strcat(errMsg, tokens[0]);
            perror(errMsg);
            exit(1);
        }
    }
    else if (p > 0) {
        // We're the parent process
        // Wait for child process to finish
        wait(NULL);
    }

}

void printHistory(int count) {
    // Print all items in the history array
    int num = 1;
    for (int i = 0; i < 20; i++) {
        int index = (count + i) % 20;
        if (history[index] != NULL) {
            printf("%2d %s\n", num++, history[index]);
        }
    }
}

void invokeHistory(char** tokens, int count) {
    int historyCount = 0;
    int index = -1;

    // Count items in history
    for (int i = 0; i < 20; i++) {
        if (history[i] != NULL) {
            historyCount++;
        }
    }

    // Check if any history exists
    if (historyCount == 0) {
        printf("Error: No commands in the history\n");
        return;
    }

    // Get index for last used command
    if (strcmp(tokens[0], "!!") == 0) {
        index = (count - 1 + 20) % 20;
    }
    // Get index for n number of commands ago
    else if (tokens[0][1] == '-') {
        int n = atoi(tokens[0] + 2);
        index = (count - n + 20) % 20;
    }
    // Get index for specific command number
    else {
        int n = atoi(tokens[0] + 1);
        int firstCmd = (count - historyCount + 20) % 20;
        index = (firstCmd + (n - 1)) % 20;
    }

    // Check index actually exists
    if (history[index] == NULL) {
        printf("Invalid history reference.\n");
        return;
    }

    // Print the command being executed
    printf("%s\n", history[index]);

    // Duplicate command and run it
    char* cmd = strdup(history[index]);
    processInput(cmd, count);
    free(cmd);
}

void saveHistory(int count) {
    // Get file path for history file
    char file[512];
    strcat(strcpy(file, getenv("HOME")), "/.hist_list");

    // Open file to save history to
    FILE *fp = fopen(file, "w");

    // Check if file opened correctly
    if (!fp) {
        printf("Failed to save history!\n");
        return;
    }

    // Save history array to file
    for (int i = 0; i < 20; i++) {
        int index = (count + i) % 20;
        if (history[index] != NULL) {
            fprintf(fp, "%s", history[index]);
            int tmp = (index + 1) % 20;
            if (history[tmp] != NULL) {
                fprintf(fp, "\n");
            }
        }
    }

    // Close file
    fclose(fp);
}

void loadHistory( int* count) {
    // Get file path for history file
    char file[512];
    strcat(strcpy(file, getenv("HOME")), "/.hist_list");

    // Open file to load history from
    FILE *fp = fopen(file, "r");

    // Check if file opened correctly
    if (!fp) {
        printf("Could not find persistant history\n");
        return;
    }

    //
    char buffer[512];

    // Load history from file
    while (fgets(buffer, 512, fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        int len = strlen(buffer);
        char* val = malloc((sizeof(char) * len) + 1);
        strcpy(val, buffer);

        history[*count] = val;

        *count = (*count + 1) % 20;
    }

    // Close file
    fclose(fp);
}

void addAlias(char** tokens) {
    if (tokens[2] == NULL) {
        printf("Error: correct usage is\nalias <name> <command>\n");
        return;
    }

    for (int i = 0; i < 10; i++) {
        if (aliases[i] != NULL && strcmp(aliases[i]->name, tokens[1]) == 0) {
            // Alias already exists
            printf("Overwriting alias %s\n", aliases[i]->name);
            char cmd[512] = "";
            for (int j = 2; j < 20; j++) {
                if (tokens[j] != NULL) {
                    strcat(cmd, tokens[j]);
                    strcat(cmd, " ");
                }
            }
            strcpy(aliases[i]->command, cmd);
            return;
        }

        if (aliases[i] == NULL) {

            // Create alias
            char cmd[512] = "";
            for (int j = 2; j < 20; j++) {
                if (tokens[j] != NULL) {
                    strcat(cmd, tokens[j]);
                    strcat(cmd, " ");
                }
                else {
                    break;
                }
            }

            aliases[i] = malloc(sizeof(alias));
            aliases[i]->name = malloc(strlen(tokens[1]) + 1);
            aliases[i]->command = malloc(sizeof(char) * 512);

            strcpy(aliases[i]->name, tokens[1]);
            strcpy(aliases[i]->command, cmd);

            return;
        }

        if (i == 9) {
            // Alias array full
            printf("Cannot create alias. You already have the maximum (10)\n");
            return;
        }
    }
}

void deleteAlias(char** tokens) {
    if (tokens[2] != NULL) {
        printf("Error: unalias takes only one parameter (the alias you want to remove)\n");
        return;
    }

    int count = 0;

    for (int i = 0; i < 10; i++) {
        if (aliases[i] != NULL) {
            count++;
        }
        if (aliases[i] != NULL && strcmp(aliases[i]->name, tokens[1]) == 0) {
            // Alias found, delete it
            free(aliases[i]->name);
            free(aliases[i]->command);
            free(aliases[i]);
            aliases[i] = NULL;
            return;
        }
    }

    if (count == 0) {
        printf("Error deleting alias: no aliases exist\n");
    }
    else {
        printf("Error deleting alias: alias doesn't exist\n");
    }

}

void printAliases() {
    int count = 0;
    for (int i = 0; i < 10; i++) {
        if (aliases[i] != NULL) {
            printf("%s = %s\n", aliases[i]->name, aliases[i]->command);
            count++;
        }
    }

    if (count == 0) {
        printf("No aliases saved\n");
    }
}

void saveAliases() {
    char file[512];
    strcat(strcpy(file, getenv("HOME")), "/.aliases");

    FILE *fp = fopen(file, "w");

    // Check if file opened correctly
    if (!fp) {
        printf("Failed to save aliases!\n");
        return;
    }
    for (int i = 0; i < 10; i++) {
        if (aliases[i] != NULL) {
            fprintf(fp, "%s ", aliases[i]->name);
            fprintf(fp, "%s\n", aliases[i]->command);

        }
        else{break;}
    }

    fclose(fp);
}

void loadAliases() {
    char file[512];
    strcat(strcpy(file, getenv("HOME")), "/.aliases");

    FILE *fp = fopen(file, "r");
    if (!fp) {
        printf("Could not find persistent aliases\n");
        return;
    }

    char buffer[512];

    while (fgets(buffer, sizeof(buffer), fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        // Split <name><space><command>
        char *space = strchr(buffer, ' ');
        if (!space) continue;

        *space = '\0';
        char *name = buffer;
        char *cmd  = space + 1;

        char *tokens[20] = {NULL};
        tokens[0] = "alias";
        tokens[1] = name;
        tokens[2] = cmd;



        addAlias(tokens);
    }

    fclose(fp);
}
