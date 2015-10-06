#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

/*We need to build a rock paper scissor game in which two child processes will play the game where jugde is the parent process
Step 1: In each iteration (also called round), P sends ready signal to the children C and D using
signal handler not pipe.
Step 2: After receiving the ready signal, each of the two child processes C and D generates a
random positive integer from 1 to 3(i.e. code 1 for paper) and sends that to P via its pipe.
Step 3: P reads the two integers and depending upon the above signal, it will increase the
points of one or both the childs according to above 4 game rules.
Step 4: The child process who first obtains more than ten points wins the game, if there is a tie
then parent will generate two random number, if the first generated number is bigger than
second. then child one wins the game and vice versa.
Step 5: When the game ends, P sends a user-defined signal to both C and D, and the child
processes exit after handling the signal (in order to know who was the winner). After C and D
exit, the parent process P exits.
*/

/*User defined signals*/
#define SIGPLAY 49					// If a child receives this signal, it will choose one of rock-paper-scissor and send it to parent via pipe
#define SIGWIN 50					// If the child wins then SIGWIN will be sent to that processes
#define SIGLOSE 51					// If the child wins then SIGLOSE will be sent to that processes

#define PAPER 1
#define SCISSOR 2
#define ROCK 3

#define READY 100

/*Signal Handling Functions*/
void play1();						// Child 1 sends rock paper or scissor to parent
void win1();						// Function called when a child 1 is declared winner
void lose1();						// Function called when a child 1 is declared loser
void play2();						// Child 2 sends rock paper or scissor to parent
void win2();						// Function called when a child 2 is declared winner
void lose2();						// Function called when a child 2 is declared loser
void quit();						// Function called when a child has to terminate

inline void swap(int *a,int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}
inline void print_move(int move)
{
	switch(move)
	{
		case PAPER:
			printf("PAPER\n");
			break;
		case SCISSOR:
			printf("SCISSOR\n");
			break;
		case ROCK:
			printf("ROCK\n");
	}
}

int fp[2][2];						// 2 pipes for parent
int fc[2][2];						// 2 pipes for child

