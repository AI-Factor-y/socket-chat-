
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX 100
#define PORT 8083
#define SA struct sockaddr

struct Fruit{

	char fruit_name[100];
	int count;

};

void clear_input_stream(){
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}


void cs_interation(int sockfd){

	char buff[MAX];
	struct Fruit fruit_buff;
	int n;
	struct Fruit Fruits[5];

	while(1){
		bzero(buff, sizeof(buff));
		printf("\nEnter the command : \n");
		n = 0;

		while ((buff[n++] = getchar()) != '\n');

		write(sockfd, buff, sizeof(buff));

		if(strncmp(buff,"exit",4)==0){ // exit condition
			break;
		}

		if(strncmp(buff,"SendInventory",13)==0){

			printf("\nFruits\t\tcount\n");
			
			read(sockfd,Fruits,sizeof(Fruits));

			for(int i=0;i<5;i++){
				printf("%s\t\t%d\n",Fruits[i].fruit_name,Fruits[i].count);
			}
			continue;
		}



		bzero(buff, sizeof(buff));

		read(sockfd, buff, sizeof(buff));

		printf("\nServer : %s\n", buff);

		if((strcmp(buff,"invalid choice")==0)){
			bzero(buff,sizeof(buff));
			continue;
		}

		printf("Enter the fruit name : \n");
		scanf("%s",fruit_buff.fruit_name);

		printf("Enter the count : \n");
		scanf("%d",&fruit_buff.count);

		write(sockfd, &fruit_buff, sizeof(fruit_buff));

		bzero(buff,sizeof(buff));

		read(sockfd, buff, sizeof(buff));

		printf("\nserver : %s\n",buff);

		clear_input_stream();

	}
}




void establish_connection(){

	int sockfd, connfd;
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
   
	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");
   
	// function for chat
	cs_interation(sockfd);
   
	// close the socket
	close(sockfd);

}


int main(){


	printf("\nCommands\n");
	printf("1 : Fruits\n");	
	printf("2 : SendInventory\n");
	printf("-------------------\n");


	establish_connection();

	return 0;
}