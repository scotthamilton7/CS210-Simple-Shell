#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {

    char input[513];
    int exit = 0;
    
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
            //printf("\"%s\"\n", tok); // remove
            tok = strtok(NULL, " \t|><&;");
        }

        // 
        pid_t p = fork();
        if (p < 0) {
            printf("Error forking\n");
        }
        else if (p == 0) {
            // child process
            char path[100] = "/bin/";

            strcat(path, tokens[0]);

            execvp(path, tokens);
        }
        else if (p > 0) {
            // Wait for child process to finish
            wait(NULL);
        }
    }

    return 0;
}
