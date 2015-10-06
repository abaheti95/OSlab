#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int getfibo(int n)
{
	if(n == 1 || n == 2)
	{
		return 1;
	}
	int i,f1=1,f2=1,fib;
	for(i=2;i<n;i++)
	{
		fib = f1 + f2;
		f1 = f2;
		f2 = fib;
	}
	return fib;
}

int main()
{
	int i,nfib,childfib;
	int mypid;
	int *fib,*pid;
	printf("Enter the value of nfib : ");
	scanf("%d",&nfib);

	// Allocate memory for the arrays
	fib = (int*)malloc(nfib*sizeof(int));
	pid = (int*)malloc(nfib*sizeof(int));
	mypid = getpid();
	for(i=0;i<nfib;i++)
	{
		pid[i] = fork();
		if(pid[i] == 0)
		{
			//child process created
			//Get the ith fibonacci number
			childfib = getfibo(i+1);

			//Print the calculated fibonacci number in the child process
			printf("The %dth fibonacci number is %d\n",i+1,childfib);
			exit(childfib);
		}
		else
		{
			//do nothing
		}
	}
	//An adequate delay is given so that the printing in child process is done neatly
	sleep(3);
	
	//Get the fibonacci numbers by killing the processes
	printf("The parent process ID = %d\n",mypid);
	for(i=0;i<nfib;i++)
	{
		//Kill process i and wait for the ith fibonacci number
		printf("The Child %d is being deleted\n",pid[i]);
		waitpid(pid[i],&fib[i],0);
		fib[i]>>=8;
	}
	//Print the fibonacci numbers in the array
	for(i=1;i<=nfib;i++)
	{
		printf("The %dth fibonacci number is %d\n",i,fib[i-1]);
	}
	// Free the allocated memory
	free(fib);
	free(pid);
	return 0;
}