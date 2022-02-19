#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close

#include<arpa/inet.h>
#include<sys/types.h>
#include<netinet/in.h>


#define MAX 1024
#define PORT 8083
#define SA struct sockaddr



static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


struct client{

	int index;
	int sockID;
	struct sockaddr_in clientAddr;
	int len;
	char name[200];
};

pthread_t thread_id[1024];
struct client Client[1024];
int connection_i=0;	

struct MsgData{
	int msg_type; // 0-> list  1->send 2-> broadcast
	char buff[MAX];
	int sender_id;
	char sender_name[200];
};


void *cs_interaction(void *vargp){

	struct client *clientDetail=(struct client *)vargp;

	int connfd=clientDetail->sockID;

	int index=clientDetail->index;

	printf("new Client with id : %d | connected \n",index+1);

	char buff[MAX];

	struct MsgData Msg_data;


	// get client name
	memset(buff,0,sizeof(buff));
	int read_id;
	// read the message from client and copy it in buffer
	read_id=recv(connfd,buff,1024,0);

	if(read_id<=0){
		printf("Client %d | connection closed\n",index+1);
		pthread_exit(NULL);
	}

	buff[read_id]='\0';

	strcpy(Client[index].name,buff);

	while(1){
		memset(buff,0,sizeof(buff));
		
		int read_id;
		// read the message from client and copy it in buffer
		read_id=recv(connfd,buff,1024,0);

		if(read_id<=0){
			printf("Client %d | connection closed\n",index+1);
			pthread_exit(NULL);
		}

		buff[read_id]='\0';

		char output[1024];

		if(strncmp("LIST",buff,4)==0){
		
			memset(buff,0,sizeof(buff));

			int l=0;
			l+=snprintf(output+l,1024,"___________________________________________________\n");
			l+=snprintf(output+l,1024,"| Client id  | socket id  | Client name \n");
			l+=snprintf(output+l,1024,"+------------+------------+------------------------\n");
			                          
			for(int i=0;i<connection_i;i++){
				if(i!=index){
					l+=snprintf(output+l,1024,"| %d          | %d          | %s        \n",i+1,Client[i].sockID,Client[i].name);
				}
			}
			l+=snprintf(output+l,1024,"+------------+------------+------------------------\n");

			Msg_data.msg_type=0;
			strcpy(Msg_data.buff,output);
			Msg_data.sender_id=index;

			send(connfd,&Msg_data,sizeof(Msg_data),0);

		}else if(strncmp("SEND", buff, 4) == 0){
			bzero(buff,MAX);

			read_id=recv(connfd,buff,1024,0);

			if(read_id<=0){
				printf("Client %d | connection closed\n",index+1);
				pthread_exit(NULL);
			}

			buff[read_id]='\0';

			int id=atoi(buff)-1;

			read_id=recv(connfd,buff,1024,0);

			if(read_id<=0){
				printf("Client %d | connection closed\n",index+1);
				pthread_exit(NULL);
			}

			buff[read_id]='\0';

			strcpy(Msg_data.buff,buff);
			Msg_data.sender_id=index;
			Msg_data.msg_type=1;

			printf("\n%s\n",Client[id].name);

			strcpy(Msg_data.sender_name,Client[index].name);

			send(Client[id].sockID,&Msg_data,sizeof(Msg_data),0);
	

		}else if(strncmp("ALL", buff, 3) == 0){

			bzero(buff,MAX);


			read_id=recv(connfd,buff,1024,0);

			if(read_id<=0){
				printf("Client %d | connection closed\n",index+1);
				pthread_exit(NULL);
			}

			buff[read_id]='\0';

			strcpy(Msg_data.buff,buff);
			Msg_data.sender_id=index;
			Msg_data.msg_type=2;

			strcpy(Msg_data.sender_name,Client[index].name);

			for(int i=0;i<connection_i;i++){
				if(i!=index){
					send(Client[i].sockID,&Msg_data,sizeof(Msg_data),0);
				}
			}


		}else{
			bzero(buff,MAX);
			strcpy(buff,"invalid choice");
			
			send(connfd,buff,1024,0);

		}
		

	}



}



void establish_connection(){

	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
   
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));
   
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
   
	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");
   
	// Now server is ready to listen and verification
	if ((listen(sockfd, 1024)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);
   
	// Accept the data packet from client and verification
	

	while(1){
		Client[connection_i].sockID = accept(sockfd, (SA*)&cli, &len);
		if (Client[connection_i].sockID < 0) {
			printf("server accept failed...\n");
			exit(0);
		}
		else
			printf("server accept the client...\n");

		
		Client[connection_i].index=connection_i;
		

		pthread_create(&thread_id[connection_i++], NULL, cs_interaction, (void *) &Client[connection_i]);
	
		if (connection_i >= 500) {
            // Update i
            connection_i = 0;
 
            while (connection_i < 500) {
                // Suspend execution of
                // the calling thread
                // until the target
                // thread terminates
                pthread_join(thread_id[connection_i++], NULL);
                
            }
 
            // Update i
            connection_i = 0;
        }
	}

	
	// close the socket
	close(sockfd);


}




int main(){

	establish_connection();

	return 0;
}