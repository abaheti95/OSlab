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
		return 1;
	}else{
		for(j = 3; j <= i;j++){
			val = x + y;
			x = y;
			y = val;
		}
		// printf("%d ", val);
	}
	return val;
}


void  main(void)
{
 	pid_t   cpid, ppid;
 	int     status;
    int     i,j;
    char    buf[BUF_SIZE];
     
    int n, val;
    
    printf("Enter the number of Fibonacci sequence :\n");
    scanf("%d", &n);                        //number of fibonacci numbers asked


    int FIB[n+1];               //array to store the fibonacci numbers
    int PID[n+1];               //array to store the pid of the child process

    //if a valid value of n is not provided, terminate
    if(n < 0) {
    	printf("Enter a positive integer value for n.\n");
    	return;
    }

    printf("The Fibonacci Sequence printed inside the child process is...\n");

    //loop for creating the child processes.
    for(i = 1; i <= n; ++i){
    	if((cpid = fork()) == 0){          //if the fork of child process was successfuls
    		val = childFib(i);            //get the corresponding ith Fibonacci value from childFib
            printf("Child number = %d, Fib value = %d\n",i,val);
			exit(val);                       //exit with value val
		}else if(cpid < 0){
			printf("Failed to fork %d child process",i);
		}else{
			PID[i] = cpid;                  // store the child's pid in the array
		}  
    }

    //here parent waits for the child to exit
    for(i = 1; i <= n; i++){
    	waitpid(PID[i],&status,0);
    	FIB[i] = WEXITSTATUS(status);              //value is saved in FIB[i] corresponding to the exit status of the process PID[i]
    }

    //Fibonacii numbers are printed
    printf("The required Fibonacci numbers are....\n");
 	for(i = 1; i <= n ;i++){
 		printf("FIB[%d] = %d\n", i, FIB[i]);
 	}

    exit(0);
}
