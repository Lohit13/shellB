#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

char* BuiltInCMDS[] = {"cd","help","history","exit","NULL"};

//**** START help COMMAND ****

void help(){
	printf("ShellB, version 0.1-release(1)\n");
	printf("`help` lists the in built commands of this shell\n\n");
	printf("cd : Change directory. Usage: cd path/\n");
	printf("help : Lists the available in built commands\n");
	printf("history : Shows the previously typed commands of the current session\n");
	printf("exit : shuts down shellB\n");
	return;
}

//**** END - help COMMAND ****

//**** START - history COMMAND ****

//Structure for list maintaing history
typedef struct _history{
	char* command;
	struct _history* next;
}history;
history *top=NULL;

//Adds command to linked list of history
void add_history(char* cmd){
	int len = strlen(cmd);
	if(top==NULL){
		top = (history *)malloc(sizeof(history));
		top->command = malloc(len*sizeof(char));
		strcpy(top->command,cmd);
		top->next = NULL;
		return;
	}

	history *temp=top;
	history *temp2=NULL;
	while(temp->next!=NULL){
		temp = temp->next;
	}
	temp2 = (history *)malloc(sizeof(history));
	temp2->command = malloc(len*sizeof(char));
	strcpy(temp2->command,cmd);
	temp2->next = NULL;
	temp->next = temp2;
	return;
}

//Displays the history of commands
void show_history(){
	history *temp=top;
	while(temp!=NULL){
		printf("%s",temp->command);
		temp = temp->next;
	}
	return;
}

//**** END - history COMMAND ****

//**** START - cd COMMAND ****

void cd(char *dir){
	if(chdir(dir)){
		perror("Could not change directory");
	}
	return;
}

//**** END - cd COMMAND ****

//Reads command from user
char* readline(){
	char *line = NULL, *wd=NULL;
	size_t size = 1024;

	wd = getcwd(wd,size);

	printf("shellB:%s$ ",wd);
	getline(&line,&size,stdin);

	return line;
}

//Splits the command into program + arguments
char** splitline(char *line)
{
  int buffer = 64;
  int pos = 0;
  char **args = malloc(buffer * sizeof(char*));
  char *token;

  if (!args) {
    perror("Allocation error");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\r\n\a");
  while (token != NULL) {
    args[pos] = token;
    pos+=1;

    if (pos >= buffer) {
      buffer += buffer;
      args = realloc(args, buffer * sizeof(char*));
      if (!args) {
        perror("Reallocation error");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, " \t\r\n\a");
  }
  args[pos] = NULL;
  return args;
}

int isBuiltIn(char* cmd){
	int i=0;
	while(strcmp(BuiltInCMDS[i],"NULL")!=0){
		if(strcmp(BuiltInCMDS[i],cmd)==0){
			return 1;
		}
		i++;
	}
	return 0;
}

void execBuiltIn(char **args){
	char *cmd = args[0];

	if(strcmp("exit",cmd)==0){
		exit(0);
	}
	if(strcmp("history",cmd)==0){
		if(args[1]!=NULL){
			printf("`history` does not take any arguments. Type `help` for more information\n");
			return;
		}
		show_history();
	}
	if(strcmp("help",cmd)==0){
		if(args[1]!=NULL){
			printf("`help` does not take any arguments. Type `help` for more information\n");
			return;
		}
		help();
	}
	if(strcmp("cd",cmd)==0){
		if(args[1]==NULL || args[2]!=NULL){
			printf("`cd` takes exactly 1 argument. Type `help` for more information\n");
			return;
		}
		cd(args[1]);
	}
	return;
}

//Executes the command
int execute(char** args){
	pid_t pid, wid;
	int status;

	if(isBuiltIn(args[0])){
		execBuiltIn(args);
		return;
	}

	pid = fork();
	if(pid<0){
		perror("Forking Error");
		return 0;
	}
	else if(pid==0){
		int t;
		t = execvp(args[0],args);
		if(t==-1){
			perror("Error executing command");
			return 0;
		}
	}
	else{
	     wid = waitpid(pid, &status, WIFSTOPPED(status));
	}

	return 1;
}



int main(){

	while(1){
		char *line;
		char **args;
		line = readline();
		add_history(line);
		args = splitline(line);
		execute(args);

		/*
		int ctr=0;
		while(arg[ctr]!=NULL){
			printf("%s\n",arg[ctr]);
			ctr++;
		}
		*/


		
	}

	return 0;
}