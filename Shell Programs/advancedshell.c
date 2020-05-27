/*Nick Kitchel
  CS 240
  Advanced Shell Assignment
*/

#include <stdio.h>
#include <stdlib.h>     //library for malloc usage
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void makearg(char st[], char*** args);  //token function
int words(char s2[]);                    //function for number of words
void shell_use(char* s, char** input, char** alias);		//useing the shell function
void semi_check(char s[], char** final, char** alias_check);	//check for semi colon
int alias_check(char s[], char*** store, int* a);		//check for alias
int clear_alias(char s[], char*** store, int* a);		//clear alias
int history_check(char s[], char*** array, int count);		//history check
void make_memory(char*** array);	//make memory to fix bugs
int pipe_check(char s[]);	//check for pipe
void piping(char s[]);		//pipe the command
void path(char s[]);		//path useing

int main()
{
	char** storage;		//store the history commands
	int test;		//stores returned value from function
	int check = 1;	//set check equal to one
	int counter = 0;	//history counter
	char** history;		//history storage
	char c[25];			//character reading
	int lines = 0;		//line count
	char** args;		//main user input

	make_memory(&history);		//allocate memory for histroy

	FILE* fp;		//open file mshrc.txt

	fp = fopen("mshrc.txt", "r");

	while (fgets(c, 10, fp) != NULL)		//while not at the end
	{
		lines++;		//count lines
	}

	fclose(fp);		//close it

	fp = fopen("mshrc.txt", "r");		//reopen from beginning
	fscanf(fp, "%[^\n]", c);		//read first line

	int i;
	for(i = 0; i < lines; i++)
	{

		if (strlen(c) > 0)			//check for input
		{			
				if (check == 0)		//test if alias should be called
					test = clear_alias(c, &storage, &check);		//clear alias

				if (check == 1)		//check if alias is needed
					test = alias_check(c, &storage, &check);

				if (test == 1)		//check for semi colon
				{
					semi_check(c, args, storage);
					(void)getc(fp);
				}
		}

		fscanf(fp, "%[^\n]", c);	//keep reading and executing lines

	}

	fclose(fp);		//close file

	while (1)		//loop to run shell
	{
		char** args;

		char str[100];

		printf("?: ");
		scanf(" %[^\n]", str); //imput string

		if (strcmp(str, "history") == 0)	//check for history print
		{
			int e;
			for (e = 0; e < 20; e++)
			{
				if (counter <= 20)
				{
					printf("%d %s\n", e, history[e]);		//print history
				}
				else
				{
					printf("%d %s\n", (counter - 19 + e), history[e + counter - 20]);
					//mem copy wrap around and only replace the last one
				}
			}
		}
		else if ((str[0] == '!') && (str[1] =='!'))		//check for bang in command input
		{		
			semi_check(history[counter - 1], args, storage);		//call main funciton with the last command
		} 
		else if (str[0] == '!')		//check for history number recalled
		{
			char number[5];
			char* endPtr;

			int i = 1;
			int j = 0;

			while (str[i] != '\0')		//go through input
			{
				number[j] = str[i];		//store into number
				i++;
				j++;
			}

			number[j] = '\0';

			int index = strtol(number, &endPtr, 10);		//find number to call history with

			semi_check(history[index], args, storage);		//call main function with history number
		}
		else if(pipe_check(str))		//Check if a pipe is in input
		{
			piping(str);		//call pipe funciton
		}
		else if((str[0] == '$') || ((str[0] == 'e') && (str[1] == 'n')))	//check for environmental changes
		{
			path(str);		//change path function
		}
		else
		{
			counter = history_check(str, &history, counter);		//add to history and counter

			if (check == 0)	//clear alias if check is 0
			{
				test = clear_alias(str, &storage, &check);	
			}
			else		//otherwise check for alias
			{
				test = alias_check(str, &storage, &check);
			}

			if (test == 1)		//just call semi check with main function in it.
				semi_check(str, args, storage);

			test = 1;		//set test
		}

	}

	return 0;
}

