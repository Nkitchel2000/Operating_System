/*Nick Kitchel
  CS 240
  Shell Assignment
*/

#include <stdio.h>
#include <stdlib.h>     //library for malloc usage
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int makearg(char s[], char*** args);  //token function

int wordsize(char s1[]);      //function or largest word size
int words(char s2[]);                    //function for number of words

int main()
{
	char** args, str[1000];

	while(1)		//loop to run shell
	{
		printf("?: ");
		scanf(" %[^\n]", str);      //imput string

		int argc;

		argc = makearg(str, &args);       //call to function, return number of arguments

		char temp[100] = "/bin/";

                strcat(temp, args[0]);	//add the first command to the end of temp, this will find the correct bin

		if(strcmp( str, "exit") == 0)		//if user types exit it will exit
                {
                        exit(0);
                } else if( fork() != 0)		//otherwisse it will fork and the parent will wait
		{
			wait( NULL );
		} else				//then the child executes the imput
		{
			if(execl(temp, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], NULL) < 0)
			{		//the if line tests the command for valid input
				printf("Exec of %s failed! Try again!\n",args[0]);	//failed command
			}
		}

	}

	return 0;
}

int makearg(char s[], char*** args)
{
	int wordcount = 0, charcount = 0, count = 0;

	char* p1 = malloc((wordsize(s) + 1) * sizeof(char));    //allocate memory
	char** copy = malloc((words(s) + 1) * sizeof(char*));   //allocate memory

	int j;
	for (j = 0; j < words(s); j++)  //allocate memory for the seconde dimension
	{
		copy[j] = malloc((wordsize(s) + 1) *sizeof(char*));
	}

	while (s[charcount] == ' ')             //edge case of miss typing with a space
	{                                                      //this will skip those
		charcount++;
	}

	while (s[charcount] != '\0')     //travers through the string
	{	
		if (s[charcount] == ' ')        //when space is found read in word
		{
			p1[count] = '\0';

			int k;
			for (k = 0; k < count + 1; k++)
			{
				copy[wordcount][k] = p1[k]; //copy it to a two dimentional array
			}
                                                                               
			wordcount++;
			count = 0;
			charcount++;
		}

		p1[count] = *(s + charcount); //read characters into p1
		count++;
		charcount++;
	}

	if (s[charcount] == '\0') //checking for the last null at end of string
	{
		p1[count] = '\0';

		int k;
		for (k = 0; k < count + 1; k++)
		{
			copy[wordcount][k] = p1[k]; //read in the final word to the two dimentional array
		}

		wordcount++;
	}

	copy[wordcount] = '\0';


	args[0] = copy;   //copy all the information into args

	return wordcount; //return tokens
}

int wordsize(char s1[]) //find largest word
{
        int largest = 0, current = 1;

        int i = 0;
        while (s1[i] != '\0')   //when string isn't finished
        {
		if (s1[i] == ' ')
        	{
                	largest = current;
                	current = 0;
        	}
        	current++;      //add one to current word
        	i++;
        }

        if (s1[i] == '\0' && (current > largest))       //if the last word is largest
	{                                                             //change the largest word to current
        	largest = current;
        }
	return largest;
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
	}      wordnum++;      //add one to word count

     return wordnum + 1;      //return count plus one for null character
}

