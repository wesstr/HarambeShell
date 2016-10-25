//Description: A very simple shell to honor our dead friend harambe.
//Shell features include "exit" to exit the shell, "cd "dir"<no args goes home>", redirect ">"
//and system programs as well as there arguments.
//TO DO:
//alias support (Works kind of...)
//Piping support (Single pipe only)
//System control (Only a single background process)
//
//Author: Wesley Strong
//NetID:  wfs51
//Date:   9/21/16
//Please not that no code was directly copied from this site however it was used
//to gain a better understanding as to how a shell functions thus some things may be similar.
//https://brennan.io/2015/01/16/write-a-shell-in-c/

//Declarations
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <search.h>
#include <signal.h>
#include <stdbool.h>
//Thanks Andrejs Cainikovs for the color stuff
//SRC: http://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//List of built in shell commands.
char *builtin[] = {"cd","exit"};

//Creates a file ".alias.dat"
void create_file()
{
	//Because im lazy i did not pass the name of the file here
	FILE *file;
	file = fopen(".alias.dat", "ab+");
	if (file == NULL)
	{
		printf("%s\n", "Weaning: Harambe could not create .alias.dat");
	}
	printf("Successfully created .alias.dat\n");
	fclose(file);
}

//Opens ".alias.dat"
FILE *open_file(FILE *file)
{
	file = fopen(".alias.dat", "r");
	if (file == NULL)
	{
		printf("%s\n", "Warning: .alias.dat not found!");
		printf("%s\n", "Harambe is attempting to create new file...");
		create_file();
		file = open_file(file);
		return file;
	}
	return file;
}

//Opens file for appending
FILE *open_append(FILE *file)
{
	file = fopen( ".alias.dat", "a" );
	return file;
}

//Reads a single line from a file that is passed to it and then returns the line
char *read_line(FILE *file)
{
	char *buff;
	buff = (char *) malloc(266);

	if (fgets(buff,266,file) == NULL){
		printf("%s\n", "Error reading file!");

		return NULL;
	}
	return buff;
}

//Gets number of lines in file and returns them
//Src: cant remember if there was a source...
int num_line_in_file(FILE *file)
{
	int lines = 0;
	char b;
	while(!feof(file))
	{
		b = fgetc(file);
		if(b == '\n') {
		lines++;
		}
	}
	return lines;
}

//Gets all lines in a file the returns them
char **get_file(int *lines)
{
	FILE *file = NULL;
	char **args;
	//Open file
	file = open_file(file);
	//Get lines in file
	*lines = num_line_in_file(file);
	//This is for that behaver thing below
	file = open_file(file);
	//allocate memory
	args = (char **) malloc(80*sizeof(char *));

	//Build array
	for (int i = 0 ; i != *lines ; i++)
	{
		args[i] = read_line(file);
	}
	//Having some very bizarre behavior here
	//When file is closed the command prompt will contain random symbols in random places...
	//fclose(file);
	return args;
}

//Stores data in hash from .alias.dat file
void store_hash()
{
	//Initialize variables for hash then allocate memory
	ENTRY e;
	char **args;
	int lines = 0;
	args = (char **) malloc(80*sizeof(char *));
	args = get_file(&lines);
	char *token;
	//create the hash table base on how many lines are in the file plus 2 encase something happens
	hcreate(lines + 2);

	//Build hash table and separate out the key and data from file
	//alias.dat syntax
	//command=replace with
	//EX
	//ls=ls -a -l
	//Note: No spaces at end of line as well as no extra lines
	for (int j = 0 ; j != lines ; j++)
	{
		token = strtok(args[j],"=");
		e.key = token;
		token = strtok(NULL, "=");
		token[strlen(token) - 1] = 0;
		e.data = token;
		hsearch(e, ENTER);
	}
}
//Searches the hash table for a match if non then returns NULL
char *find_hash(char *to_find)
{
	ENTRY e, *ep = NULL;
	e.key = to_find;
	ep = hsearch(e, FIND);
	return ep ? ep->data : NULL;
}