void path(char s[])
{
	const int length = strlen(s);		//set length to input length
	char* input[length];
	char* path = getenv("PATH");	//store path variable
	const int size = strlen(path);
	char new_path[size];		//make new path with size
	char copy[length];		//create a copy

	strcpy(copy, s);		//copy input to not change it
	strcpy(new_path, path);

	*input = strtok(s, " ");

	if (strcmp(*input, "$PATH") == 0)	//if input is path print path
	{
		printf("%s\n", path);
	}
	else if (strcmp(*input, "export") == 0)		//if changeing path call this
	{
		char temp[length];
		int i;
		int j = 0;
		for (i = 17; i < length; i++)		//run through the stuff we need
		{
			temp[j] = copy[i];
			j++;
		}

		temp[j] = '\0';

		strcat(new_path, temp);		//put new path onto the path

		setenv("PATH", new_path, 1);		//set that path

		path = getenv("PATH");		//store it

		printf("%s\n", path);		//print the new path
	}
	else
	{
		printf("Error, no such command\n");		//otherwise print an error
	}
}

int pipe_check(char s[])		//check if pipe command is used
{
	int count = 0;
	int i = 0;
	while (s[i] != '\0')
	{
		if (s[i] == '|')		//set to one if we find one so we know to pipe
		{
			count = 1;
		}
		i++;
	}
	return count;
}

void piping(char s[])		//function for piping
{
	int pipefd[2];
	int pid;
	char* p1 = malloc((strlen(s) + 1) * sizeof(char));		//create temp variables to store input
	char* p2 = malloc((strlen(s) + 1) * sizeof(char));
	char* p3 = malloc((strlen(s) + 1) * sizeof(char));

	const char space[2] = " ";
	char** copy = malloc((words(s) + 1) * sizeof(char*));   //allocate memory

	int i;
	for (i = 0; i < words(s); i++)  //allocate memory for the seconde dimension
	{
		copy[i] = malloc((strlen(s) + 1) * sizeof(char*));
	}

	p1 = strtok(s, space);		//store first word in p1

	for (i = 0; i < words(s) + 1; i++)
	{
		copy[i] = p1;		//set copy[i] to whatever p1 is

		p1 = strtok(NULL, space);		//keep reading through input
	}

	copy[i] = "\0";

	strcpy(p2, copy[0]);		//set p2 to the first word to be piped
	strcpy(p3, copy[2]);		//set p3 to the last word that is executing the pipe

	
	char* child_args[] = { p2, NULL };		//child pipe line
	char* parent_args[] = { p3, NULL };		//parent pipe line

		pipe(pipefd);

		pid = fork();		//fork so we can pipe effectively

		if (pid == 0)
		{
			dup2(pipefd[0], 0);		//change from std so we can read continuously
			close(pipefd[1]);		//close one end of the pipe
			close(pipefd[0]);		//close stdin
			execvp(p3, parent_args);		//execute the pipe
		}
		else
		{
			dup2(pipefd[1], 1);		//change from stdin so we can read continously
			close(pipefd[0]);		//close one end of the pipe
			close(pipefd[1]);		//close stdout
			execvp(p2, child_args);	//sent first command to other end of pipe
		}
}

void make_memory(char*** array)		//allocate memntory for an array
{
	char** copy = malloc(500 * sizeof(char*));   //allocate memory

	int j;
	for (j = 0; j < 500; j++)  //allocate memory for the seconde dimension
	{
		copy[j] = malloc(20 * sizeof(char*));
	}

	array[0] = copy;
}

int history_check(char s[], char*** array, int count)		//add the current command to history
{
	char* p1 = malloc((strlen(s) + 1) * sizeof(char));
	
	strcpy(p1, s);
	strcpy(array[0][count], p1);		//add this to the history list

	count++;		//add one to the history count

	return count;
}

void makearg(char st[], char*** args)
{
	const char s[2] = " ";
	char* p1 = malloc((strlen(st) + 1) * sizeof(char));    //allocate memory
	char** copy = malloc((words(st) + 1) * sizeof(char*));   //allocate memory

	int i;
	for (i = 0; i < words(st); i++)  //allocate memory for the seconde dimension
	{
		copy[i] = malloc((strlen(st) + 1) * sizeof(char*));
	}

	p1 = strtok(st, s);

	for(i = 0; i < words(st) + 1; i++)		//loop through for the amount of words we have
	{
		copy[i] = p1;

		p1 = strtok(NULL, s);		//till we reach the end
	}

	copy[i] = "\0";
	
	args[0] = copy;   //copy all the information into args
}

int words(char s2[])    //function to count words
{
	int wordnum = 1;

	int i = 0;
	while (s2[i] != '\0')   //while string isn't finished
	{
		if (s2[i] == ' ')
		{
			wordnum++;      //when a word is finished add one to count
		}

		i++;
	}
	wordnum++;      //add one to word count

	return wordnum + 1;      //return count plus one for null character
}

