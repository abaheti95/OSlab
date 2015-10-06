#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LION 11
#define JACKAL 22
#define RANGER 33
#define PITS 44
#define QUEUE 55
#define NUMPITS 3

void sigsem(int semid, int sn)
{
	struct sembuf sop;
	sop.sem_num=sn;
	sop.sem_op=1;
	sop.sem_flg=0;
	semop(semid, &sop, 1);
}

void waitsem(int semid, int sn)
{
	struct sembuf sop;
	sop.sem_num=sn;
	sop.sem_op=-1;
	sop.sem_flg=0;
	semop(semid, &sop, 1);
}

int main()
{
	int i=0, nL=0, semid, retval, key, status=0;
	int lion_id, jackal_id, ranger_id, pit_id, rand_id=0, queue_id, sav_id=0, L_hunger;
	int lion_cnt, jack_cnt, ranger_cnt, food_amt, queue_cnt;
	printf("Please enter number of Lions : ");
	scanf("%d", &nL);
	printf("Please enter Food to be eaten by each Lion : ");
	scanf("%d", &L_hunger);
	lion_id = semget (LION,NUMPITS,IPC_CREAT|0666);
	jackal_id = semget (JACKAL,NUMPITS,IPC_CREAT|0666);
	ranger_id = semget (RANGER,NUMPITS,IPC_CREAT|0666);
	pit_id = semget (PITS,NUMPITS,IPC_CREAT|0666);
	queue_id = semget (QUEUE,NUMPITS,IPC_CREAT|0666);

	for(i=0;i<nL;i++)
	{
		if(!fork())
			break;
	}

	if(i==nL)
	{
		for(i=0;i<nL;i++)
		{
			wait(&status);
		}
	}
	else
	{
		while(L_hunger)
		{
			while(!rand_id)
				rand_id=rand()%4;

			sav_id=rand_id;
			int iterations=1;

			while(iterations<3)
			{
				lion_cnt=semctl(lion_id,rand_id-1,GETVAL,0);
				jack_cnt=semctl(jackal_id,rand_id-1,GETVAL,0);
				ranger_cnt  =semctl(ranger_id,rand_id-1,GETVAL,0);
				food_amt=semctl(pit_id,rand_id-1,GETVAL,0);
				queue_cnt=semctl(queue_id,rand_id-1,GETVAL,0);
				printf("Lion %d requesting food from meat pit %d", i, rand_id);
				if(!jack_cnt&&!ranger_cnt&&food_amt>lion_cnt)
				{
					printf("Lion %d in control of meat pit %d", i, rand_id);
					sigsem(lion_cnt, rand_id-1);
					semctl(food_amt,rand_id-1,SETVAL,semctl(food_amt,rand_id-1,GETVAL,0)-1);
					waitsem(lion_cnt, rand_id-1);
					printf("Lion %d left meat pit %d", i, rand_id);
					if(semctl(lion_id,rand_id-1,GETVAL,0)>0 || semctl(food_amt,-1,GETVAL,0)==0)
					{	
						continue;
					}
					else
					{
						printf("Lion %d giving sigsem to wait-queue of all meat pit", i);
						int i=0;
						for(i=0;i<3;i++)
						{
							while(semctl(queue_id,i,GETVAL,0)<=0)
							{
								sigsem(queue_id, i);
							}
							wait(queue_id, i);
						}
						break;
					}

				}
				else
				{
					printf("Lion %d denied access", i);
					rand_id=(rand_id+1)%3;
					iterations++;
				}
			}

			if(iterations==3)
			{
				printf("Lion %d in wait-queue of meat pit %d", i, sav_id);
				waitsem(queue_id, sav_id);
			}
			L_hunger--;
		}
	}
	return 0;
}