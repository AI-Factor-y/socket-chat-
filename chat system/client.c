#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <pthread.h>


#define PORT 8083
#define MAX 1024

struct MsgData{
	int msg_type; // 0-> list  1->send
	char buff[MAX];
	int sender_id;
	char sender_name[200];
};


void * recieve_comm(void * sockID){

	struct MsgData Msg_data;

	int clientSocket = *((int *) sockID);

	while(1){

		int read = recv(clientSocket,&Msg_data,sizeof(Msg_data),0);
			
		if(Msg_data.msg_type==0){
			printf("%s",Msg_data.buff);
		}else if(Msg_data.msg_type==1){
			printf("%d| %s > %s\n",Msg_data.sender_id+1,Msg_data.sender_name,Msg_data.buff);
		}else{
			printf("[Broadcast] %d| %s > %s\n",Msg_data.sender_id+1,Msg_data.sender_name,Msg_data.buff);
		}
	}

}

int main(){

	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connection to server Established\n");

	pthread_t thread;
	pthread_create(&thread, NULL, recieve_comm, (void *) &clientSocket );

	char client_name[200];

	printf("Enter your name : ");
	scanf("%[^\n]s",client_name);

	send(clientSocket,client_name,200,0);


	while(1){

		
		char input[MAX];
		printf("\n");
		scanf("%s",input);

		if(strcmp(input,"LIST") == 0){

			send(clientSocket,input,MAX,0);

		}
		if(strcmp(input,"SEND") == 0){

			send(clientSocket,input,MAX,0);
			
			scanf("%s",input);
			send(clientSocket,input,MAX,0);
			
			scanf("%[^\n]s",input);
			send(clientSocket,input,MAX,0);

		}

		if(strcmp(input,"ALL") == 0){

			send(clientSocket,input,MAX,0);
			
			scanf("%[^\n]s",input);
			send(clientSocket,input,MAX,0);

		}



	}


}