//If cd is left NULL then returns to home dir otherwise go to destination.
int harambe_cd(char **args)
{
	//Checks if NULL if true then go to home directory
	if (args[1] == NULL)
	{
		//If there is no home directory otherwise error
		if (chdir(getenv("HOME")) == -1)
		{
			fprintf(stderr, "%s\n", "ERROR: You don't have a home!");
		}
	}
	//If that directory doesn't exist print this error
	else if (chdir(args[1]) == -1)
	{
		fprintf(stderr, "%s\n", "ERROR: harambe cant go there because that not a directory!");
		return 0;
	}
	else
	{
		//All went well return 1
		return 1;
	}
	//Something failed return 0
	return 0;
}

//Exits shell
int harambe_exit(char **args)
{
	hdestroy();
	exit(0);
	return 0;
}

//The contents of the function are a combinations of a few different sources.
//SRC'S:
//http://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
//http://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c
//Creates a new file and redirects the output stream into the file.
void harambe_redirect(char **args, char **output, bool builtin_flags[5])
{
	int fd1,fd2,fd3;


	if (builtin_flags[0])
	{

		if ((fd1 = open(output[0],O_RDWR | O_CREAT , 0666)) < 0) {
			perror("Couldn't open the output file\n");
			exit(0);
		}
		dup2(fd1, STDOUT_FILENO);
		close(fd1);
	}
	if (builtin_flags[2])
	{
		if((fd2 = open(output[2], 0644)) < 0)
		{
			perror("Could not open the file!\n");
		}
		dup2(fd2, STDIN_FILENO);
	}
	if (builtin_flags[1])
	{
		if((fd3 = open(output[1] ,O_WRONLY | O_APPEND)) < 0)
		{
			perror("Could not open file!\n");
		}
		dup2(fd3, STDOUT_FILENO);
	}
}

//Rebuilds args if an alias was found in has
char **alias(char **args)
{
	char *replace;
	char *token;
	char **to_replace;

	//Not sure why malloc here
	replace = malloc(300);
	//I am sure here however...
	to_replace = (char **) malloc(900*sizeof(char *));

	//checks if alias was found
	if (find_hash(args[0]) != NULL ){
		//cant remember why i did this...
		strcpy(replace, find_hash(args[0]));
		//rebuild form alis gotten from file
		token = strtok(replace , " \n");
		int i = 0;
		to_replace[i++] = token;

		while ( (token = strtok(NULL, " \n")) != NULL )
		{
			to_replace[i++] = token;
		}

		int j = 0;

		//If there were additional parameter, Append them to the list
		for (j = 1 ; args[j] != NULL ; j++)
		{
			to_replace[i++] = args[j];
		}

		//Total number of tokens, then set end of file to NULL
		j = j + i;
		to_replace[j] = (char *) NULL;

		return to_replace;
	}
	else {
		return args;
	}
}

//Inserts new alias into the file
void alias_insert(char **args)
{

	FILE *file = NULL;
	file = open_append(file);
	char *to_be_added = malloc(800);

	for (int i = 1 ; args[i] != NULL ; i++)
	{
		strcat(to_be_added, args[i]);
		strcat(to_be_added, " ");
	}
	strcat(to_be_added, "\n");
	fputs(to_be_added, file);
	fclose(file);
}

//Builds the args variable to contain no spaces.
char **build_args(char **args, char line[81])
{
	int i;
	char *token;
	char *separator = " \t\n";

	token = strtok(line, separator);

	i = 0;
	args[i++] = token;  //build command array
	while (token != NULL) //walk through other tokens
	{
		token = strtok(NULL, separator);
		args[i++] = token;   //and build command array
	}
	args[i] = (char *) NULL;
	return args;
///Src:http://stackoverflow.com/questions/2085302/printing-all-environment-variables-in-c-c
}

void print_env(char **envp)
{
	char** env;

	  for (env = envp; *env != 0; env++)
	  {
		      char* thisEnv = *env;
		          printf("%s\n", thisEnv);

	  }
	    return;
}

//This was an attempt to try and implement job processing... 
//DOES NOT WORK!!
void print_jobs(int *jobs)
{
	for (int i = 0 ; jobs[i] != '\0' ; i++)
	{
		printf("%s%i%s%s%i\n","[",i,"]", " pid: ",jobs[i]);
	}
}

