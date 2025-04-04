#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>

#define PATH_MAX 50
#define HISTORY_FILE ".zoid_history"
#define MAX_ALIAS 100

char *HOME;
char cwd[PATH_MAX];
char **get_input(char *);
const char *prompt_color = "\033[1;34m";
const char *error_color = "\033[1;31m";
const char *prompt_default = "\033[0m"; 

typedef struct {
  char *alias;
  char *command;
} Alias;

Alias alias_table[MAX_ALIAS];
int alias_count = 0;

void init_home() {
  HOME = getenv("HOME");
}

void set_alias(const char *alias, const char *command) {
  if(alias_count < MAX_ALIAS) {
    alias_table[alias_count].alias = strdup(alias);
    alias_table[alias_count].command = strdup(command);
    alias_count++;
  }

  else {
    fprintf(stderr, "Max alias limit reached!\n");
  }
}

const char *resolve_alias(const char *input) {
  for(int i = 0; i < alias_count; i++) {
    if(strcmp(input, alias_table[i].alias) == 0) {
      return alias_table[i].command;
    }
  }
  return input;
}

int cd(char *path) {
  return chdir(path);
}

int main() {
    init_home();
    set_alias("ls", "ls --color=auto");

    read_history(HISTORY_FILE);
    stifle_history(100);
    
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    char prompt[PATH_MAX + 10];

    while (1) {
      if(getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, sizeof(prompt), "%s(%szoid%s) %s ~> ", prompt_default, prompt_color, prompt_default, cwd);
        input = readline(prompt);
        
        if(strlen(input) > 0) {
          add_history(input);
        }

        if(strlen(input) == 0) {
          free(input);
          continue;
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
        if (command) {
          for (int i = 0; command[i] != NULL; i++) {
          free(command[i]);
        }
        free(command);
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

        if (strcmp(command[0], "exit") == 0) {
          free(input);
          free(command);
          exit(0);
        }

        child_pid = fork();
        if (child_pid == 0) {
            // never returns if the call is successful 
            execvp(command[0], command);
            printf("%s%s was not recognized as a valid command.\n", error_color, command[0]);
            _exit(1);
        }

		else {
            waitpid(child_pid, &stat_loc, WUNTRACED);
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
		command[index] = strdup(parsed);
		index++;

		parsed = strtok(NULL, seperator);
	}

	command[index] = NULL;
	return command;
}

