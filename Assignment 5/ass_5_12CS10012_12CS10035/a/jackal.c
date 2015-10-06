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

#define sem_rival sem_lion
#define sem_self sem_jackal
#define SELF "Jackal"

int sem_mut,sem_lion,sem_jackal,sem_ranger,sem_pit,sem_mem,sem_jackal_init;
int id;

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

int check_availability(int curr_pit)
{
	int i;
	//printf("%s %d Checking Availability for %d\n",SELF,id,curr_pit);
	for(i = 0; i < N_PITS; i++)
	{
		//Mutex
		down(sem_mut,0);
		printf("%s %d requesting food from meat-pit %d\n",SELF,id,curr_pit);
		if(!occupied(sem_rival,curr_pit) && !occupied(sem_ranger,curr_pit) && semctl(sem_self,curr_pit,GETVAL,0) < semctl(sem_pit,curr_pit,GETVAL,0))
		{
			up(sem_mut,0);
			return curr_pit;
		}
		if(semctl(sem_pit,curr_pit,GETVAL,0) == 0)
			printf("Meat-pit %d empty\n",curr_pit);
		else	
			printf("%s %d denied access\n",SELF,id);
		up(sem_mut,0);
		// Change curr_pit
		curr_pit = curr_pit%N_PITS + 1;	
	}
	// No pit available
	return -1;
}

int main(int argc, char const *argv[])
{
	int n;						// number of lions
	int k;						// number of times each jackal is going to eat
	pid_t *pid;
	int status;

	int i,j;					// iterating variables
	int curr_pit;				// stores the current pit from which the jackal is going to eat

	int check;					// stores the pit that will be available;

	// read the number of lions form arguments
	if(argc < 3)
	{
		perror("Didn't provide the number of lions");
		exit(0);
	}
	// Read the number of lions
	n = atoi(argv[1]);
	// Read the number of times each jackal is going to eat
	k = atoi(argv[2]);


	pid = (pid_t*)malloc(n*sizeof(pid_t));
	// Wait for the start signal from the Ranger

	// Create Semaphores
	sem_mut 	= semget(MUTEX, 1, IPC_CREAT|0666);
	sem_lion 	= semget(LION, N_PITS + 1, IPC_CREAT|0666);
	sem_jackal 	= semget(JACKAL, N_PITS + 1, IPC_CREAT|0666);
	sem_ranger 	= semget(RANGER, N_PITS + 1, IPC_CREAT|0666);
	sem_pit 	= semget(PIT, N_PITS + 1, IPC_CREAT|0666);
	sem_mem		= semget(NEXT_PIT, 1, IPC_CREAT|0666);
	sem_jackal_init = semget(INIT_JACKAL, 1, IPC_CREAT|0666);
	semctl(sem_jackal_init,0,SETVAL,0);
	down(sem_jackal_init,0);
	printf("Jackal Started %d %d\n",n,k);

	// fork n lions
	for(i = 0; i < n; i++)
	{
		pid[i] = fork();
		if(pid[i] == 0)
		{
			srand(time(NULL));
			// forked process
			// actual jackal code
			id = i;
			// chosen a random pit
			curr_pit = rand()%N_PITS + 1;
			for(j = 0; j < k; j++)
			{

				check = check_availability(curr_pit);
				if(check == -1)
				{
					// no pit available
					curr_pit = (curr_pit - 1 + N_PITS - 1)%N_PITS + 1;
					printf("%s %d in wait queue of meat-pit %d\n",SELF,id,curr_pit);
					// block in the waiting queue
					down(sem_self,0);
					curr_pit = semctl(sem_mem,0,GETVAL,0);
					j--;
					continue;
				}
				//Current consumer will eat from meat-pit number 'check'
				curr_pit = check;
				// Take control
//				down(sem_mut,0);
				up(sem_self,curr_pit);
				printf("%s %d in control of meat-pit %d\n\n\n",SELF,id,curr_pit);
				// Consume
				down(sem_pit,curr_pit);
				printf("\t\t\t\t\t Meat in Pits %d %d %d\n",semctl(sem_pit, 1, GETVAL, 0),semctl(sem_pit, 2, GETVAL, 0),semctl(sem_pit, 3, GETVAL, 0));
//				up(sem_mut,0);
				sleep(2);
				// Check if last consumer to leave the pit

				printf("Number of %s in meat-pit %d = %d\t\t\t%s %d\n",SELF,curr_pit,semctl(sem_self,curr_pit,GETVAL,0),SELF,id);
				if(semctl(sem_self,curr_pit,GETVAL,0) > 1)// || semctl(sem_pit,curr_pit,GETVAL,0) == 0)
				{
					// do not send signal and continue
					down(sem_self,curr_pit);
					continue;
				}
				printf("Down karega %s %d\n",SELF,id);
				down(sem_self,curr_pit);

				
				// down(sem_mut,0);
				
				printf("%s %d left meat-pit %d\n",SELF,id,curr_pit);
				// send signals
				// set shared memory to current pit
				semctl(sem_mem,0,SETVAL,curr_pit);
				// wake up ranger and rival
				if(semctl(sem_ranger,0,GETVAL,0) == 0)				// wake up ranger only when sleeping
					up(sem_ranger,0);
				if(semctl(sem_rival, 0, GETNCNT, 0) == 0)
					printf("Koi uthane ko nahi hai %s %d\n",SELF,id);
				printf("\t\t\tNumber of Jackals waiting %d\n",semctl(sem_jackal, 0, GETNCNT, 0));
				printf("\t\t\tNumber of Lions waiting %d\n",semctl(sem_lion, 0, GETNCNT, 0));
				upn(sem_rival,0,semctl(sem_rival, 0, GETNCNT, 0));
				printf("%s %d giving signal to wait queue of all meat-pit\n",SELF,id);
				
				curr_pit = rand()%N_PITS + 1;
				
				// up(sem_mut,0);
			}
			exit(0);
		}
		else if(pid[i] < 0)
		{
			perror("Fork error!!");
			exit(0);
		}
	}
	for(i = 0; i < n; i++)
		waitpid(pid[i],&status,0);
	free(pid);
	return 0;
}
