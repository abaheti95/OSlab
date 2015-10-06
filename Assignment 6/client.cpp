#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utmp.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pwd.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <queue>
#include <string>
#include <bits/stdc++.h>

using namespace std;
// Kill SIGNAL
#define SIG_KILL                20

// Shared memory sizes
#define SHM_M_SIZE              5000
#define SHM_PID_SIZE    		500

// Semaphores Keys
#define SEM1                    123
#define SEM2                    234
// MessageQueue Keys
#define MQKEY                   345
// Shared Memory Keys
#define SHM1                    456
#define SHM2                    567

// buffer for storing the messages received from Message Queue
#define BUFF_SIZE               5000
typedef struct message{
        long type;
        char mText[BUFF_SIZE];
} message;

// Gloabal Variables
// Semaphores
int sem1,sem2;
// Shared Memory
int shm1,shm2;
int *pid_arr,*arr_size;
char *msg;
// queue for storing the messages
queue<string> msg_queue;
// Child pid
pid_t pid;

int sign;


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


void ctrl_c_handler(int dummy = 0)
{
	int i;                          // iterating variable
	char buff[BUFF_SIZE];
	// read the message and push it in the message queue
	printf("<Ctrl+C is pressed>\n--- Enter your message:\n");
	scanf("%[^\n]",buff);
	getchar();
	if(strcmp(buff,"bye") == 0)
	{
        // exit program
        // remove pid from pid array
		down(sem1,0);
		for(i = 0; i < (*arr_size); i++)
		{
			if(pid_arr[i] == getpid())
			{
	            // remove this pid
				break;
			}
		}
	        // shift on position
		for(; i < (*arr_size)-1; i++)
			pid_arr[i] = pid_arr[i+1];
		(*arr_size)--;
		if((*arr_size) == 0)
		{
            // send * to the server
			sembuf b[2];
	  		b[0].sem_num=0;
	  		b[0].sem_flg=0;
	  		b[0].sem_op=0;

	  		b[1].sem_flg=0;
	  		b[1].sem_num=0;
	  		b[1].sem_op=1;
	
	  		semop(sem2, b, 2);
            // add the message in message queue
			strcpy(msg,"*");
			up(sem2,0);
		}
		up(sem1,0);
	        // kill child program
		kill(SIG_KILL,pid);
	        // kill self
		exit(EXIT_SUCCESS);
	}
	else
	{
	    // add the message in queue
	    char temppid[100];
	    sprintf(temppid, "%d", getpid());
	    string m = string(temppid) + ":" + string(buff);
	    cout << "m " << m << endl;
		msg_queue.push(m);
	}
}

void die(int dummy = 0)
{
	exit(EXIT_SUCCESS);
}


void donothing(int num)
{
	sign=true;
}

int main()
{
	// check if server exists by checking ser.txt file
	if( access( "ser.txt", F_OK ) == -1 )
	{
	// server doesn't exist
		perror("Server doesn't exists!!");
		exit(1);
	}

	// Get the IPC resources
	sem1 = semget(SEM1, 1, IPC_CREAT|0666);         // Semaphore to provide mutual exclusion to the shared PID Array
	sem2 = semget(SEM2, 1, IPC_CREAT|0666);         // Semaphore to provide mutual exclusion to the shared message segment
	// Here the first subsemaphore is for the client program and second semaphore for the server program
	// Message Queue
	int mq;
	mq = msgget(MQKEY,IPC_CREAT|0666);                      // Message queue to transfer messages to all the available MIDs

	// shm1 is shared PID array
	if((shm1 = shmget(SHM1, SHM_PID_SIZE,IPC_CREAT|0666)) == -1) 
	{
		perror("Shared PID array was not created!");
		exit(1); 
	}
	if((pid_arr = (int*)shmat(shm1, NULL, 0)) == (int *)-1)
	{
		perror("shmat");
		exit(1);
	}
	arr_size = pid_arr;
	pid_arr = &pid_arr[1];
	// shm2 is shared message section
	if((shm2 = shmget(SHM2, SHM_M_SIZE,IPC_CREAT|0666)) == -1) 
	{
		perror("Shared Message Segment was not created!");
		exit(1); 
	}
	if((msg = (char*)shmat(shm2, NULL, 0)) == (char *)-1)
	{
		perror("shmat");
		exit(1);
	}










	// Creating a child receiver
	if((pid = fork()) == -1)
	{
		perror("Fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{

		signal(SIGINT, donothing);
		// Child receiver process
		// add the parent PID in the shared array
		down(sem1,0);
		pid_arr[*arr_size] = getppid();
		(*arr_size)++;
		up(sem1,0);

		// signal(SIG_KILL,die);
		// cout << "Inside child " << endl;
		// Keep receiving messages and display them
		message buff;
		// while(1){
		while(msgrcv(mq, &buff, BUFF_SIZE, (long)getppid(),0))
		{
			printf("Recieved msg \"%s\"\n",buff.mText);
		}
		// }
		
		exit(EXIT_SUCCESS);
	}
	else
	{
	// attach signal handler to the parent
		signal(SIGINT,ctrl_c_handler);
		// Parent Program which chats
		while(true)
		{
		// If there is a message in message queue then try sending that
			// cout << "Inside parent " << endl;
			// cout << "msg = " << msg << endl;
			// cout << msg_queue.size() << endl;
			if(msg_queue.size() > 0)
			{
		        // send message
				// down(sem2,0);
				sembuf b[2];
		  		b[0].sem_num=0;
		  		b[0].sem_flg=0;
		  		b[0].sem_op=0;

		  		b[1].sem_flg=0;
		  		b[1].sem_num=0;
		  		b[1].sem_op=1;
		  		
		  		// cout << "for sending" << endl;
		  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
		  		semop(sem2, b, 2);
		  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
		        // add the message in message queue
				strcpy(msg,msg_queue.front().c_str());
				cout << "Sending msg " << msg << endl;
				msg_queue.pop();
				// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
				up(sem2,0);
				// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
			}
			else
			{
		        // send .
				// down(sem2,0);
				sembuf b[2];
		  		b[0].sem_num=0;
		  		b[0].sem_flg=0;
		  		b[0].sem_op=0;

		  		b[1].sem_flg=0;
		  		b[1].sem_num=0;
		  		b[1].sem_op=1;
		  		
		  		// cout << "For dot" << endl;
		  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
		  		semop(sem2, b, 2);
		  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
		        // add the message in message queue
		        // sleep(10);
		        sleep(2);
				strcpy(msg,".");
				// cout << "msg" << endl << msg  << endl;
				// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
				up(sem2,0);
				// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
			}
		}
	}

	

	return 0;
}