//Discription: A very simple shell to to honor our dead friend harambe.
//Shell features include "exit" to exit the shell, "cd "dir"<no args goes home>", redirect ">"
//and system programs as well as there arguments.
//New additions:
//alias support
//Piping support
//System control
//
//Author: Wesley Strong
//NetID:  wfs51
//Date:   9/21/16
//Please not that no code was directley copied from this site however it was used
//to gain a better understanding as to how a shell functinos thus some things may be similar.
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

//Thanks Andrejs Cainikovs for the ccolor stuff
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
	printf("Succfully created .alias.dat\n");
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
//Src:
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
	FILE *file;
	char **args;
	//Open file
	file = open_file(file);
	//Get lines in file
	*lines = num_line_in_file(file);
	//This is for that behavor thing below
	file = open_file(file);
	//allocate memory
	args = (char **) malloc(80*sizeof(char *)); 

	//Build array
	for (int i = 0 ; i != *lines ; i++)	
	{
		args[i] = read_line(file);
	}
	//Having some very bizzare behivor here
	//When file is closed the command prompt will contain random symbols in random places...
	//fclose(file);
	return args;
}

//Stores data in hash from .alias.dat file
void store_hash()
{
	//Initalize varables for hash then allocate memory
	ENTRY e, *ep;
	char **args;
	int *lines = 0;
	args = (char **) malloc(80*sizeof(char *));	
	args = get_file(&lines);
	char *token;
	//create the hash table base on how many lines are in the file plus 2 incase something happens
	hcreate(lines + 2);
	
	//Build hash table and seprate out the key and data from file
	//alias.dat sytax
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
		ep = hsearch(e, ENTER);
	}
}
//Searches the hash tabe for a match if non then returns NULL
char *find_hash(char *to_find)
{
	ENTRY e, *ep = NULL;
	e.key = to_find;
	ep = hsearch(e, FIND);
	return ep ? ep->data : NULL;
}

//If cd is left NULL then returns to home dir otherwise go to distination.
int harambe_cd(char **args)
{
	//Checks if NULL if true then go to home directory
	if (args[1] == NULL)
	{
		//If there is no home directory rais error
		if (chdir(getenv("HOME")) == -1)
		{
			fprintf(stderr, "%s\n", "ERROR: You dont have a home!");
		}
	}
	//If that directory doesnt exest print this error
	else if (chdir(args[1]) == -1)
	{
		fprintf(stderr, "%s\n", "ERROR: harambe cant go there because that not a directory!");
		return 0;
	}
	else
	{
		//All went well retun 1
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

//The contents of the function are a compinations of a few diffrent sources.
//SRC'S:
//http://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
//http://stackoverflow.com/questions/8516823/redirecting-output-to-a-file-in-c
//Creates a new file and redirects the output stream into the file.
void harambe_redirect(char **args, char output[64])
{
	int fd1;
	if ((fd1 = creat(output, 0644)) < 0) {
		perror("Couldn't open the output file");
		exit(0);
	}
	//fflush(stdout);
	dup2(fd1, STDOUT_FILENO);
	close(fd1);
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
		//cant rember why i did this...
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

		//If there were additional paremter, append them to the list
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

char *alias_insert(char **args)
{
	
	FILE *file;
	file = open_file(file);
	char *to_be_added = malloc(800);

	for (int i = 1 ; args[i] != NULL ; i++)
	{
		printf("test1\n");		
		strcat(to_be_added, args[i]);
		printf("test\n");
		strcat(to_be_added, " ");
		printf("%s\n", args[i]);
	}
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
	        printf( " %s\n", token );
		token = strtok(NULL, separator);
		args[i++] = token;   //and build command array
	}
	args[i] = (char *) NULL;
	return args;
}

//Iterate through the loop to see if there is a shell command or a redirect.
//SRC: http://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
void harambe_builtin(char **args, int *not_builtin, int *out, char output[50])
{
	for (int i = 0; args[i] != '\0'; i++)
	{
		//printf("%s\n", args[i]);
		if (strcmp(args[i], builtin[1]) == 0)
		{
			*not_builtin = 0;
			harambe_exit(args);
		}
		//Had problems with this running eather iteration of args?
		//Im stupid i wasnet accessing the array at the i'th element
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
				printf("%s\n","Harambe needs a filename to name the file!");
			}
			else
			{
				args[i] = NULL;
				strcpy(output, args[i + 1]);
				*out = 1;
			}	
		}
		else if (strcmp(args[0], "alias") == 0)
		{
			alias_insert(alias);
		}
	}
}

//Whenver a system command is need a for is started otherwise do nothing.
void harambe_fork(char **args, int *not_builtin, int *out, char line[81], char output[50], int status)
{
	int pid;

	if (*not_builtin)
	{
		pid = fork();
		if (pid == -1)
		{
			fprintf(stderr, "ERROR harambe can't live!\n");
		}
		else if (pid == 0)
		{
			if (*out)
			{
				harambe_redirect(args, output);
			}
			execvp(args[0], args);
			perror(args[0]);
			//If the system doesnot have the command listed then the child cannot
			//continue there fore this continues to execute and prints error.
			fprintf(stderr, "ERROR: harambe does not know what to do with child.\n");
			exit(1);
		}
		else
		{
			*out = 0;
			while (wait(&status) != pid);
		}
	}
	else
	{
		*not_builtin = 1;
	}
}

//Builds the prompt whenever called
char *harambe_build_prompt()
{
    char *cwd = get_current_dir_name();
    size_t sizex;
    char *user = getenv("USER");
    char hostname[50];
    char *prompt;

    //find last / and remote it
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
        fprintf(stderr, "%s\n", "Harambe does not know who s using this computer!");
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

int main()
{
	int not_builtin = 1;
	char line[81];
	char **args;
	int status;
	int out = 0;
	char output[50];
	char *prompt;

	store_hash();
    	prompt = harambe_build_prompt();

    	//max 80 tokens in line
	args = (char **) malloc(80*sizeof(char *));

	//print inital prompt.
	fprintf(stderr, "%s", prompt);
	while (fgets(line, 80, stdin) != NULL) {

		args = build_args(args, line);
		args = alias(args);
		harambe_builtin(args, &not_builtin, &out, output);
		harambe_fork(args,&not_builtin, &out, line, output, status);

		//Print prompt after everything has finished as will as rebuild in case directory was changed.
		prompt = harambe_build_prompt();
        fprintf(stderr, "%s", prompt);
	}
	//Delete hash cleanin up :)
	hdestroy();
	//Have a nice day!
	exit(0);

} //end main
