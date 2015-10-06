#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*The aim is to calculate Fibonacci Numbers using fork() and wait() functions
The idea is to loop the process of creating a child process and calculating the next fibonacci number in the child process*/
int main()
{
	int i,f1=0,f2=1,fib=1,n;
	int pid,status;
	printf("Which Fibonacci Number do you want : ");
	scanf("%d",&n);
	for(i = 1;i < n; i++)
	{
		// create a child process
		pid = fork();
		if(pid == 0)
		{
			//Child process
			printf("Child %d pid %d begins\n",i,getpid());
			//Calculate next fibonacci number
			fib = f1 + f2;
			//Print the calculated number
			printf("%dth Fibonacci number = %d\n",i,fib);
			exit(0);
		}
		else
		{
			//Waiting for the child to finish execution
			waitpid(pid,&status,0);
			//Propogate the values
			fib = f1 + f2;
			f1 = f2;
			f2 = fib;
		}
	}
	printf("%dth Fibonacci = %d\n",n,fib);

	return 0;
}
