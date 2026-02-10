#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

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
        char *tokens[50];
        char *tok = strtok(input, " \t|><&;");
        while (tok != NULL) {
            tokens[i++] = tok;
            tok = strtok(NULL, " \t|><&;");
        }
        tokens[i] = NULL;

        externalCommand(tokens);
    }

    // Restore original PATH
    setenv("PATH", originalPath, 1);

    return 0;
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