#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>

#define START 100

#define N_PITS 3

#define PIT 100
#define RANGER 101
#define LION 102
#define JACKAL 103
#define MUTEX 104
#define NEXT_PIT 105
#define INIT_LION 106
#define INIT_JACKAL 107

void up(int sem_id, int sub_sem_id)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

void down(int sem_id, int sub_sem_id)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

int main()
{
	int sem_mem;
	int i,id;
	int pid;
	sem_mem		= semget(NEXT_PIT, 1, IPC_CREAT|0666);
	semctl(sem_mem,0,SETVAL,0);
	for(i=0;i<3;i++)
	{
		pid = fork();
		if(pid == 0)
		{
			id = i;
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			up(sem_mem,0);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			up(sem_mem,0);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			up(sem_mem,0);
			sleep(1);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			down(sem_mem,0);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			down(sem_mem,0);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			down(sem_mem,0);
			printf("%d value = %d\n",id , semctl(sem_mem,0,GETVAL,0));
			exit(0);
		}
	}
	
	for(i=0;i<3;i++)
		wait(NULL);
	return 0;
}