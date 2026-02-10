#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

void getpath(char** tokens);
void setpath(char** tokens);
void changeDirectory(char** tokens);
void externalCommand(char** tokens);

int main(void) {

    char input[513];
    int exit = 0;

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

        // Split input string into tokens
        int i = 0;
        char *tokens[50] = {NULL};
        char *tok = strtok(input, " \t|><&;");
        while (tok != NULL) {
            tokens[i++] = tok;
            tok = strtok(NULL, " \t|><&;");
        }
        tokens[i] = NULL;

        if (strcmp(tokens[0], "getpath") == 0) {
            getpath(tokens);
        }
        else if (strcmp(tokens[0], "setpath") == 0) {
            setpath(tokens);
        }
        else if (strcmp(tokens[0], "cd") == 0){
            changeDirectory(tokens);
        }
        else {
            externalCommand(tokens);
        }        
    }

    // Restore and print original PATH
    setenv("PATH", originalPath, 1);
    printf("Restored PATH:\n");
    getpath(NULL);

    return 0;
}

void getpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens != NULL && tokens[1] != NULL) {
        printf("Error: getpath takes no parameters\n");
        return;
    }

    // Get and print the current value of PATH
    char* currentPath = getenv("PATH");
    printf("%s\n", currentPath);
}

void setpath(char** tokens) {
    // Check if any parameters were passed in
    if (tokens[2] != NULL || tokens[1] == NULL) {
        printf("Error: setpath takes exactly 1 parameter\n");
        return;
    }

    // Set the value of PATH to tokens[1]
    setenv("PATH", tokens[1], 1);
}

void changeDirectory(char** tokens){
    if(tokens[1] != NULL){
        //To be completed
    }
    else{chdir(getenv("HOME"));}
};

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
