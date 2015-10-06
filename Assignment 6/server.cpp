#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <dirent.h>
#include <utmp.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <bits/stdc++.h>

using namespace std;

vector <string> usersAllowed;

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

string serFile = "ser.txt";


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

// Message Queue
int mq;

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

bool search_list(char const *argv[], int argc, char * name)
{
	bool flag = false;
	int i;
	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i],name) == 0)
			flag = true;
	}
	return flag;
}



string itos(int num)
{
    if(num==0) return "0";
    string temp="";
    char c;
    while(num>0)
    {
        c = '0'+num%10;
        temp.push_back(c);
        num = num/10;
    }
    reverse(temp.begin(),temp.end());
    return temp;
}


void release(){
	// cout << "Closing the server... " << endl;

	//to remove the ser.txt file created
	remove(serFile.c_str());
	//to remove the shared memory, semaphores and message queues created
	if(msgctl(mq, IPC_RMID, NULL)<0)
		perror("msgctl");
	if(semctl(sem1, 0, IPC_RMID)<0)
		perror("semctl sem_msg:");
	if(semctl(sem2, 0, IPC_RMID)<0)
		perror("semctl sem_pid :");
	if(shmctl(shm1, IPC_RMID, NULL)<0)
		perror("shmctl msg:");
	if(shmctl(shm2, IPC_RMID, NULL)<0)
		perror("shmctl pid :");
}

void ctrl_c_handler(int dummy = 0){
	release();
	// cout << shm1 << endl;
	exit(1);
}

void fileaccess(){

	if(access(serFile.c_str(), F_OK) != -1 ){	//this checks for the existence of the file, returns -1 if the file doesnot exist
		cout << "A server is already running. Only one instance is allowed" << endl;
		exit(1);
	}else{
		//else create a file ser.txt which contains the pid of the server
		int fd = open(serFile.c_str(), O_CREAT|O_EXCL|O_WRONLY, 0640);
		pid_t serpid = getpid();
		char buff[100];
		sprintf(buff, "%d", serpid);
		write(fd, buff, strlen(buff));
		close(fd);
	}
}



void commencing(int argc, char const *argv[]){

	struct utmp u;
	char name[500];
	FILE* fp = fopen("/var/run/utmp", "rb");			//Working way
	while(fread(&u, sizeof(u), 1, fp))
	{
		// Read all the user's until the file ends
		struct passwd pwd;
		struct passwd *result;
		char *buff;
		size_t buffsize;
		int s;

		buffsize = sysconf(_SC_GETPW_R_SIZE_MAX);
		if (buffsize == 0)          /* Value was indeterminate */
			buffsize = 16384;        /* Should be more than enough */

		buff = (char*)malloc(buffsize);
		if (buff == NULL) 
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		printf("Searching for user %s : %s : %s\n",u.ut_user,u.ut_name,u.ut_line);
		s = getpwnam_r(u.ut_user, &pwd, buff, buffsize, &result);
		if (result == NULL)
		{
			if (s == 0)
				printf("Not found\n");
			else
			{
				errno = s;
				perror("getpwnam_r");
			}
			//exit(EXIT_FAILURE);
		}
		else
		{
			//Search if it is in argument list
			if(search_list(argv,argc,u.ut_name))
			{
				// successful search call commence
				sprintf(name, "./commence > /dev/%s", u.ut_line);
				system(name);
				// sleep(1);
			}
		}
		free(buff);
		//printf("Name: %s; UID: %ld\n", pwd.pw_gecos, (long) pwd.pw_uid);
	}
}




int main(int argc, char const *argv[]){

	signal(SIGINT,ctrl_c_handler);

	
	if(argc == 1)
	{
		// no arguments given
		perror("No arguments given!!");
		exit(EXIT_FAILURE);
	}

	release();
	fileaccess();
	commencing(argc, argv);
	

 	// Get the IPC resources
    // Semaphores
	int sem1,sem2;
	sem1 = semget(SEM1, 1, IPC_CREAT|0666);         // Semaphore to provide mutual exclusion to the shared PID Array
	sem2 = semget(SEM2, 1, IPC_CREAT|0666);         // Semaphore to provide mutual exclusion to the shared message segment
	// First subsemaphore is for the client program and second semaphore for the server program


	//initializing the semaphores
	semctl(sem1, 0, SETVAL, 1);
	semctl(sem2, 0, SETVAL, 0);
	// semctl(sem2, 1, SETVAL, 0);

	
	mq = msgget(MQKEY,IPC_CREAT|0666);                      // Message queue to transfer messages to all the available MIDs

	// Shared Memory
	int shm1,shm2;
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
	pid_arr[0] = 0;
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


	while(1){
		// cout << "here" << endl;
		// down(sem2, 1);

		sembuf b[2];
  		b[0].sem_num=0;
  		b[0].sem_flg=0;
  		b[0].sem_op=-2;

  		b[1].sem_flg=0;
  		b[1].sem_num=0;
  		b[1].sem_op=1;
  		
  		
  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
  		semop(sem2, b, 2);
  		// cout << "sem2 val" << semctl(sem2, 0, GETVAL, 0) << endl;
  		// cout << "after sempop" << endl;

  		char* copy = strdup(msg);
  		// cout << "Received message" << endl << copy << endl;
  		down(sem2, 0);
		//get the message from the shared memory
		// cout << "palash " << endl << "msg" << endl << copy << endl;
		if(strcmp(copy, "." ) == 0){
			// cout << "Inside dot" << endl;
			// up(sem2,0);
			continue;
		}else if(strcmp(copy, "*") == 0){
			//goes for termination
			cout << "Terminating the server" << endl;
			break;
		}else{
			char buff[SHM_M_SIZE];
			strcpy(buff, copy);
			char *ch = strtok(buff, ":");

			//getting the message sent by the client
			char *msgp = strtok(NULL, ":");
			//getting the pid of the client which sent the message
			pid_t pid = atoi(ch);
			// cout << "pid" <<  pid << endl;
			cout << "Got message " << endl << copy << endl;
			//waiting on the sem1 to access the pid array
			message mqmsg;
			strcpy(mqmsg.mText, msgp);
			down(sem1, 0);
			for(int i = 0; i < (*arr_size); i++)
			{
				if(pid_arr[i] != pid)
				{	
					// cout << "Inside loop" << endl;
					mqmsg.type = pid_arr[i];
		            if(msgsnd(mq, &mqmsg, strlen(mqmsg.mText) + 1, 0) == -1){
		            	cout << "Message sending failed to pid = " << pid_arr[i] << endl;
		            }
		            cout << "Message :" << copy << "sent to  pid = " << pid_arr[i] << endl;;
				}
			}

			up(sem1,0);
		}
	}


	release();
	return 0;

}