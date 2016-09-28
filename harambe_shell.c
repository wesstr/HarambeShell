//Discription: A very simple shell to to honor our dead friend harambe.
//Shell features include "exit" to exit the shell, "cd "dir"<no args goes home>", redirect ">"
//and system programs as well as there arguments.
//Author: Wesley Strong
//NetID:  wfs51
//Date:   9/21/16
//Please not that no code was directley copied from this site however it was used
//to gain a better understanding as to how a shell functinos thus some things may be similar.
//https://brennan.io/2015/01/16/write-a-shell-in-c/

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
	fflush(stdout);
	dup2(fd1, STDOUT_FILENO);
	close(fd1);
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
	   // printf( " %s\n", token );
		token = strtok(NULL, separator);
		args[i++] = token;   //and build command array
	}
	args[i] = (char *) NULL;
	return args;
}

//Iterate through the loop to see if there is a shell command or a redirect.
//SRC: http://stackoverflow.com/questions/11515399/implementing-shell-in-c-and-need-help-handling-input-output-redirection
void harambe_builtin(char **args, int *not_builtin, int *out, char *output)
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
			args[i] = NULL;
			strcpy(output, args[i + 1]);
			*out = 1;
		}
	}
}

//Whenver a system command is need a for is started otherwise do nothing.
void harambe_fork(char **args, int *not_builtin, int *out, char line[81], char *output, int *status)
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
			fprintf(stderr, "ERROR harambe does not know what to do with child.\n");
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
    int *cwd = get_current_dir_name();
    size_t sizex;
	const char *user = getenv("USER");
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
        strcpy(hostname, NULL);
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
	int child_pid;
	char line[81];
	char **args;
	int status;
	int out = 0;
	char *output[50];
	char *prompt;
    prompt = harambe_build_prompt();

    //max 80 tokens in line
	args = (char **) malloc(80*sizeof(char *));

	//print inital prompt.
	fprintf(stderr, "%s", prompt);
	while (fgets(line, 80, stdin) != NULL) {

		args = build_args(args, line);
		harambe_builtin(args, &not_builtin, &out, output);
		harambe_fork(args,&not_builtin, &out, line, output, &status);

		//Print prompt after everything has finished as will as rebuild in case directory was changed.
		prompt = harambe_build_prompt();
        fprintf(stderr, "%s", prompt);
	}
	exit(0);
} //end main