int clear_alias(char s[], char*** store, int* a)		//get rid of a set alias
{
	const char space[2] = " ";
	int i;
	char* tmp = malloc(8 * sizeof(char));
	char** copy = malloc(2 * sizeof(char*));   //allocate memory

	for (i = 0; i < 2; i++)  //allocate memory for the seconde dimension
	{
		copy[i] = malloc(16 * sizeof(char*));
	}

	tmp = strtok(s, space);

	if (strcmp(tmp, "unalias") == 0)		//if we are unaliasing, free the spot
	{
		free(store[0]);		//clear the alias
		store[0] = copy;		//reallocate memory for the alias
		*a = 1;		//set so in main we know we have cleared alias and can't check for one
		return 0;
	}
	else
	{
		return 1;		//otherwise nothing happened, return 1 so we know
	}
}

int alias_check(char s[], char*** store, int *a)		//check if we find an alias
{
	char s2[48];
	strcpy(s2, s);		//store input to another variable
	const char space[2] = " ";		//check for space
	char* tmp = malloc(24 * sizeof(char));		//temp variable
	char** copy = malloc(8 * sizeof(char*));
	int current = 0, wordcount = 0, charcount = 6;

	int j;
	for (j = 0; j < 8; j++)  //allocate memory for the seconde dimension
	{
		copy[j] = malloc(16 * sizeof(char*));
	}

	tmp = strtok(s2, space);

	if (strcmp(tmp, "alias") == 0)		//check if we are setting an alias
	{
		while (s2[charcount] != '=')		//run till equals sign
		{
			copy[wordcount][current] = s2[charcount];
			charcount++;
			current++;
		}

		copy[wordcount][current] = '\0';		//run till we are empty
		wordcount++;
		charcount += 2;
		current = 0;

		while (s2[charcount] != '"')	//run till we reach a quote, this is the alias command that is covered
		{
			copy[wordcount][current] = s2[charcount];
			charcount++;
			current++;
		}

		store[0] = copy;		//store the variable in the alias list
		*a = 0;
		return 0;
	} else
	{
		copy[0] = "\0";		//set it to empty
		store[0] = copy;		//alias is still empty
		return 1;
	}
}

void semi_check(char s[], char** final, char** alias_check)
{
	char com1[100];
	int length = strlen(s);		//set length to input length

	int i = 0;
	int j = 0;
	int k;
		
	for(k = 0; k < length; k++)		//loop through all input length
	{
			if (s[j + 1] == ';')		//check if we hit a semicolon
			{
				com1[i] = '\0';

				makearg(com1, &final);       //call to function, return number of arguments

				(void)shell_use(com1, final, alias_check);		//call exec main function to run current command

				i = 0;
				j += 3;		//skip the " ; " characters
			}

			com1[i] = s[j];		//keep reading
			i++;
			j++;
	}

	com1[i] = '\0';		//set last one to null

	makearg(com1, &final);       //call to function, return number of arguments

	(void)shell_use(com1, final, alias_check);		//call main shell use function for last command after semicolon
}
void shell_use(char s[], char** input, char** alias)		//main function with exec inside of it
{
	char temp[100] = "/bin/";		//set a /bin/ variable

	if (input[0][0] == '/')
	{
		int i;
		for (i = 0; i < 20; i++)
		{
			temp[i] = input[0][i];		//read the input in
		}
	} else if (strcmp(input[0], alias[0]) == 0)		//check if an alias is used
	{
		strcat(temp, alias[1]);		//set alias to temp

		if (fork() != 0)         //otherwise it will fork and the parent will wait
		{
			wait(NULL);
		}
		else                          //then the child executes the imput
		{
			if (execl(temp, alias[1], NULL) < 0)		//exec the alias stored in temp
			{               //the if line tests the command for valid input
				printf("Exec of %s failed! Try again!\n", input[0]);      //failed command
			}	
		}
		return;		//exit
	} 
	else
	{
		strcat(temp, input[0]);  //add the first command to the end of temp, this will find the correct bin
	}

	if (strcmp(s, "exit") == 0)           //if user types exit it will exit
	{
		exit(0);
	} 
	else if (fork() != 0)         //otherwisse it will fork and the parent will wait
	{
		wait(NULL);
	} 
	else                          //then the child executes the imput
	{
		if (execl(temp, input[0], input[1], input[2], input[3], input[4], NULL) < 0)
		{               //the if line tests the command for valid input
			printf("Exec of %s failed! Try again!\n", input[0]);      //failed command
		}
	}
}