#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>

#define PATH_MAX 1024
#define CMD_MAX 1024
#define HISTORY_FILE ".zoid_history"

char cwd[PATH_MAX];
char **get_input(char *);

int cd(char *path) {
  return chdir(path);
}

int main() {
    read_history(HISTORY_FILE);
    stifle_history(100);
    
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    char prompt[PATH_MAX + 10];

    while (1) {
      if(getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, sizeof(prompt), "(zoid) %s ~> ", cwd);
        input = readline(prompt);
        
        if(strlen(input) > 0) {
          add_history(input);
        }

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

        if (strcmp(command[0], "clrhistory") == 0) {
          clear_history();
          remove(HISTORY_FILE);

          free(input);
          free(command);
          continue;
        }

        // piping
        int pipe_count = 0;
        char *pipe_segment = strtok(input, "|");
        char *pipe_commands[10];

        while(pipe_segment != NULL) {
          pipe_commands[pipe_count++] = pipe_segment;
          pipe_segment = strtok(NULL, "|");
        }

        int pipefds[2 * (pipe_count - 1)];
        
        for (int i = 0; i < pipe_count - 1; i++) {
            pipe(pipefds + i * 2);  
        }

        for (int i = 0; i < pipe_count; i++) {
            child_pid = fork();

            if (child_pid == 0) {
                char **cmd = get_input(pipe_commands[i]);

                if (i > 0) {
                    dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
                }

                if (i < pipe_count - 1) {
                    dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
                }

                for (int j = 0; j < 2 * (pipe_count - 1); j++) {
                    close(pipefds[j]);
                }

                execvp(cmd[0], cmd);
                printf("%s was not recognized as a valid command.\n", command[0]);
                exit(1);
            }
        }

        for (int i = 0; i < 2 * (pipe_count - 1); i++) {
            close(pipefds[i]);
        }

        for (int i = 0; i < pipe_count; i++) {
            wait(&stat_loc);
        }

        write_history(HISTORY_FILE);
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

