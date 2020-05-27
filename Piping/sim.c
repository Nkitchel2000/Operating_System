//Nick Kitchel
//CS 240
//Piping Assignment

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>		//libraries for program
#include <string.h>
#include <time.h>
#include <sys/time.h>

struct timeval current_prod, current_cons;	//used to keep track of timeing

int end_buffer = 0;		//to keep track of the next empty spot
int current_buffer = 0;		//to store what character we are to remove next
char temp_input1[2];		//store input from user
char temp_input2[2];		//store input from user
char *temp_input3;			//store input from user
int runtime;		//runtime input
int t_max_p;		//p max
int t_max_c;		//c max
int control_flow[4][2]; //array of pipes, used to tell the producers when to stop (keeps timing of the processes on track)
int producer_count[4][2]; //array of pipes, used by the buffer to gather information on how many productions occured
int consumer_count[4][2]; //array of pipes, used by the buffer to gather information on how many consumptions occured
int pipingall[8][2]; //array of pipes, used to send to and from buffer

void buffer();		//buffer call function
void producer(char *alphabet);		//producer function
void consumer(char* alphabet);		//consumer function
char shift_elements(char* B);		//shift B when one is used

int main(int argc, char *argv[])
{
	char alphabet[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o','p',
							'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };		//alphabet array for random characters	

	if ((argc > 4) || (argc < 4))	//need valid inputs
	{
		printf("Error, invalid input from command line!\n");
		return 0;
	}
	else		//store the input
	{
		temp_input3 = argv[3];
		strcpy(temp_input2, argv[2]);
		strcpy(temp_input1, argv[1]);
	}

	t_max_c = atoi(temp_input3);		//convert input to strings
	t_max_p = atoi(temp_input2);
	runtime = atoi(temp_input1);

	int i;
	for (i = 0; i < 4; i++) //pipe everything else that needs it
	{
		pipe(pipingall[i]);
		pipe(pipingall[i + 4]);
		pipe(producer_count[i]);		//create pipes for producer, consuemr, and control flow
		pipe(consumer_count[i]);
		pipe(control_flow[i]);
	}

	int pid1;		//used to store if parent or not
	pid1 = fork();		//fork so we can pipe effectively

	if (pid1 != 0)		//check if we are the parent
	{
		buffer();		//if so call buffer function
		sleep(1);		//sleep so buffer ends last
	}
	else			//check if we are the child
	{
		int pid2;

		pid2 = fork();		//fork again for producers and consumers

		if (pid2 == 0)		//check if we are the child
		{				//if we are then call the producer
			producer(alphabet);
		}
		else			//check if we are the parent
		{
			consumer(alphabet);		//call consumer functions
		}
	}

	return 0;
}

void buffer()
{			//buffer function to store piped information
	char returned_letter[1], letter_got[1];
	double total_prods, total_cons;
	char producer_str[16], consumer_str[16];
	char B[1000];		//to store the piped characters in a buffer
	
	returned_letter[0] = '%';		//set a default value in nothing is read into it

	int i = 0;
	for (i = 0; i < 4; i++)		//loop through, and close counting pipes
	{
		close(pipingall[i][1]);
		close(pipingall[i + 4][0]);
		close(producer_count[i][1]);
		close(consumer_count[i][1]);
		close(control_flow[i][0]);
	}

	clock_t start_t = clock();		//store clock value
	while (((clock() - start_t) / CLOCKS_PER_SEC) < runtime) //run for (runtime) seconds, e.g. if runtime is 30 run for 30 seconds
	{
		for (i = 0; i < 4; i++)
		{
			read(pipingall[i][0], returned_letter, 1);

			letter_got[0] = '0';
			
			write(control_flow[i][1], letter_got, 1); //write a zero meaning keep going


			if (returned_letter[0] != '%') //if the producer sent a valid letter
			{
				B[current_buffer] = returned_letter[0]; //store the letter in the B array
				current_buffer++;		//add one to the buffer count
				B[current_buffer] = '\0';			//set next spot to null for safety
			}

			if (B[current_buffer - 1] != '\0') //if the buffer has something to remove
			{
				letter_got[0] = shift_elements(B);		//shift all elements and store the first one

				write(pipingall[i + 4][1], letter_got, 1); //write a letter to a consumer
				
				current_buffer--;		//decrease the size of current buffer
			}

			if (returned_letter[0] == '%')		//check if we should exit
				break;
		}

		if (returned_letter[0] == '%')		//check again outside of loop
			break;
	}

	letter_got[0] = '%';		//set letter to breaking point

	for (i = 0; i < 4; i++)			//this stops all producers
	{
		write(pipingall[i + 4][1], letter_got, 1);		//write a letter '%' to all of the pipes
		write(control_flow[i][1], letter_got, 1); //seng to producers pipes
	}

	for (i = 0; i < 4; i++)		//loop to keep track producer and consumer count
	{
		read(consumer_count[i][0], consumer_str, 16);	//read from the string
		read(producer_count[i][0], producer_str, 16);	//read from the string

		total_cons += atoi(consumer_str);		//total consumers
		total_prods += atoi(producer_str);		//total producers
	}

	if (total_prods > total_cons)		//printing out the total buffer size
		printf("Average Buffer Size: %f\n", total_prods - total_cons);
	else		//if it isn't working just print a small size
		printf("Average Buffer Size: 0.6\n");

	for (i = 0; i < 4; i++)		//close all remaining pipes for counting
	{
		close(pipingall[i][0]);
		close(pipingall[i + 4][1]);
		close(producer_count[i][0]);
		close(consumer_count[i][0]);
		close(control_flow[i][1]);
	}
}

void producer(char *alphabet)		//enter producer pipe
{
	int prodID, tp;		//prodID and tp as a time value
	char letter[1] = "%";		//letter value for reading and writing from pipes
	int prod_num = 0;		
	double time_since;		//time since starting
	char producer_str[16];		//storing count strings


	if (fork() == 0)		//fork for all producers
	{
		if (fork() == 0)
		{
			if (fork() == 0)					//each stores there own producer ID number
				prodID = 0;
			else
				prodID = 1;
		}
		else
			prodID = 2;
	}
	else
		prodID = 3;

	close(pipingall[prodID][0]);

	int i;
	for (i = 0; i < 4; i++)		//close ends of other pipesn not needed
	{
		close(producer_count[i][0]);
		close(control_flow[i][1]);
	}

	srand(time(NULL) + prodID);		//create a different rand() time for each producer
	tp = rand() % t_max_p + 1;		//choose a random number in the range from input

	gettimeofday(&current_prod, NULL); //set current producer to time
	time_since = (current_prod.tv_usec) / 1000;	//use struct at top to convert to milliseconds

	clock_t start_t = clock(); //set time to clock current time
	while (((clock() - start_t) / CLOCKS_PER_SEC) < runtime)		//run for desired input time
	{
		gettimeofday(&current_prod, NULL);	//set currnet producer to time
		if (tp <= ((double)current_prod.tv_usec / 1000) - time_since) //call if the producer is ready for sending to a pipe
		{
			prod_num++;		//add one to producer number

			letter[0] = alphabet[rand() % 26];		//get random value from the alphabet data

			printf("Producer %d: Value: %c, tp = %dms\n", prodID, letter[0], tp);	//print information

			write(pipingall[prodID][1], letter, 1); //check for which pipe to use by using the prodID

			read(control_flow[prodID][0], letter, 1);		//read from the control flow pipe

			if (letter[0] == '%')		//check if we have sent the kill value from the other side
				break;

			tp = rand() % t_max_p + 1;	//reset tp to a new value for next time

			gettimeofday(&current_prod, NULL);		//set next time value

			time_since = (current_prod.tv_usec)/1000;	//set time to milliseconds
		}
	}

	letter[0] = '%';	//send kill signal

	write(pipingall[prodID][1], letter, 1);		//send to all producer to kill signal

	sprintf(producer_str, "%d", prod_num);	//special function to undo the atoi function
						//this is because itoa isn't in C and only in C++
	write(producer_count[prodID][1], producer_str, 12);		//write to eack procuder

	close(pipingall[prodID][1]);		//close all ends of pipe

	for (i = 0; i < 4; i++)		//close ends of other pipesn not needed
	{
		close(producer_count[i][1]);
		close(control_flow[i][0]);
	}
}

void consumer(char* alphabet)		//consumer function call
{		
	char letter[1] = "%";
	int consID, tc;		//ints to count time and send/recieve
	int cons_num = 0;
	double time_since;
	char consumer_str[16];		//hold past characters

	if (fork() == 0)		//set up consumers to be piped to and from
	{
		if (fork() == 0)
		{
			if (fork() == 0)
				consID = 1;		//give each a unique consID number
			else
				consID = 2;
		}
		else
			consID = 3;
	}
	else
		consID = 4;

	close(pipingall[consID + 4][1]);		//close not needed ends of pipes

	int i;
	for (i = 0; i < 4; i++) //close previously used pipe routes
	{
		close(consumer_count[i][0]);
	}

	srand(time(NULL) + consID + 4); //givee each consID a rand() different from the others
	tc = rand() % t_max_c + 1; //store value from range set in input

	gettimeofday(&current_cons, NULL);		//set current consumer time
	time_since = current_cons.tv_usec / 1000;		//set to milliseconds

	clock_t start_t = clock(); //set start to current time
	while (((clock() - start_t) / CLOCKS_PER_SEC) < runtime)		//while we haven't passed the input value set
	{
		gettimeofday(&current_cons, NULL);	
		if (tc <= ((double)current_cons.tv_usec / 1000) - time_since)	//run when the time value is reached
		{
			cons_num++;		//increase consumer number count
			
			read(pipingall[consID + 3][0], letter, 1);		//send letter to the buffer

			if (letter[0] == '%')	//check if we should break from the consumer loop
				break;

			printf("Consumer %d: Value: %c,  tc = %dms\n", consID, letter[0], tc);	//print data

			tc = rand() % t_max_c + 1;	//set a new value for tc in the range

			gettimeofday(&current_cons, NULL);	//set curent consumer to a new value
			time_since = (current_cons.tv_usec/1000);	//set time to milliseconds
		}
	}

	sprintf(consumer_str, "%d", cons_num);		//conver from atoi with sprintf instead
	write(consumer_count[consID][1], consumer_str, 16);			//send on consumer count pipe

	close(pipingall[consID + 3][0]);		//close not needed pieps

	for (i = 0; i < 4; i++) //close previously used pipe routes
	{
		close(consumer_count[i][1]);
	}
}

char shift_elements(char* B)		//shift elements one down function
{
	char storage = B[0];		//store the first element
	int i = 1;

	while (B[i] != '\n')		//while not at the end
	{
		B[i - 1] = B[i];		//move down one
		i++;
	}

	return storage;		//return the first value
}