//Iterate through the loop to see if there is a shell command or a redirect.
//SRC: http://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
//WARNING OPEN THIS FOLD AT YOUR OWN RISK!!!
char **harambe_builtin(char **args, int *not_builtin, bool builtin_flags[4], char **output, char** envp, int *jobs, int *pip_count)
{
	//A possible different solution to searching the args array for building commands
//	for (int i = 0; args[i] != NULL; i++)
//	{
//		for (int j = 0; builtin[j] != NULL; j++)
//		{
//			if (strcmp(builtin[j], args[]))
//		}
//	}
	int i;

	for (i = 0; args[i] != '\0'; i++)
	{
		//printf("%s\n", args[i]);
		if (strcmp(args[i], builtin[1]) == 0)
		{
			*not_builtin = 0;
			harambe_exit(args);
		}
		//Had problems with this running either iteration of args?
		//I'm stupid I wasn't accessing the array at the i'th element
		else if (strcmp(args[i], builtin[0]) == 0)
		{
			*not_builtin = 0;
			harambe_cd(args);
		}
		//sets the out flag to 1 and removes the > symbol from the args array
		else if (strcmp(args[i], ">") == 0)
		{
			if (args[i + 1] == NULL || strcmp(args[i + 1], " ") == 0)
			{
				printf("%s\n","Harambe needs a filename to name the file or open one!");
			}
			else
			{
				//remove > from string
				args[i] = NULL;
				//move file name to output
				output[0] = malloc(sizeof(args[i+ 1]));
				strcpy(output[0], args[i + 1]);
				builtin_flags[0] = true;
			}
		}
		else if (strcmp(args[i], ">>") == 0)
		{
			//2 means append to end of file
			args[i] = NULL;
			output[1] = malloc(sizeof(args[i+ 1]));
			strcpy(output[1], args[i + 1]);
			builtin_flags[1] =  true;
		}
		else if(strcmp(args[i], "<") == 0)
		{
			//input redirect
			//3 meand input redirect
			args[i] = NULL;
			output[2] = malloc(sizeof(args[i+ 1]));
			strcpy(output[2], args[i + 1]);
			builtin_flags[2] = true;
		}

		//PIPING WOOO!!!
		else if (strcmp(args[i], "|") == 0)
		{
			args[i] = NULL;
			builtin_flags[3] = true;
			*pip_count = *pip_count + 1;

		}
	//If something is first element raise flags 
	}
	if (strcmp(args[0], "alias") == 0)
	{
		alias_insert(args);
		//hdestroy();
		args = alias(args);
		*not_builtin = 0;
	}

	//If & is at end then set flag for back group process
	if (strcmp(args[i - 1], "&") == 0 )
	{
		builtin_flags[4] = true;
		args[i - 1] = NULL;
	}

	//Print environment variables
	if (strcmp(args[0], "env") == 0)
	{
		print_env(envp);
		*not_builtin = 0;
	}

	//Send process to background (Not working!)
	if (strcmp(args[0], "bg") == 0){
		builtin_flags[4] = true;
		
		*not_builtin = 0;
	}

	//Get process into foreground (Not working!)
	if (strcmp(args[0], "fg") == 0){
		builtin_flags[4] = false;
		*not_builtin = 0;
	}

	//Print jobs and there pid's (Not working!)
	if (strcmp(args[0], "jobs") == 0){
		print_jobs(jobs);
		*not_builtin = 0;
	}

	return args;

}

