**Contribution Note:** This project was developed collaboratively and equally by myself and Adam Morrell, Jack Hardy, Jude Graham, and Ethan McGee. Due to technical issues with access to the university's GitLab accounts during development, the Git commit history does not accurately represent each contributor's individual contributions. The commit history should therefore not be used as an indication of the relative contributions of team members.

# Simple Shell Project

The aim of this project was to develop a simple OS shell using C for a Unix-type system. 

# Functionality

The following functionality is supported by the shell (taken from the `simpleShell.pdf` assignment specification file):
1. Execution of external system programs including their parameters.
2. The following built-in commands:
   1. `cd` — change working directory
   2. `getpath` — print system path
   3. `setpath` — set system path
   4. `history` — print history contents (numbered list of commands in history including their parameters in ascending order from least to most recent)
   5. `!!` — invoke the last command from history (e.g. if your last command was ‘ls -lF’ then ‘!!’ should execute ‘ls -lF’)
   6. `!<no>` — invoke command with number <no> from history (e.g. ‘!5’ will execute the command with number 5 from the history)
   7. `!-<no>` — invoke the command with number the number of the current command minus <no> (e.g. ‘!-3’ if the current command number is 5 will execute the command with number 2, or ‘!-1’ will execute again the last command)
   8. `alias` — print all set aliases (alias plus aliased command)
   9. `alias <name> <command>` — alias name to be the command. Note that the command may also include any number of parameters, while (any number of) command parameters should work correctly with aliasing (e.g. if I alias ‘la’ to be ‘ls -la’ then when I type ‘la .’ the shell should execute ‘ls -la .’). Note also that aliasing should also work correctly with history (e.g. ‘!5 will execute the command with number 5 from the history, if this command is an alias like the ‘la’ above then ‘!5’ will execute ‘ls -la’). In the enhanced form of the alias, it should be possible to alias history invocations (e.g. if I alias ‘five’ to be ‘!5’ then when I type ‘five’ the shell should execute the command with number 5 from the history). It should also be possible to alias aliases (e.g. If I alias ‘l’ to be ‘ls’ and then I alias ‘la’ to be ‘l -a’, then when I type ‘la’ the shell should execute ‘ls -a’).
   10. `unalias <name>` — remove any associated alias
3. Persistent history of user commands (save history in a file and load it when you run
the shell again)
4. Persistent aliases (save aliases in a file and load it when you run the shell again)

# Testing

All tests found in the `SimpleShellTests.pdf` file were ran, and passed, during development to ensure proper functionality.

# How to Run
1. Clone the repository
2. Open a terminal in the folder downloaded
3. Run `gcc shell.c list.c -o shell` in the terminal to compile the program
4. Run `./shell` in the same terminal to start the program
