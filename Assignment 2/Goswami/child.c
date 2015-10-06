#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500

int gen_random(int min,int max)
{
	return min + rand() %(max-min+1);
}

void signal_handlar(int sig)
{
	if(sig==SIGUSR1)
		exit(1);
}

int main(int argc,char const *argv[])
{
	srand(time(NULL));

	char filename[11];
	int arr[5],starting_index=0,size=5,i,command,ack,pivot,random;

	int read_fd = atoi(argv[1]),child_id;
	read(read_fd,&child_id,sizeof(child_id));

	sprintf(filename,"data_%d.txt",child_id);

	FILE *fp = fopen(filename,"r");
	
	for(i=0;i<5;i++)
		fscanf(fp,"%d",&arr[i]);

	fclose(fp);

	ack = READY;
	int write_fd = atoi(argv[2]);
	write(write_fd,&ack,sizeof(ack));

	while(1)
	{
		signal(SIGUSR1,signal_handlar);



		read(read_fd,&command,sizeof(command));

		if (command == REQUEST)
		{
			int tmp;

			if (size>0)
			{
				random = gen_random(0,size-1);
				tmp=arr[random];
			}
			else
				tmp=-1;

			write(write_fd,&tmp,sizeof(tmp));

		}
		else if (command == PIVOT)
		{
			read(read_fd,&pivot,sizeof(pivot));
			int count=0;
			for(i=0;i<size;i++)
			{
				if(arr[i]>pivot)
					count++;
			}
			printf("--child %d receives pivot and replies %d\n",child_id,count);
			write(write_fd,&count,sizeof(count));
		}
		else if (command == SMALL)
		{
			int ind=0;
			for(i=0;i<size;i++)
			{
				if(arr[i]>=pivot)
				{
					arr[ind]=arr[i];
					ind++;
				}
			}
			size=ind;
		}
		else if (command == LARGE)
		{
			int ind=0;
			for(i=0;i<size;i++)
			{
				if(arr[i]<=pivot)
				{
					arr[ind]=arr[i];
					ind++;
				}
			}
			size=ind;
		}
	}
	
	close(read_fd);
	close(write_fd);

	return 0;
}