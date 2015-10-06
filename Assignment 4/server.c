#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>



#define MAX_CLIENT 20
#define MAX_CHAT_ID 50
#define SER_TYPE 1
#define MAXC MAX_CLIENT * (MAX_CHAT_ID + 10)

typedef struct message{
	long type;
	char mText[MAXC];
} message;


key_t keyUp = 101;			//message queue ID for UP 
key_t keyDown = 202;		//message queue ID for DOWN

	
int compare(char const *s1, char const *s2){
    int i = 0;
    for(i = 0; s2[i]; i++){
        if(s1[i] != s2[i])
            return -1;
    }
    return 0;
}

int search_pid(pid_t clientPid[MAX_CLIENT],int nClient,pid_t sender_pid){
	int i;		// iterating variable
	for(i = 0; i < nClient; i++){
		if(clientPid[i] == sender_pid)
			return i;
	}
	
	return -1;
}

int search_recipient_ID(char client[MAX_CLIENT][MAX_CHAT_ID],int nClient,char * recipient){
	int i;		// iterating variable
	for(i = 0; i < nClient; i++){
		if(strcmp(client[i],recipient) == 0)
			return i;
	}
	
	return -1;
}

void handler(int s){
	struct msqid_ds buf;
	int msqidUp;
	int msqidDown;

	msqidUp = msgget(keyUp, 0666);
    msqidDown = msgget(keyDown, 0666);

	msgctl(msqidUp,IPC_RMID,&buf);
	msgctl(msqidDown,IPC_RMID,&buf);
	printf("\n****Terminating Server****\n");
	exit(0);
}

int main(){
	int msqidUp, msqidDown, mLen;	
	message msg, buff;

	struct msqid_ds info;		//stores the info for the current status of the queue

	msqidUp = msgget(keyUp,IPC_CREAT|0666);		//Up message queue from client to server
	msqidDown = msgget(keyDown,IPC_CREAT|0666);	//Down message queue from server to client

	int nClient = 0;		//number of clients
	char client[MAX_CLIENT][MAX_CHAT_ID];

	pid_t clientPid[MAX_CLIENT];

	char *ch;	// pointer to the char tokens when using strtok 

	// char clientList[MAXC];	//store the name of the clients to be broadcasted 

	signal(SIGINT, handler);



	while(1){
		//receiving message from UP queue (from client)
		//buff stores the message
		//MAXC max size of the message
		//SER_TYPE is the type of message designated for the server(1), default is 0
		// 0 msgflag denotes waiting condition
		msgrcv(msqidUp, &buff, MAXC, SER_TYPE, 0);

		if(compare(buff.mText, "NEW") == 0){
			ch = strtok(buff.mText,";");
			ch = strtok(NULL,";");				//points to the name of the client


			//stores the name of the client(chat ID)
			strcpy(client[nClient], ch);

			if (msgctl(msqidUp, IPC_STAT, &info) == -1) {
				perror("msgctl: msgctl failed");
				exit(1);
			}

			//extracting the pid of the last client joined
			clientPid[nClient++] = info.msg_lspid;

			//creating the message to be sent to clients	
			int i = 0;
			strcpy(msg.mText, "LIST;");
			for(i = 0; i < nClient; i++){
				strcat(msg.mText, client[i]);
				strcat(msg.mText, ";");
			}

			//broadcasting the client info to all clients
			for(i = 0; i < nClient; i++){
				msg.type = clientPid[i];
				if(msgsnd(msqidDown,&msg,strlen(msg.mText) + 1,0) == -1){
					perror("msgsnd: msgsnd failed");
					exit(1);
				}
			}

			printf("%s","\n***********New client***********\n");
			printf("%-20s %s\n%-20s %d\n","Chat ID:",client[nClient - 1],"PID:",clientPid[nClient - 1]);
			printf("%-20s %d\n\n","Number of Clients:",nClient);
			
		}else if(compare(buff.mText, "MSG") == 0){
			// extracting the client ID and message to be relayed
			
			// Getting the info from message queue status
			if (msgctl(msqidUp, IPC_STAT, &info) == -1) {
				perror("msgctl: msgctl failed");
				exit(1);
			}			
			int sender_index = search_pid(clientPid,nClient,info.msg_lspid);		//searching for sender

			ch = strrchr(buff.mText,';') + 1;		// Last index of ':' in the message. Pointer to the recipient ID
			int recipient_index = search_recipient_ID(client,nClient,ch);			//searching for receiver

			// timestamp
			time_t send_time = info.msg_stime;
			char out_time[20];
			strftime(out_time, 20, "%Y-%m-%d %H:%M:%S ", localtime(&send_time));

			// Generating the message to be sent
			ch = strtok(buff.mText, ";");
			ch = strtok(NULL, ";");
			strcpy(msg.mText,"MSG;");
			strcat(msg.mText,ch);		//concatenating the message to be sent to the recipient and ";"
			strcat(msg.mText, ";");
			strcat(msg.mText, out_time);
			strcat(msg.mText,";");
			strcat(msg.mText, client[sender_index]);

			// relay the message
			msg.type = clientPid[recipient_index];

			msgsnd(msqidDown, &msg, strlen(msg.mText) + 1,0);

			ch = strtok(msg.mText,";");
			ch = strtok(NULL,";");
			printf("***********New Message***********\n\n");
			printf("%-20s %s\n","Sender:",client[sender_index]);
			printf("%-20s %s\n","Message:", ch);
			printf("%-20s %s\n","Sent time:",out_time);
			printf("%-20s %s\n","Recipient:" ,client[recipient_index]);
			printf("%-20s %d\n\n","#messages in queue:",(int)info.msg_qnum);
			printf("\n");
		}
	}
	return 0;
}