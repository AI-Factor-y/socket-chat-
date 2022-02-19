#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close
#define MAX 100
#define PORT 8083
#define SA struct sockaddr

struct Fruit{

	char fruit_name[100];
	int count;

};

struct Fruit Fruits[5];



void setFruits(){

	strcpy(Fruits[0].fruit_name,"apple");
	strcpy(Fruits[1].fruit_name,"mango");
	strcpy(Fruits[2].fruit_name,"banana");
	strcpy(Fruits[3].fruit_name,"chikoo");
	strcpy(Fruits[4].fruit_name,"papaya");

	for(int i=0;i<5;i++){
		Fruits[i].count=5;
	}

}



struct Args{

    int connfd;
};


void *cs_interaction(void *vargp){

	struct Args *args=(struct Args *)vargp;

	int connfd=args->connfd;

	printf("new Client connected \n");

	char buff[MAX];
	struct Fruit fruit_buff;

	while(1){
		bzero(buff, sizeof(buff));
		
		// read the message from client and copy it in buffer
		if(read(connfd, buff, sizeof(buff))==0){
			printf("Client connection closed\n");
			pthread_exit(NULL);
		}


		if(strncmp("Fruits",buff,6)==0){
		
			bzero(buff,MAX);

			printf("\n=> request for fruits recieved from client\n");

			strcpy(buff,"Enter the name of the fruit");

			write(connfd, buff, sizeof(buff));

			if(read(connfd, &fruit_buff, sizeof(fruit_buff))==0){
				printf("Client connection closed\n");
				pthread_exit(NULL);
			}

			printf("fruit requested : %s count requested : %d\n",fruit_buff.fruit_name,fruit_buff.count);
			
			bzero(buff,MAX);

			int found=0;

			for(int i=0;i<5;i++){
				if(strcmp(fruit_buff.fruit_name,Fruits[i].fruit_name)==0){
					if(fruit_buff.count>Fruits[i].count){
						strcpy(buff,"Not available");
						write(connfd, buff , sizeof(buff));
					}else{
						Fruits[i].count-=fruit_buff.count;
						strcpy(buff,"Request has been processed");
						write(connfd, buff , sizeof(buff));
					}
					found=1;
					break;
				}
			}

			if(!found){
				strcpy(buff,"this fruit is not sold here");
				write(connfd, buff , sizeof(buff));
			}

		}else if(strncmp("SendInventory", buff, 13) == 0){
			bzero(buff,MAX);

			printf("\n=> request for inventory recieved from client\n");

			// here
			write(connfd, Fruits , sizeof(Fruits));

		}else if(strncmp("exit", buff, 4) == 0){

			printf("Server Exit...\n");
			break;

		}else{
			bzero(buff,MAX);
			strcpy(buff,"invalid choice");
			write(connfd,buff,sizeof(buff));
		}
		

	}



}

pthread_t thread_id[100];

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
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);
   
	// Accept the data packet from client and verification
	int connection_i=0;	

	while(1){
		connfd = accept(sockfd, (SA*)&cli, &len);
		if (connfd < 0) {
			printf("server accept failed...\n");
			exit(0);
		}
		else
			printf("server accept the client...\n");

		
		struct Args args;
		args.connfd=connfd;
		

		pthread_create(&thread_id[connection_i++], NULL, cs_interaction, (void *) &args);
	
		if (connection_i >= 50) {
            // Update i
            connection_i = 0;
 
            while (connection_i < 50) {
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

	setFruits();

	establish_connection();

	return 0;
}