pid_t pid, pid2;
//Whenever a system command is need a for is started otherwise do nothing.
void harambe_fork(char **args, int *not_builtin, bool builtin_flags[5], char line[81], char **output, int *jobs, int *job_count, int* pipe_count)
{
	pid_t sid;
	int status, status2;
	int pipe_disc[2];
	char ***pipe_cmds;
	//Allocate memory first elements of array
	pipe_cmds = malloc(sizeof(char *));
	//pip_cmd_tmp = (char **) malloc(80*sizeof(char *));
	int k;
	int i = 0;

	//If pipe has been entered the need to build an additional args
	if (builtin_flags[3])
	{
		while ( k != (*pipe_count = *pipe_count + 1 )){
			//pipe() is not right here need to fix...
			status = pipe(pipe_disc);
			//Allocate memory for that pipe cmd and its args
			pipe_cmds[k] = (char **) malloc(80*sizeof(char *));
			
			//Separate out commands into array
			//Get to first NULL in cmd
			for (;args[i] != NULL ; i ++);
			i++;
			//Now add them to the array
			int j;
			for (j = 0 ; args[i] != NULL ; j++, i++){
				pipe_cmds[k][j] = malloc(sizeof(args[i]));
				strcpy(pipe_cmds[k][j], args[i]);
			}
			pipe_cmds[k][j++] = (char *) NULL;

			if ( k != (*pipe_count = *pipe_count + 1 )){
				//Not at the end yet got to go up 1
				i++;
			}
			//args[i++] = NULL;
			k++;
		}
	}
	
	if (*not_builtin)
	{
		pid = fork();
		//If fork fails
		if (pid == -1)
		{
			fprintf(stderr, "ERROR harambe can't live!\n");
		}
		// FIRST CHILD HERE
		else if (pid == 0)
		{
			//For IO redirect
			harambe_redirect(args, output, builtin_flags);

			//For background processing
			if (builtin_flags[4])
			{
				sid = setsid();
			}

			//If piping redirect IO accordingly
			if (builtin_flags[3])
			{
				dup2(pipe_disc[1],1);
				close(pipe_disc[0]);
				execvp(args[0], args);
				perror(args[0]);

				printf("1st child failed!\n");
				fprintf(stderr, "ERROR1: harambe does not know what to do with child.\n");
				exit(1);


				
			} else { 

			//Execute normally if no flags etc
			execvp(args[0], args);
			perror(args[0]);
			}
			
			//If the system doesnot have the command listed then the child cannot
			//continue there fore this continues to execute and prints error.
			printf("1st child failed!\n");
			fprintf(stderr, "ERROR1: harambe does not know what to do with child.\n");
			exit(1);
		}
		// PARENT HERE STUPID!
		else
		{
			//If creating a  background process then don't need to wait...
			if (builtin_flags[4]){
				printf(" [ %d ] \n", pid);
				jobs[*job_count] = pid;
				*job_count = *job_count + 1;
				
			}
			//Anything else then harambe will have to wait till the child finishes
			else{
				//SECOND CHILD HERE
				//If pipe then need to make other for and redirect IO accordingly
				if (builtin_flags[3])
				{
					pid2 = fork();
					if (pid2 == -1){
						fprintf(stderr, "ERROR harambe can't live!\n");
					} 

					if (pid2 == 0)
					{
						//Do IO redirecting and execute
						dup2(pipe_disc[0], 0);
						close(pipe_disc[1]);
						//Execute process
						execvp(pip_cmd_tmp[0], pip_cmd_tmp);
						perror(pip_cmd_tmp[0]);
			
						//If the system doesn't have the command listed then the child cannot
						//continue there fore this continues to execute and prints error.
						printf("2nd child failed!\n");
						fprintf(stderr, "ERROR: harambe does not know what to do with child.\n");
						exit(1);
					}
					else {
					//Close everything from piping
					close(pipe_disc[0]);
					close(pipe_disc[1]);
					//wait for second child to finish
					while (wait(&status2) != pid2);
					
					}	
				} else {
					//Wait for first child to finish
					while (wait(&status) != pid);

				
				}
			}
			//Change all flags to false because we are done!
			for (int i = 0; i != 6; i++){
				builtin_flags[i] = false;
			}
		}
	}
	else
	{
		//Child does not change memory in parent stupid!
		*not_builtin = 1;
	}
}

