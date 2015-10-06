#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*MACROS*/
#define REQUEST 100
#define PIVOT 	200
#define LARGE	300
#define SMALL	400
#define READY	500
#define EXIT	600

/*Create a pipe in parent and send from child*/
inline void swap(int *a,int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}
int main(int argc, char const *argv[])
{
	int fd[5][2];						// 5 pipes for parent
	int fc[5][2];						// 5 pipes for child
	int pid[5];							// Pids of the five child processes
	int status;							// Status of the killed child process
	int i;							// Iterating variable
	int pivot;							// To store the input pivot value
	int id;
	int send,receive;					//Variables to send and receive data from the pipes
	int k,n,m;							//Variables for the algorithm
	char * _argv[4];

	/*The assignment is to establish a interprocess communication between the parent and the child programs*/
	// First we will create 5 child and 10 pipes
	for(i=0;i<5;i++)
	{
		pipe(fd[i]);
		pipe(fc[i]);
		swap(&fd[i][1],&fc[i][1]);
		if((pid[i] = fork()) == -1)
		{
			perror("fork");
			exit(1);
		}
		else if(pid[i] == 0)
		{
			//Entered the child process
			//Read the id from the parent process
			close(fd[i][0]);
			close(fd[i][1]);
			_argv[0]="child";
			_argv[1]=(char *)malloc(10*sizeof(char));
			sprintf(_argv[1],"%d",fc[i][0]);
			_argv[2]=(char *)malloc(10*sizeof(char));
			sprintf(_argv[2],"%d",fc[i][1]);
			_argv[3]=NULL;

			execv(_argv[0],_argv);
			free(_argv[1]);
			free(_argv[2]);
		}
		else
		{
			//Unneeded child pipes
			close(fc[i][0]);
			close(fc[i][1]);
		}
	}

	for(i=0;i<5;i++)
	{		
		//Parent
		id = i+1;
		printf("Sending id %d to child %d\n",i+1,i);
		write(fd[i][1],&id,sizeof(id));
		//Wait for all the ready
		read(fd[i][0],&receive,sizeof(receive));
		if(receive == READY)
			printf("Child %d is READY\n",i);
	}
	printf("Parent READY\n");
	n = 25;
	k = n>>1;						//k = n/2
	while(1)
	{
		// Ask a random child for pivot
		m = 0;
		while(1)
		{
			send = REQUEST;
			id = rand()%5;
			write(fd[id][1],&send,sizeof(send));
			printf("Parent sends REQUEST to Child %d\n",id+1);
			//Recieve the PIVOT element if any
			read(fd[id][0],&receive,sizeof(receive));
			if(receive!=-1)
				break;
		}
		pivot = receive;
		printf("Parent broadcasts pivot %d to all children\n",pivot);
		for(i=0;i<5;i++)
		{
			//Send the pivot element to every child
			send = PIVOT;
			write(fd[i][1],&send,sizeof(send));
			write(fd[i][1],&pivot,sizeof(pivot));
		}
		//Read responses from all the children
		for(i=0;i<5;i++)
		{
			read(fd[i][0],&receive,sizeof(receive));
			m += receive;
		}
		if(m == k)
		{
			printf("Pivot %d found!!!\n",pivot);
			break;
		}
		else if(m > k)
		{
			//Send SMALL requests to all children
			printf("Since %d(m) > %d(k) Parent Sends SMALL to all children\n",m,k);
			for(i=0;i<5;i++)
			{
				send = SMALL;
				write(fd[i][1],&send,sizeof(send));
			}
		}
		else if(m < k)
		{
			//Send LARGE requests to all children
			printf("Since %d(m) < %d(k) Parent Sends LARGE to all children\n",m,k);
			for(i=0;i<5;i++)
			{
				send = LARGE;
				write(fd[i][1],&send,sizeof(send));
			}
			k = k - m;
		}
	}
	//Terminate all the child processes
	printf("Parent sends kill signal to all children\n");
	for(i=0;i<5;i++)
	{
		send = EXIT;
		write(fd[i][1],&send,sizeof(send));
		waitpid(pid[i],&status,0);
	}
	close(fd[i][0]);
	close(fd[i][1]);
	return 0;
}
