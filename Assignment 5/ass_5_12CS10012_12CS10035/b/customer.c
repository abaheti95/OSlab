#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define KEY_CUSTOMER 100
#define KEY_BARBER 101
#define KEY_MUTEX 102
#define NUMCHAIRS 10

int numWait = 0;
int count = 1;

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


int main(){
	//creating the corresponding semaphore variables
	int sem_customer = semget(KEY_CUSTOMER, 1, IPC_CREAT|0666);
	int sem_barber = semget(KEY_BARBER, 1, IPC_CREAT|0666);
	int sem_mutex = semget(KEY_MUTEX, 1, IPC_CREAT|0666);

	//setting the initial value for semaphore variables
	semctl(sem_customer, 0, SETVAL, 0);
	semctl(sem_barber, 0, SETVAL, 0);
	semctl(sem_mutex, 0, SETVAL, 1);

	printf("Barber shop opens...\n");

	// while(1){
	// sleep(rand()%20 + 1);
	down(sem_mutex,0);
	numWait = semctl(sem_barber, 0, GETNCNT, 0);
	if(numWait < NUMCHAIRS){
		up(sem_customer,0);
		up(sem_mutex,0);
		// if(count == 1){
			// printf("Customer %d wakes up the barber\n", count);
		// }else{
			printf("Customer %d is waiting..\n",count);
		// }
		down(sem_barber, 0);
		printf("Customer is getting a haircut.");
		// count++;
		
	}else{
		printf("Chair capacity full\n");
		printf("Customer leaves\n");
		// printf("Customer %d has to leave\n", count);
		// count++;
		up(sem_mutex,0);
	}
	// }
	return 0;
}