//Builds the prompt whenever called
char *harambe_build_prompt()
{
    char *cwd;
    size_t sizex;
    char *user = getenv("USER");
    char hostname[50];
    char *prompt;

    cwd = malloc(500);
    cwd = getcwd(cwd, 500);

    //find last / and remove it
    cwd = 1 + strrchr(cwd,'/');

    //allocate memory for prompt
    sizex = sizeof(user) + sizeof(cwd) + sizeof(hostname) + 4;
    prompt = (char *) malloc(10*sizex);

    //Some basic error checking
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        fprintf(stderr, "%s\n", "Harambe could not find hostname!");
    }
    if (cwd == NULL)
    {
        fprintf(stderr, "%s\n", "Harambe does not know what to do with directory!");
    }
    if(user == NULL)
    {
        fprintf(stderr, "%s\n", "Harambe does not know who is using this computer!");
    }

    //Build prompt
    strcat(prompt, ANSI_COLOR_BLUE);
    strcat(prompt, user);
    strcat(prompt, ANSI_COLOR_RESET);
    strcat(prompt, "@");
    strcat(prompt, ANSI_COLOR_RED);
    strcat(prompt, hostname);
    strcat(prompt, ANSI_COLOR_RESET);
    strcat(prompt, " ");
    strcat(prompt, ANSI_COLOR_MAGENTA);
    strcat(prompt, cwd);
    strcat(prompt, "% ");
    strcat(prompt, ANSI_COLOR_RESET);

    return prompt;
}

//ctrl + c signal function
int main(int argc, char **argv, char** envp);
void signal_handle()
{
	//Simply call main?
	//I have no idea if this is the correct way to do this...
	//May cause errors and process that are running in the background...
	//A more graceful way of restarting the shell.
	if ( pid != 0 )
		kill(pid, SIGKILL);
	//May not be needed...
	//But just in case something goes wrong in the shell

}

//Contains the history of the last 10 commands entered
char **command_his;
int *jobs;
//Signal handle that creates a log file with the last 10 commands
//Every time a command is entered
void log_file()
{
	FILE *file;
	file = fopen("audit.log","w+");

	for (int i = 9; command_his[i] != NULL && i != -1 ; i--)
		fputs(command_his[i], file);
	fclose(file);
}

//No clue why this is not working...
//Ctrl + z signal function
void pause_process()
{
	kill(pid, SIGSTOP);
	printf("%s%i\n", "Stopped: ", pid);

	for (int i = 0 ; jobs[i] != '\0' ; i++)
	{
		printf("%s%i%s%s%i\n","[",i,"]", " pid: ",jobs[i]);
	}
}

//Stores last 10 commands if there more than 10 then it nulls the last then
//Adds the latest to the first of the list
//
void command_his_store(int *count, char *line, char **command_his)
{
	if(*count == 0) {
		for (int k = 9; k > 0; k--){   
			command_his[k]=command_his[k-1];
		}	
		command_his[0] = malloc(sizeof(line));
		strcpy(command_his[0], line);
	}
	else{
		*count = *count -  1;		
		command_his[*count] = malloc(sizeof(line));
		strcpy(command_his[*count], line);
	}
}

//If you don't know what main is then you shouldn't be looking at this code...
int main(int argc, char **argv, char** envp)
{
	int not_builtin = 1;
	char **args;
	char **output;
	char *prompt;
	bool builtin_flags[5] = {false,false,false,false,false};
	char line[81];
	int count = 10;
	int pip_count = 0;
	int job_count;
	
	//Ctrl + c signal
	signal(SIGINT, signal_handle);
	//USR1 signal
	signal(SIGUSR1, log_file);
	//Ctrl + z signal (does not work)
	signal(SIGTSTP,pause_process);

	//Store alias to hash from file.
	store_hash();
	//Build prompt
    	prompt = harambe_build_prompt();

    	//max 80 tokens in line
	args = (char **) malloc(80*sizeof(char *));
	output = (char **) malloc(80*sizeof(char *));
	command_his = (char **) malloc(81*sizeof(char *));

	jobs = (int*) calloc(4, sizeof(int));

	//print initial prompt.
	fprintf(stderr, "%s", prompt);
	while (fgets(line, 80, stdin) != NULL) {

		//Store command entered
		command_his_store(&count, line, command_his);
		//Call USR1 signal
		kill(getpid(), SIGUSR1);

		args = build_args(args, line);
		args = alias(args);
		args = harambe_builtin(args, &not_builtin, builtin_flags, output, envp, jobs, &pip_count);
		harambe_fork(args, &not_builtin, builtin_flags, line, output, jobs, &job_count, &pip_count);

		//Print prompt after everything has finished as will as rebuild in case directory was changed.
		prompt = harambe_build_prompt();
        fprintf(stderr, "%s", prompt);
	}
	//Delete hash cleaning up :)
	hdestroy();
	//Have a nice day!
	exit(0);

} //end main