int main()
{
	int pid[2];						// array to store pIDs of two child processes
	int i;							// Iterating variable
	//int id;							// ID of the child process
	int score[2];					// array to store the scores of the two childs;
	int move[2];					// variables to store player moves

	sigset_t myset;
	
	srand(time(NULL));
	//Parent will create two child and two pipes
	for(i=0;i<2;i++)
	{
		pipe(fp[i]);
		pipe(fc[i]);
		swap(&fp[i][1],&fc[i][1]);
		if((pid[i] = fork()) == -1)
		{
			perror("Fork error! Terminate\n");
			exit(1);
		}
		else if(i == 0 && pid[i] == 0)
		{
			// child process
			// Declare ID of the child
			//id = i+1;
			// close unneeded parent pipes
			close(fp[i][0]);		// parent read pipe
			close(fp[i][1]);		// parent write pipe
			close(fc[i][0]);		// child read pipe
			// declare the signal handling functions - basically what will happen if the signal is recieved
			printf("First Child Process\n");
			signal(SIGPLAY,play1);
			signal(SIGWIN,win1);
			signal(SIGLOSE,lose1);
			signal(SIGQUIT,quit);

			//Send ready to parent
			move[i] = READY;
			write(fc[i][1],&move[i],sizeof(move[i]));

			(void) sigemptyset(&myset);
			while(1)
			{
				sigsuspend(&myset);
			}				// Wait forever in while loop - thus the child program will terminate only when parent says it to quit
		}
		else if(i==1 && pid[i] == 0)
		{
			// child process
			// Declare ID of the child
			//id = i+1;
			// close unneeded parent pipes
			close(fp[i][0]);		// parent read pipe
			close(fp[i][1]);		// parent write pipe
			close(fc[i][0]);		// child read pipe
			// declare the signal handling functions - basically what will happen if the signal is recieved			
			printf("Second child process\n");
			signal(SIGPLAY,play2);
			signal(SIGWIN,win2);
			signal(SIGLOSE,lose2);
			signal(SIGQUIT,quit);

			//Send ready to parent
			move[i] = READY;
			write(fc[i][1],&move[i],sizeof(move[i]));
			(void) sigemptyset(&myset);
			while(1)
			{
				sigsuspend(&myset);
			}				// Wait forever in while loop - thus the child program will terminate only when parent says it to quit
		}
		else
		{
			// Parent Process
			// close unneeded pipes
			close(fc[i][0]);		// child read pipe
			close(fc[i][1]);		// child write pipe
			close(fp[i][1]);		// parent write pipe
		}
	}

	//Receive ready from both child processes
	read(fp[0][0],&move[0],sizeof(move[0]));
	if(move[0] == READY)
		printf("Player 1 ready\n");
	read(fp[1][0],&move[1],sizeof(move[1]));
	if(move[1] == READY)
		printf("Player 2 ready\n");
	// Parent AKA Judge will start the game
	printf("Match between Process %d AKA Player 1 and Process %d AKA Player 2\n",pid[0],pid[1]);
	score[0] = score[1] = 0;		// initially both childs AKA players will have zero score
	i = 0;
	while(score[1]<10 && score[0]<10)
	{
		// Keep playing until one of the player's score reached 10
		i++;
		printf("ROUND %d :\n",i);
		kill(pid[1],SIGPLAY);
		sleep(3);
		kill(pid[0],SIGPLAY);
		sleep(3);

		read(fp[0][0],&move[0],sizeof(move[0]));
		read(fp[1][0],&move[1],sizeof(move[1]));
		printf("Read Value from 1 = ");
		print_move(move[0]);
		printf("Read Value from 2 = ");
		print_move(move[1]);
		if(move[0] == move[1])
		{
			// Its a tie
			if(rand() > rand())
				score[0]++;
			else
				score[1]++;
		}
		else if(move[0] == ROCK && move[1] == SCISSOR)
			score[0]++;
		else if(move[0] == ROCK && move[1] == PAPER)
			score[1]++;
		else if(move[0] == PAPER && move[1] == ROCK)
			score[0]++;
		else if(move[0] == PAPER && move[1] == SCISSOR)
			score[1]++;
		else if(move[0] == SCISSOR && move[1] == PAPER)
			score[0]++;
		else if(move[0] == SCISSOR && move[1] == ROCK)
			score[1]++;
		printf("Score 1 = %d and Score 2 = %d\n",score[0],score[1]);
	}
	// Declare winner and loser
	if(score[0] == 10)
	{
		// Child 1 AKA Player 1 wins and Child 2 AKA Player 2 loses
		kill(pid[0],SIGWIN);
		sleep(3);
		kill(pid[1],SIGLOSE);
		sleep(3);
	}
	else
	{
		// Child 2 AKA Player 2 wins and Child 1 AKA Player 1 loses
		kill(pid[0],SIGLOSE);
		sleep(3);
		kill(pid[1],SIGWIN);
		sleep(3);
	}
	// Terminate both the processes
	kill(pid[0],SIGQUIT);
	sleep(3);
	kill(pid[1],SIGQUIT);
	sleep(3);

	printf("Game Over!\n");
	exit(0);
}

/*First and foremost, printing from signal handler is a bad idea. Signal handler is like an interrupt handler - it happens asynchronously, it could be raised while being inside your standard library code and calling another stdlib routine might mess up with non-reentrant internals of it (imagine catching SIGINT while inside of printf() in your loop).*/
void play1()
{
	signal(SIGPLAY,play1);			// reset signal
	srand(time(NULL));
	int send = rand()%3 + 1;
	write(fc[0][1],&send,sizeof(send));
	//printf("Player 1 sending %d\n",send);				// Not printing in the signal handler as it is not recommended
}
void play2()
{
	signal(SIGPLAY,play2);			// reset signal
	srand(time(NULL));
	int send = rand()%3 + 1;
	write(fc[1][1],&send,sizeof(send));
	//printf("Player 2 sending %d\n",send);				// Not printing in the signal handler as it is not recommended
}

void win1()
{
	signal(SIGWIN,win1);			// reset signal
	printf("Player 1 Wins! :D\n");
}
void win2()
{
	signal(SIGWIN,win2);			// reset signal
	printf("Player 2 Wins! :D\n");
}

void lose1()
{
	signal(SIGLOSE,lose1);			// reset signal
	printf("Player 1 Loses! :'(\n");
}
void lose2()
{
	signal(SIGLOSE,lose2);			// reset signal
	printf("Player 2 Loses! :'(\n");
}

void quit()
{
	printf("Child %d Terminates!\n",getpid());
	exit(0);
}