#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

char* BuiltInCMDS[] = {"cd","help","history","exit","NULL"};


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
		printf("%s\n",temp->command);
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

//**** START - > redirection COMMAND ****

int isRedirectOne(char **args){
	int ctr=1;
	while(args[ctr]!=NULL){
		if(strcmp(args[ctr],">")==0){
			return 1;
		}
		ctr++;
	}
	return 0;
}

void redirectOne(char **args){

	char *cmd=NULL;
	cmd = malloc(1024*sizeof(char));
	strcat(cmd,"");
	int ctr=0;
	while(strcmp(args[ctr],">")!=0){
		strcat(cmd,args[ctr]);
		strcat(cmd," ");
		ctr++;
	}
	ctr++;

	char **argv = splitline(cmd);

	int stdoutCopy = dup(1); 

	int out = open(args[ctr], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

    dup2(out, 1);
    close(out);

	//execvp(argv[0],argv);
	pid_t pid, wid;
	int status;
	pid = fork();
	if(pid<0){
		perror("Forking Error");
		return;
	}
	else if(pid==0){
		int t;
		t = execvp(argv[0],argv);
		if(t==-1){
			dup2(stdoutCopy,1);
    		close(stdoutCopy);
			perror("Error executing command");
			printf("%s\n",argv[0]);
			return;
		}
	}
	else{
	     wid = waitpid(pid, &status, WIFSTOPPED(status));
	}

	dup2(stdoutCopy,1);
    close(stdoutCopy);
   
    return;
}

//**** END - > redirection COMMAND


//**** START - < redirection COMMAND

int isRedirectTwo(char **args){
	int ctr=1;
	while(args[ctr]!=NULL){
		if(strcmp(args[ctr],"<")==0){
			return 1;
		}
		ctr++;
	}
	return 0;
}

void redirectTwo(char **args){

	char *cmd=NULL;
	cmd = malloc(1024*sizeof(char));
	strcat(cmd,"");
	int ctr=0;
	while(strcmp(args[ctr],"<")!=0){
		strcat(cmd,args[ctr]);
		strcat(cmd," ");
		ctr++;
	}
	ctr++;

	char **argv = splitline(cmd);

	int stdinCopy = dup(0); 

	int in = open(args[ctr], O_RDONLY);

    dup2(in, 0);
    close(in);

	//execvp(argv[0],argv);
	pid_t pid, wid;
	int status;
	pid = fork();
	if(pid<0){
		perror("Forking Error");
		return;
	}
	else if(pid==0){
		int t;
		t = execvp(argv[0],argv);
		if(t==-1){
			dup2(stdinCopy,0);
    		close(stdinCopy);
			perror("Error executing command");
			return;
		}
	}
	else{
	     wid = waitpid(pid, &status, WIFSTOPPED(status));
	}

	dup2(stdinCopy,0);
    close(stdinCopy);
   
    return;
}

//**** END - < redirection COMMAND

//**** START - | pipe COMMAND

int isPipe(char **args){
	int ctr=1;
	while(args[ctr]!=NULL){
		if(strcmp(args[ctr],"|")==0){
			return 1;
		}
		ctr++;
	}
	return 0;
}

void pipecmd(char **args){

	char *cmd1=NULL, *cmd2=NULL;
	cmd1 = malloc(1024*sizeof(char));
	cmd2 = malloc(1024*sizeof(char));
	strcat(cmd1,"");
	strcat(cmd2,"");

	//Get the left command
	int ctr=0;
	while(strcmp(args[ctr],"|")!=0){
		strcat(cmd1,args[ctr]);
		strcat(cmd1," ");
		ctr++;
	}
	ctr++;
	char **argv1 = splitline(cmd1);

	//Get the right command
	while(args[ctr]!=NULL){
		strcat(cmd2,args[ctr]);
		strcat(cmd2," ");
		ctr++;
	}
	ctr++;
	char **argv2 = splitline(cmd2);

	//printf("%s %s | %s %s", argv1[0],argv1[1],argv2[0],argv2[1]);

	int fd[2];
	pipe(fd);

	//Copy original file descriptors
	int stdinCopy = dup(0);
	int stdoutCopy = dup(1);


	pid_t pid, wid;
	int status;
	pid_t pid2,wid2;
	int status2;
	pid = fork();
	if(pid<0){
		perror("Forking Error");
		return;
	}
	else if(pid==0){
		//PIPING STUFF HERE
		pid2 = fork();

		if(pid2<0){
			perror("Forking Error");
			return;
		}
		else if(pid2==0){
			//input for right command
			dup2(fd[0],0);
			close(fd[1]);
			int t;
			t = execvp(argv2[0],argv2);
			if(t==-1){
				dup2(stdinCopy,0);
	    		close(stdinCopy);
				perror("Error executing command");
				return;
			}
			dup2(stdinCopy,0);
	    	close(stdinCopy);

		}
		else{
			//wid2 = waitpid(pid2, &status2, WIFSTOPPED(status));

			//output for left command
			dup2(fd[1],1);
			close(fd[0]);
			int t;
			t = execvp(argv1[0],argv1);
			if(t==-1){
				dup2(stdoutCopy,1);
	    		close(stdoutCopy);
				perror("Error executing command");
				return;
			}
			dup2(stdoutCopy,1);
	    	close(stdoutCopy);
		}

	}
	else{
	     wid = waitpid(pid, &status, WIFSTOPPED(status));
	}

}

//**** END - | pipe COMMAND


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

	if(args[0]==NULL){
		return;
	}

	if(isBuiltIn(args[0])){
		execBuiltIn(args);
		return;
	}
	

	if(isRedirectOne(args)){
		redirectOne(args);
		return;
	}

	if(isPipe(args)){
		pipecmd(args);
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


//Handler for Ctrl+C interrupt
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    char *wd=NULL;
	size_t size = 1024;

	wd = getcwd(wd,size);

	printf("\nshellB:%s$ ",wd);
    fflush(stdout);
    return;
}

int main(){
	signal(SIGINT, sigintHandler);
	//system("gnome-terminal");

	while(1){
		char *line;
		char **args;
		line = readline();
		args = splitline(line);
		if(args[0]!=NULL){
			add_history(line);
		}
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