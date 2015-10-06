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

#define TRUE 1
#define FALSE 0

int gen_random(int min,int max)
{
	return min + rand() %(max-min+1);
}

int main (int agrc,char const* argv[])
{
	srand(time(NULL));

	int fd[10][2],i,index,ack,command,ready[5],random,pivot,k,n=25,m;
	pid_t childs_pid[5];
	char * _argv[4];

	for(i=0;i<10;i++)
		pipe(fd[i]);

	for(index=1;index<=5;index++)
	{
		childs_pid[index-1] = fork();
		if (childs_pid[index-1] == (pid_t) 0) 
		{
			close(fd[2*index-1][0]);

			close(fd[2*index-2][1]);

			_argv[0]="child";
			_argv[1]=(char *)malloc(10*sizeof(char)); sprintf(_argv[1],"%d",fd[2*index-2][0]);
			_argv[2]=(char *)malloc(10*sizeof(char)); sprintf(_argv[2],"%d",fd[2*index-1][1]);
			_argv[3]=NULL;

			execv(_argv[0],_argv);
		}
		else
		{
			close(fd[2*index-2][0]);
			write(fd[2*index-2][1],&index,sizeof(index));
			// close(fd[2*index-2][1]);
		}
	}

	

	for(index=1;index<=5;index++)
	{
		ready[index]=FALSE;

		close(fd[2*index-1][1]);
		read(fd[2*index-1][0],&ack,sizeof(ack));
		// close(fd[2*index-1][0]);

		if(ack==READY)
		{
			ready[index]=TRUE;
			printf("--child %d sends READY\n",index);
		}
	}

	int flag=TRUE;

	for(i=0;i<5;i++)
	{
		if(ready[i])
			continue;
		else
		{
			flag=FALSE;
			break;
		}
	}

	if(flag)
		printf("--parent READY\n");

	k=n/2;

	while(1)
	{
		random = gen_random(1,5);
		command = REQUEST;
		write(fd[2*random-2][1],&command,sizeof(command));
		// close(fd[2*random-2][1]);
		printf("--parent sends REQUEST to child %d\n",random);



		read(fd[2*random-1][0],&ack,sizeof(ack));
		if(ack==-1)
		{
			printf("--child %d sends %d to parent\n",random,ack);
			continue;
		}
		printf("--child %d sends %d to parent\n",random,ack);


		command=PIVOT;
		pivot=ack;
		m=0;
		int tmp[5];
		printf("--parent broadcasts %d to all children\n",pivot);
		for(index=1;index<=5;index++)
		{
			write(fd[2*index-2][1],&command,sizeof(command));
			write(fd[2*index-2][1],&pivot,sizeof(pivot));

			read(fd[2*index-1][0],&tmp[index-1],sizeof(tmp[index-1]));
			m+=tmp[index-1];
		}
		if(m==0)
			continue;
		else if(m>k)
			command=SMALL;
		else if (m<k)
		{
			k=k-m;
			command=LARGE;
		}
		else
		{
			printf("--parent: m=%d+%d+%d+%d+%d. %d=%d. Median found!\n",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],m,k);
			break;
		}

		for(index=1;index<=5;index++)
			write(fd[2*index-2][1],&command,sizeof(command));
	}

	printf("--parent sends kill signal to all children\n");
	for(i=0;i<5;i++)
	{
		if(kill(childs_pid[i],SIGUSR1)==-1)
			perror("SIG Error: ");
		else
			printf("--Chid %d terminates\n",i+1);
	}

	for(index=1;index<=5;index++)
	{
		close(fd[2*index-2][1]);
		close(fd[2*index-1][0]);
	}

	for(i=0;i<5;i++)
		waitpid(childs_pid[i],NULL,0);

	return 0;
}