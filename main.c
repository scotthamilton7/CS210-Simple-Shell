#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void processInput(char* input, char** history, int count);
void getpath(char** tokens);
void setpath(char** tokens);
void changeDirectory(char** tokens);
void externalCommand(char** tokens);
void printHistory(char** history, int count);
void invokeHistory(char** tokens, char** history, int count);

int main(void) {

    char input[513];
    char* history[20] = {NULL};
    int exit = 0;
    int count = 0;

    // Save original PATH
    char* originalPath = getenv("PATH");

    // Set current directory to HOME
    char* userHomeDir = getenv("HOME");
    chdir(userHomeDir);
    
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
        
        // Remove newline from input
        input[strcspn(input, "\n")] = 0;
        
        // Check if input is "exit", if it is then quit the shell
        if (strcmp(input, "exit") == 0) {
            exit = 1;
            break;
        }

        // Add command to history
        if (strcspn(input, "!") != 0) {
            history[count] = strdup(input);
            count = (count + 1) % 20;
        }

        processInput(input, history, count);
    }

    // Restore and print original PATH
    setenv("PATH", originalPath, 1);
    printf("Restored PATH:\n");
    getpath(NULL);

    return 0;
}

void processInput(char* input, char** history, int count) {
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
        printHistory(history, count);
    }
    else if (strcspn(input, "!") == 0) {
        invokeHistory(tokens, history, count);
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
        printf("Error: Not enough arguments. setpath requires 1 paramater (the path)\n");
        return;
    }

    // Set the value of PATH to tokens[1]
    setenv("PATH", tokens[1], 1);
}

void changeDirectory(char** tokens){
    // Check if too many paramaters have been passed
    if (tokens[2] != NULL) {
        printf("Error: Too many arguments. cd takes either 0 or 1 paramaters\n");
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

void printHistory(char** history, int count) {
    // Print all items in the history array
    int num = 1;
    for (int i = 0; i < 20; i++) {
        int index = (count + i) % 20;
        if (history[index] != NULL) {
            printf("%2d %s\n", num++, history[index]);
        }
    }
}

void invokeHistory(char** tokens, char** history, int count) {
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
    processInput(cmd, history, count);
    free(cmd);
}