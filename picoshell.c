#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define BUFF_SIZE 40000
#define MAX_ARGS 9999

char *lvarS[5000];
char *lvarV[5000];
int lvarpos;
int main(int argc, char *argv[])
{
	char buf[BUFF_SIZE];
	int last_status = 0;

	while (1) {
		printf("Your only friend is here ;) > ");
		fflush(stdout);

		if (fgets(buf, BUFF_SIZE, stdin) == NULL) {
			break;
		}

		size_t len = strlen(buf);
		if (len > 0 && buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
		}

		if (buf[0] == '\0') {
			continue;
		}
		if (strcmp(buf, "exit") == 0) {
			printf("Good Bye\n");
			break;
		}

		char *args[MAX_ARGS];
		int i = 0;
		args[i] = strtok(buf, " ");
		while (args[i] != NULL && i < MAX_ARGS - 1) {
			i++;
			args[i] = strtok(NULL, " ");
		}
		args[i] = NULL;
	
		i = 1;
		while(args[i] != NULL){
			if(strchr(args[i],'$') != NULL){
				char *p = strchr(args[i],'$') + 1;
				int counter = 0;
				for(counter = 0;counter <= lvarpos;counter++){
					if(*p == lvarV[counter]){
						args[i] = lvarV[counter];
					}
					break;
				}
				i++;
			}

		if (strcmp(args[0], "cd") == 0) {
			if (args[1] == NULL
			    || (last_status = chdir(args[1])) == -1)
				printf("cd: %s: No such file or directory\n",
				       args[1]);
			//exit(0);
			continue;
		} else if (strchr(args[0], '=') != NULL) {
			char *pos = strchr(args[0], '=');
			*pos = '\0';
			char *sympol = args[0];
			char *val = pos + 1;
			lvarS[lvarpos] = strdup(sympol);
			lvarV[lvarpos] = strdup(val);
			lvarpos++;

			continue;
		}
		else if((strcmp("echo",args[0]) == 0) && (strchr(args[1],'$') != NULL)){
			char *ptr = strchr(args[1],'$');
			ptr++;
			int spos = -1;
			int i = 0;
			for(i = 0; i <= lvarpos;i++){
				if(strcmp(lvarS[i] , ptr) == 0){
					spos = i;
					break;
				}
			}
			if(spos != -1){
				printf("%s\n",lvarV[spos]);
			}
			else{
				printf("Sympol not found\n");
			}
			continue;
		}
			

		pid_t pid = fork();
		if (pid < 0) {
			last_status = 1;
			continue;
		}

		if (pid == 0) {

			execvp(args[0], args);
			printf("%s: command not found\n", args[0]);
			exit(-1);

			exit(1);
		} else {

			int status;
			waitpid(pid, &status, 0);
			if (WIFEXITED(status))
				last_status = WEXITSTATUS(status);
			else
				last_status = 1;
		}
	}

	return last_status;
}
