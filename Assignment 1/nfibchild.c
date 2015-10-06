//Palash Mittal
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define   BUF_SIZE   100


//function to calculate the fibonacci number in the child process
//val stores the value of ith fibonacci number
int childFib(int i){
	int val,j;
	int x = 1, y = 1;
	if(i <= 2){
		printf("1 ");
	}else{
		for(j = 3; j <= i;j++){
			val = x + y;
			x = y;
			y = val;
		}
		printf("%d ", val);
	}
	return val;
}


void  main(void)
{
 	pid_t   cpid;		//child's pid
 	int     status;		//stat
    int     i,j;
    char    buf[BUF_SIZE];
     
    int n, val;
    
    printf("Enter the number of Fibonacci sequence :\n");
    scanf("%d", &n);			//number of fibonacci numbers asked

    //if a valid value of n is not provided, terminate
    if(n < 0) {
    	printf("Enter a positive integer value for n.\n");
    	return;
    }

    //loop to create n child processes as required
    for(i = 1; i <= n; ++i){
    	if((cpid = fork()) == 0){	//process created and checked for its success
    		val = childFib(i);		//childFib is called to compute ith Fibonacci 
			exit(val);				//child process exits with val
		}else if(cpid < 0){			//if child process creation fails to be forked
			printf("Failed to fork %d child process",i);
		}else{
			wait(&status);			//parent waits for the child
		}
  
    }

    exit(0);
}
