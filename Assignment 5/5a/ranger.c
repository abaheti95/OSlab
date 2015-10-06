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
#define PIT_CAPACITY 50

#define PIT 100
#define RANGER 101
#define LION 102
#define JACKAL 103
#define MUTEX 104
#define NEXT_PIT 105
#define INIT_LION 106
#define INIT_JACKAL 107

int sem_mut,sem_lion,sem_jackal,sem_ranger,sem_pit,sem_mem,sem_lion_init,sem_jackal_init;

int occupied(int sem_id, int sub_sem_id)
{
	return (semctl(sem_id,sub_sem_id,GETVAL,0) > 0);
}

void down(int sem_id, int sub_sem_id)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

void up(int sem_id, int sub_sem_id)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

void upn(int sem_id, int sub_sem_id,int n)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = n;
	sop.sem_flg = 0;
	semop(sem_id,&sop,1);
}

void fill(int sub_sem_id)
{
	struct sembuf sop;
	sop.sem_num = sub_sem_id;
	sop.sem_op = 10;
	sop.sem_flg = 0;
	semop(sem_pit,&sop,1);	
}

int check_availability(int curr_pit)
{
	int i;
	printf("Checking Availability\n");
	for(i = 0; i < N_PITS; i++)
	{
		//Mutex
		down(sem_mut,0);
		printf("Ranger requesting control over meat-pit %d\n",curr_pit);
		if(!occupied(sem_lion,curr_pit) && !occupied(sem_jackal,curr_pit) && (semctl(sem_pit,curr_pit,GETVAL,0) <= (PIT_CAPACITY-10)))
		{
			up(sem_mut,0);
			return curr_pit;
		}
		printf("Ranger denied access over meat-pit %d\n",curr_pit);
		up(sem_mut,0);
		// Change curr_pit
		curr_pit = curr_pit%N_PITS + 1;	
	}
	// No pit available
	return -1;
}

void fifo(struct semid_ds *buf_l, struct semid_ds *buf_j)
{
	//traverse the wait queues
	
}

int main()
{
	int curr_pit;
	int check;
	int i;						// iterating variable

	struct semid_ds *buf_l,*buf_j;		// used for storing the status of the semaphore vairable

	srand(time(NULL));
	// Create Semaphores
	sem_mut 	= semget(MUTEX, 1, IPC_CREAT|0666);
	sem_lion 	= semget(LION, N_PITS + 1, IPC_CREAT|0666);
	sem_jackal 	= semget(JACKAL, N_PITS + 1, IPC_CREAT|0666);
	sem_ranger 	= semget(RANGER, N_PITS + 1, IPC_CREAT|0666);
	sem_pit 	= semget(PIT, N_PITS + 1, IPC_CREAT|0666);
	sem_mem		= semget(NEXT_PIT, 1, IPC_CREAT|0666);
	sem_lion_init = semget(INIT_LION, 1, IPC_CREAT|0666);
	sem_jackal_init = semget(INIT_JACKAL, 1, IPC_CREAT|0666);

	//Initialize Mutex semaphore
	semctl(sem_mut,0,SETVAL,1);
	semctl(sem_lion,0,SETVAL,0);
	semctl(sem_jackal,0,SETVAL,0);
	semctl(sem_ranger,0,SETVAL,0);
	semctl(sem_pit,0,SETVAL,0);
	semctl(sem_mem,0,SETVAL,1);
	for(i = 1;i <= N_PITS; i++)
	{
		semctl(sem_lion,i,SETVAL,0);
		semctl(sem_jackal,i,SETVAL,0);
		semctl(sem_ranger,i,SETVAL,0);
		semctl(sem_pit,i,SETVAL,0);
	}

	curr_pit = rand()%N_PITS + 1;
	fill(curr_pit);
	//Initialize consumers
	up(sem_lion_init,0);
	up(sem_jackal_init,0);
	semctl(sem_mut,0,SETVAL,1);

	curr_pit = rand()%N_PITS + 1;
	while(1)
	{
		check = check_availability(curr_pit);
		if(check == -1)
		{
			// no pit available
			printf("Ranger Signal %d %d %d %d\n",semctl(sem_ranger,0,GETVAL,0),semctl(sem_ranger,1,GETVAL,0),semctl(sem_ranger,2,GETVAL,0),semctl(sem_ranger,3,GETVAL,0));
			printf("\t\t\t\t\t Meat in Pits %d %d %d\n",semctl(sem_pit, 1, GETVAL, 0),semctl(sem_pit, 2, GETVAL, 0),semctl(sem_pit, 3, GETVAL, 0));
			down(sem_ranger,0);
		}
		else
		{
			down(sem_mut,0);
			// fill pit number 'check'
			curr_pit = check;
			// Take control
			printf("Ranger in control of meat-pit %d\n",curr_pit);
			up(sem_ranger,curr_pit);
			fill(curr_pit);	
			down(sem_ranger,curr_pit);
			// wake up the sleeping lions and jackals
			// set memory to current pit
			semctl(sem_mem,0,SETVAL,curr_pit);
			printf("\t\t\tNumber of Jackals waiting %d\n",semctl(sem_jackal, 0, GETNCNT, 0));
			printf("\t\t\tNumber of Lions waiting %d\n",semctl(sem_lion, 0, GETNCNT, 0));
			printf("\t\t\t\t\t%d %d %d\n",semctl(sem_pit, 1, GETVAL, 0),semctl(sem_pit, 2, GETVAL, 0),semctl(sem_pit, 3, GETVAL, 0));
			// Implement fifo
			semctl(sem_lion,0,IPC_STAT,buf_l);
			semctl(sem_lion,0,IPC_STAT,buf_j);
			fifo(buf_l,buf_j);
			upn(sem_jackal,0,semctl(sem_jackal, 0, GETNCNT, 0));
			upn(sem_lion,0,semctl(sem_lion, 0, GETNCNT, 0));
			up(sem_mut,0);
		}
	}
	return 0;
}
