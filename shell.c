#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>


//Reads line from the user
char* readline(){
	char *line = NULL;
	size_t size = 1024;

	fflush(stdout);
	printf(">");
	getline(&line,&size,stdin);

	return line;
}

//Spliting the line into different arguments
char** splitline(char* line){
	int size = 100;
	char delim[2] = " ";
	char *split;
	char  **args = malloc(sizeof(char*)*size);

	split = strtok(line,delim);

	int ctr=0;
	while(split!=NULL){
		args[ctr] = split;
		ctr+=1;

		if(ctr>=size);
		size+=size;
		args = realloc(args,size*sizeof(char*));

		split = strtok(NULL, delim);

	}
	args[ctr]=NULL;

	return args;
}

int execute(char **args){
	pid_t pid;
	int wait, status;

	pid = fork();

	if(pid == 0){
		execvp(args[0],args);
	}
	else{
		wait = waitpid(pid,&status,WIFEXITED(status));
	}
}

int main(){

	while(1){
		char *line=NULL;
		char **args=NULL;
		line = readline();
		args = splitline(line);
		execute(args);
		free(args);
		free(line);
	}	

	return 0;
}