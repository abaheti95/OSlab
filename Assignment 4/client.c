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


#define MAX_CLIENT 20
#define MAX_CHAT_ID 50
#define SER_TYPE 1
#define MAXC MAX_CLIENT * (MAX_CHAT_ID + 10)

typedef struct message{
	long type;
	char mText[MAXC];
} message;

	
int compare(char const *s1, char const *s2){
    int i = 0;
    for(i = 0; s2[i]; i++){
        if(s1[i] != s2[i])
            return -1;
    }
    return 0;
}


int main(){
	int msqidUp, msqidDown, mLen;	
	message msg, buff;

	struct msqid_ds info;		//stores the info for the current status of the queue

	key_t keyUp = 101;			//message queue ID for UP 
	key_t keyDown = 202;		//message queue ID for DOWN

	msqidUp = msgget(keyUp,IPC_CREAT|0666);		//Up message queue from client to server
	msqidDown = msgget(keyDown,IPC_CREAT|0666);	//Down message queue from server to client

	int nClient = 0;		//number of clients
	char client[MAX_CLIENT][MAX_CHAT_ID];

	pid_t clientPid[MAX_CLIENT];
	char clientMSG[MAXC];

	char *ch;	// pointer to the char tokens when using strtok 

	char chatID[MAX_CHAT_ID];


	printf("***********Welcome***********\n\n");
	printf("Enter the client chat ID(Max Size 50 characters)\n(WARNING: Do not use \';\' in the chatID):\n");
	scanf("%s", chatID);

	strcpy(msg.mText, "NEW;");
	strcat(msg.mText, chatID);

	msg.type = SER_TYPE;

	msgsnd(msqidUp,&msg, strlen(msg.mText) + 1, 0);

	sleep(1);

	while(1){
		// printf("here\n");
		while(msgrcv(msqidDown, &buff, MAXC, (long)getpid(),IPC_NOWAIT) != -1){
			// printf("here\n");
			if(compare(buff.mText, "LIST") == 0){
				ch = strtok(buff.mText, ";");
				int i = 0;
				while((ch = (strtok(NULL,";")))!= NULL){
					char * flag;
					flag = strchr(ch, ';');
					if(flag == NULL){
						strcpy(client[i], ch);
					}else
						strncpy(client[i], ch, (size_t)(flag - ch) );
					i++;
				}
				nClient = i;
				// printf("\n%d nclient\n", nClient);

			}else if(compare(buff.mText, "MSG") == 0){
				char * chr;
				ch = strtok(buff.mText,";");
				printf("\n***********New Message***********\n");

				ch = strtok(NULL, ";");
				printf("%-20s %s\n","Message:", ch);

				ch = strtok(NULL, ";");
				printf("%-20s %s\n","Sent time:", ch);

				ch = strtok(NULL, ";");
				printf("%-20s %s\n","Recipient chatID:", ch);
			}
		}

		printf("\n\n***********List of clients online***********\n\n");
		int i = 0;
		for(i = 0; i < nClient; i++){
			printf("%d->%s\n", i+1, client[i]);
		}

		printf("(Enter 0 to refresh the list and receive pending messages.)\n\n");

		int clientSelected;
		printf("Pick a client (Enter the client Number):\n");
		scanf("%d", &clientSelected);
		if(clientSelected == 0){
			continue;
		}
	
		getchar();

		printf("\nEnter the message to be sent:\n");
		scanf("%[^\n]", clientMSG);

		strcpy(msg.mText, "MSG;");
		strcat(msg.mText, clientMSG);
		strcat(msg.mText, ";");
		strcat(msg.mText, client[clientSelected - 1]);

		msg.type = SER_TYPE;

		msgsnd(msqidUp, &msg, strlen(msg.mText) + 1, 0);
		sleep(1);
	}
	return 0;
}