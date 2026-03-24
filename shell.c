#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "shell.h"

// Create global variables
alias* aliases[MAX_ALIASES] = {NULL};
char*  history[MAX_HISTORY] = {NULL};

int main(void) {
    // Create local program variables
    char input[MAX_INPUT];
    int exit = 0;
    int historyCount = 0;
    char* originalPath = "";

    // Run start up tasks to initialise shell
    startTasks(&originalPath, &historyCount);

    while (!exit) {
        // Print input prompt
        printf("$ ");

        // Validate user input
        int valid = getInput(input);

        // Check if program loop needs to continue
        if (valid == 1) {
            continue;
        }
        // Check if program loop needs to exit
        else if (valid == 2) {
            printf("\n");
            exit = 1;
            break;
        }

        // Process validated input
        processInput(input, &historyCount);
    }

    // Run exit tasks to free memory etc
    exitTasks(originalPath, historyCount);

    return 0;
}

void startTasks(char** originalPath, int* historyCount) {
    // Save original PATH
    *originalPath = getenv("PATH");

    // Set current directory to HOME
    char* userHomeDir = getenv("HOME");
    chdir(userHomeDir);

    // Load history from file
    loadHistory(historyCount);

    // Load aliases
    loadAliases();
}

void exitTasks(char* originalPath, int historyCount) {
    // Save history and aliases to persistent file
    saveHistory(historyCount);
    saveAliases();

    // Clear history memory allocations
    for (int i = 0; i < MAX_HISTORY; i++) {
        if (history[i] != NULL) {
            free(history[i]);
        }
    }

    // Clear alias memory allocations
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i] != NULL) {
            free(aliases[i]->name);
            free(aliases[i]->command);
            free(aliases[i]);
        }
    }

    // Restore and print original PATH
    setenv("PATH", originalPath, 1);
    printf("Restored PATH:\n");
    getpath(NULL);
}

int getInput(char* input) {
    // Return 0 if input is valid and processing can continue
    // Return 1 to do nothing and continue loop
    // Return 2 if user wants to exit shell

    // Check if CTRL-D was pressed
    if (fgets(input, MAX_INPUT - 1, stdin) == NULL) {
        return 2;
    }

    // If only a new line character is in input, continue to next loop
    if (strcmp(input, "\n") == 0) {
        return 1;
    }

    // Trim input
    trim(input);

    // Remove newline from input
    input[strcspn(input, "\n")] = 0;

    // Check if exit was entered
    if (strcmp(input, "exit") == 0) {
        return 2;
    }

    // Input is good, continue processing
    return 0;
}

void trim(char *s) {
    // Remove *leading* whitespace from string
    // Two pointers initially at the beginning
    int i = 0, j = 0;

    // Skip leading spaces, i now points to first non whitespace character
    while (s[i] == ' ') i++; 

    // Shift the characters of string to remove leading spaces
    while ((s[j++] = s[i++]));
}

char* getAliasCommand(char* token) {
    //If token is alias name then return alias command, else return NULL
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i] != NULL && strcmp(token, aliases[i]->name) == 0) {
            return aliases[i]->command;
        }
    }
    // Token is not an alias
    return NULL;
}

int replaceAliases(List aliases_used, char** input) {
    char expandedInput[MAX_INPUT] = "";
    char tempInput[MAX_INPUT];
    strcpy(tempInput, *input);

    char* tok = strtok(tempInput, " \t");
    while (tok != NULL) {
        char* expansion = getAliasCommand(tok);

        if (expansion != NULL) {
            // Alias has been used
            if (contains(aliases_used, tok)) {
                printf("Circular alias detected with alias \"%s\": aborted\n", tok);
                clear(aliases_used);
                free(aliases_used);
                return 1;
            }
            push(aliases_used, tok);

            strcat(expandedInput, expansion);
        }
        else {
            // No alias, keep original token
            strcat(expandedInput, tok);
        }

        // Add whitespace between tokens
        strcat(expandedInput, " ");
        tok = strtok(NULL, " \t");
    }

    // Copy expanded input back into input
    trim(expandedInput);
    strcpy(*input, expandedInput);

    return 0;
}

void processInput(char* input, int* count) {
    // Create alias list
    List aliases_used = new_list();
    char prev_input[MAX_INPUT];

    // Save input for history
    char originalInput[MAX_INPUT];
    strcpy(originalInput, input);

    // If not creating or deleting an alias, replace tokens with their command
    if (strncmp(input, "alias", strlen("alias")) != 0 && strncmp(input, "unalias", strlen("unalias")) != 0) {
        do {
            strcpy(prev_input, input);
            if (replaceAliases(aliases_used, &input) == 1) {
                // Circular alias detected
                return;
            }
        } while (strcmp(input, prev_input) != 0);
    }

    // Split input string into tokens
    int i = 0;
    char* tokens[50] = {NULL};
    char* tok = strtok(input, " \t|><&;");
    while (tok != NULL) {
        tokens[i++] = tok;
        tok = strtok(NULL, " \t|><&;");
    }
    tokens[i] = NULL;

    if (strcspn(input, "!") == 0) {
        invokeHistory(tokens, count);
    }
    else {
        // Add command to history
        int pos = *count;
        if (history[pos] != NULL) {
            free(history[pos]);
        }
        history[pos] = strdup(originalInput);
        *count = (pos + 1) % MAX_HISTORY;

        // Check command
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
            printHistory(tokens, *count);
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

    clear(aliases_used);
    free(aliases_used);
}

void getpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens != NULL && tokens[1] != NULL) {
        printf("Error: Too many arguments. getpath takes no parameters\n");
        return;
    }

    // Get and print the current value of PATH
    char* currentPath = getenv("PATH");
    printf("%s\n", currentPath);
}

void setpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens[2] != NULL) {
        printf("Error: Too many arguments. Correct usage is:\nsetpath <path>\n");
        return;
    }
    else if (tokens[1] == NULL) {
        printf("Error: Not enough arguments. Correct usage is:\nsetpath <path>\n");
        return;
    }

    // Set the value of PATH to tokens[1]
    setenv("PATH", tokens[1], 1);
}

void changeDirectory(char** tokens){
    // Check if too many parameters have been passed
    if (tokens[2] != NULL) {
        printf("Error: Too many arguments. Correct usage is:\ncd <directory>\tTo change to <directory>\ncd\t\tTo change to home directory\n");
        return;
    }

    // If one parameter given, change to that directory
    if(tokens[1] != NULL){
        if (chdir(tokens[1]) != 0) {
            // Error changing directories
            char errMsg[200] = "Directory change failed: ";
            strcat(errMsg, tokens[1]);
            perror(errMsg);
        }
    }
    // No parameters, change to home folder
    else{
        printf("Changing to %s\n", getenv("HOME"));
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

void printHistory(char** tokens, int count) {
    // Check if parameters have been passed in
    if (tokens[1] != NULL) {
        printf("Error: history takes no parameters\n");
        return;
    }

    // Print all items in the history array
    int num = 1;
    for (int i = 0; i < MAX_HISTORY; i++) {
        int index = (count + i) % MAX_HISTORY;
        if (history[index] != NULL) {
            printf("%2d %s\n", num++, history[index]);
        }
    }
}

void invokeHistory(char** tokens, int* count) {
    int historyCount = 0;
    int index;

    int pos = *count;

    // Count items in history
    for (int i = 0; i < MAX_HISTORY; i++) {
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
        index = (pos - 1 + MAX_HISTORY) % MAX_HISTORY;
    }
    // Get index for n number of commands ago
    else if (tokens[0][1] == '-') {
        int n = atoi(tokens[0] + 2);
        index = (pos - n + MAX_HISTORY) % MAX_HISTORY;
    }
    // Get index for specific command number
    else {
        int n = atoi(tokens[0] + 1);
        if (n < 1 || n > 20) {
            printf("History reference invalid or out of bounds.\n");
            return;
        }
        int firstCmd = (pos - historyCount + MAX_HISTORY) % MAX_HISTORY;
        index = (firstCmd + (n - 1)) % MAX_HISTORY;
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
    char file[MAX_INPUT];
    strcat(strcpy(file, getenv("HOME")), "/.hist_list");

    // Open file to save history to
    FILE *fp = fopen(file, "w");

    // Check if file opened correctly
    if (!fp) {
        printf("Failed to save history!\n");
        return;
    }

    // Save history array to file
    for (int i = 0; i < MAX_HISTORY; i++) {
        int index = (count + i) % MAX_HISTORY;
        if (history[index] != NULL) {
            fprintf(fp, "%s", history[index]);
            if (history[(index + 1) % MAX_HISTORY] != NULL) {
                fprintf(fp, "\n");
            }
        }
    }

    // Close file
    fclose(fp);
}

void loadHistory( int* count) {
    // Get file path for history file
    char file[MAX_INPUT];
    strcat(strcpy(file, getenv("HOME")), "/.hist_list");

    // Open file to load history from
    FILE *fp = fopen(file, "r");

    // Check if file opened correctly
    if (!fp) {
        printf("Could not find persistent history\n");
        return;
    }

    char buffer[MAX_INPUT];

    // Load history from file
    while (fgets(buffer, MAX_INPUT - 1, fp)) {
        buffer[strcspn(buffer, "\n")] = 0;

        int len = strlen(buffer);
        char* val = malloc((sizeof(char) * len) + 1);
        strcpy(val, buffer);

        history[*count] = val;

        *count = (*count + 1) % MAX_HISTORY;
    }

    // Close file
    fclose(fp);
}

void addAlias(char** tokens) {
    // Check that the correct number of parameters have been passed
    if (tokens[2] == NULL) {
        printf("Error: correct usage is\nalias <name> <command>\n");
        return;
    }

    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i] != NULL && strcmp(aliases[i]->name, tokens[1]) == 0) {
            // Alias already exists
            printf("Overwriting alias %s\n", aliases[i]->name);
            char cmd[MAX_INPUT] = "";
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
            char cmd[MAX_INPUT] = "";
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
            aliases[i]->command = malloc(sizeof(char) * MAX_INPUT);

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

    for (int i = 0; i < MAX_ALIASES; i++) {
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
    for (int i = 0; i < MAX_ALIASES; i++) {
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
    char file[MAX_INPUT];
    strcat(strcpy(file, getenv("HOME")), "/.aliases");

    // Try to open aliases file
    FILE *fp = fopen(file, "w");

    // Check if file opened correctly
    if (!fp) {
        printf("Failed to save aliases!\n");
        return;
    }

    // Write saved aliases to file
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i] != NULL) {
            fprintf(fp, "%s ", aliases[i]->name);
            fprintf(fp, "%s\n", aliases[i]->command);

        }
        else{
            break;
        }
    }

    // Close aliases file
    fclose(fp);
}

void loadAliases() {
    char file[MAX_INPUT];
    strcat(strcpy(file, getenv("HOME")), "/.aliases");

    // Try to open aliases file
    FILE *fp = fopen(file, "r");
    if (!fp) {
        printf("Could not find persistent aliases\n");
        return;
    }

    char buffer[MAX_INPUT];

    // Create alias for each entry in file
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

    // Close aliases file
    fclose(fp);
}
