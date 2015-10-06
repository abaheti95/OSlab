#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/*MACROS*/
#define REQUEST 100
#define PIVOT 	200
#define LARGE	300
#define SMALL	400
#define READY	500
#define EXIT	600


int main(int argc, char const *argv[])
{
	srand(time(NULL));

	int fc[2];							// Pipe variables
	int j;								// Iterating variable
	int numbers[5];						// Array for storing 5 random numbers
	int size = 5;						// Storing the current size of the array
	int pivot;							// To store the input pivot value
	int shift;							// To store the amount of shift to be done on SMALL and LARGE request
	int id;
	FILE *fp;
	char fname[10];
	int send,receive;					//Variables to send and receive data from the pipes
	fc[0] = atoi(argv[1]);
	fc[1] = atoi(argv[2]);
	read(fc[0],&id,sizeof(id));
	printf("This is child %d\n",id);
	//Read all the random numbers from data_i.txt
	sprintf(fname,"data_%d.txt",id);
	printf("fname = %s\n",fname);
	fp = fopen(fname,"r");
	if(fp == NULL)
	{
		perror("Error while reading file!!");
		exit(EXIT_FAILURE);
	}
	for(j=0;j<5;j++)
		fscanf(fp,"%d",&numbers[j]);
	printf("The numbers belonging to child %d are : ",id);
	for(j=0;j<5;j++)
		printf("%d ",numbers[j]);
	printf("\n");

	//Send Ready to the Parent
	send = READY;
	printf("Child %d sends READY\n",id);
	write(fc[1],&send,sizeof(send));

	while(1)
	{
		//Read from the parent, execute and wait for the next command
		read(fc[0],&receive,sizeof(receive));
		switch(receive)
		{
			case REQUEST:
				if(size == 0)
				{
					//Send -1 back to the parent
					send = -1;
					write(fc[1],&send,sizeof(send));
				}
				else
				{
					//Send a random number from the array back to the parent
					send = numbers[rand()%5];
					write(fc[1],&send,sizeof(send));
				}
				break;
			case PIVOT:
				read(fc[0],&pivot,sizeof(pivot));
				for(j=0;j<size;j++)
				{
					if(numbers[j] > pivot)
						break;
				}
				send = shift = size - j;
				printf("Child %d receives pivot %d and replies %d\n",id,pivot,send);
				write(fc[1],&send,sizeof(send));
				break;
			case SMALL:
				//Delete all the integers smaller than the pivot
				for(j=0;j<shift;j++)
				{
					numbers[j] = numbers[j+size-shift];
				}
				size = shift;
				break;
			case LARGE:
				//Delete all the integers larger than the pivot
				size -= shift;
				break;
			case EXIT:
				printf("Child %d terminates\n",id);
				close(fc[0]);
				close(fc[1]);
				exit(0);
		}
	}
	return 0;
}