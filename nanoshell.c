#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 40000
#define MAX_ARGS 9999
#define MAX_VARS 5000

char *lvarS[MAX_VARS];
char *lvarV[MAX_VARS];
int lvarpos;

void substitute_variables(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        char *dollar = args[i];
        while ((dollar = strchr(dollar, '$')) != NULL) {
            char *var_start = dollar + 1;
            char *var_end = var_start;
            
            // Find end of variable name
            while (*var_end != '\0' && (isalnum(*var_end) || *var_end == '_')) {
                var_end++;
            }
            
            // Extract variable name
            size_t var_len = var_end - var_start;
            char var_name[var_len + 1];
            strncpy(var_name, var_start, var_len);
            var_name[var_len] = '\0';

            // Search local variables
            char *value = NULL;
            for (int j = 0; j < lvarpos; j++) {
                if (strcmp(lvarS[j], var_name) == 0) {
                    value = lvarV[j];
                    break;
                }
            }

            // Fallback to environment variables
            if (!value) value = getenv(var_name);

            // Replace the variable reference
            if (value) {
                size_t prefix_len = dollar - args[i];
                size_t new_len = prefix_len + strlen(value) + strlen(var_end) + 1;
                char *new_arg = malloc(new_len);
                
                strncpy(new_arg, args[i], prefix_len);
                strcpy(new_arg + prefix_len, value);
                strcat(new_arg, var_end);
                
                free(args[i]);
                args[i] = new_arg;
            }

            dollar = var_end;
        }
    }
}

int main(int argc, char *argv[]) {
    char buf[BUFF_SIZE];
    int last_status = 0;

    while (1) {
        printf("Your only friend is here ;) > ");
        fflush(stdout);

        if (fgets(buf, BUFF_SIZE, stdin) == NULL) break;
        buf[strcspn(buf, "\n")] = '\0';

        if (buf[0] == '\0') continue;
        if (strcmp(buf, "exit") == 0) {
            printf("Good Bye\n");
            break;
        }

        // Handle variable assignment
        char *equals = strchr(buf, '=');
        if (equals != NULL && equals != buf) {
            *equals = '\0';
            char *name = buf;
            char *value = equals + 1;

            // Update existing variable if it exists
            int exists = 0;
            for (int i = 0; i < lvarpos; i++) {
                if (strcmp(lvarS[i], name) == 0) {
                    free(lvarV[i]);
                    lvarV[i] = strdup(value);
                    exists = 1;
                    break;
                }
            }
            
            if (!exists && lvarpos < MAX_VARS) {
                lvarS[lvarpos] = strdup(name);
                lvarV[lvarpos] = strdup(value);
                lvarpos++;
            }
            continue;
        }

        // Tokenize command
        char *args[MAX_ARGS];
        int arg_count = 0;
        args[arg_count] = strtok(buf, " ");
        while (args[arg_count] != NULL && arg_count < MAX_ARGS - 1) {
            arg_count++;
            args[arg_count] = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        // Handle built-in commands
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL || chdir(args[1])) {
                printf("cd: %s: No such file/directory\n", args[1]);
            }
            continue;
        }
        else if (strcmp(args[0], "export") == 0) {
            if (args[1]) {
                for (int i = 0; i < lvarpos; i++) {
                    if (strcmp(lvarS[i], args[1]) == 0) {
                        setenv(lvarS[i], lvarV[i], 1);
                        break;
                    }
                }
            }
            continue;
        }

        // Substitute variables in all arguments
        substitute_variables(args);

        // Execute command
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(127);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        } else {
            perror("fork");
        }
    }

    // Cleanup
    for (int i = 0; i < lvarpos; i++) {
        free(lvarS[i]);
        free(lvarV[i]);
    }

    return last_status;
}
