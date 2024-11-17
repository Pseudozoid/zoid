#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>

#define PATH_MAX 50

char cwd[PATH_MAX];
char **get_input(char *);

int cd(char *path) {
  return chdir(path);
}

int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    char prompt[PATH_MAX + 10];

    while (1) {
      if(getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, sizeof(prompt), "(zoid) %s ~> ", cwd);
        input = readline(prompt);
        command = get_input(input);
      }

      else {
        perror("getcwd");
        return 1;
      }

        if (!command[0]) {      // handle empty commands
            free(input);
            free(command);
            continue;
        }

        if (strcmp(command[0], "cd") == 0) {
            if (cd(command[1]) < 0) {
                perror(command[1]);
            }

            /* Skip the fork */
            continue;
        }

        child_pid = fork();
        if (child_pid == 0) {
            // never returns if the call is successful 
            execvp(command[0], command);
            printf("Execution Error!\n");
        }

		else {
            waitpid(child_pid, &stat_loc, WUNTRACED);
        }

        free(input);
        free(command);
    }

    return 0;
}

char **get_input(char *input) {
	const char *seperator = " "; // seperate substrings by space
	int capacity = 8; // initial capacity
	char **command = malloc(capacity * sizeof(char *));
	char *parsed = strtok(input, seperator);
	int index = 0;

	while(parsed != NULL) {
		if(index >= capacity) {
			capacity *= 2;
			command = realloc(command, capacity * sizeof(char *)); // reallocate if command has more than 8 strings
		}
		command[index] = parsed;
		index++;

		parsed = strtok(NULL, seperator);
	}

	command[index] = NULL;
	return command;
}

