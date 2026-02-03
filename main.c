#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
        char *tokens[32];
        char *tok = strtok(input, " \t|><&;");
        while (tok != NULL) {
            tokens[i++] = tok;
            printf("\"%s\"\n", tok); // remove
            tok = strtok(NULL, " \t|><&;");
        }
    }

    return 0;
}
