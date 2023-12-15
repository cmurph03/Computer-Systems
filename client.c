#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define SOCK_PATH "Studio_14"

char receive[1024];
char toSend[1024];
pthread_t threads[2];
int end = 0;

void* accept_message(void* listenFD) {
	FILE* sock = fdopen((int)listenFD, "r+");
	while(1) {
		bzero(receive,1024);
		// read message from the server
		fgets(receive, 1024, sock);
		if (receive[0] != '\0') {
 		    printf("%s\n", receive);
		}
		if (end == -1) {
			return 0;
		}
		sleep(0.01);
	}
	return 0;
}

void* send_message(void* listenFD) {
	while(1) {
		bzero(toSend, 1024);
		// read message from client
		fgets(toSend, 1024, stdin);
		if (strncmp("quit", toSend, 4) == 0) {
			write((int)listenFD, toSend, 1024);
			end = -1;
			exit(0);
	 	}
		else if (toSend[0] != '\0') {
		    // send to server, making sure it's not an empty string
		    int ret = write((int)listenFD, toSend, 1024);
		    if (ret == -1) {
			perror("Error writing to the server");
		    }
		}
		sleep(0.01);
	}
	return 0;
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		printf("Usage: <IP address> <port number>");
		return -1;
	}
	int PORT_NUM = atoi(argv[2]);
	char* CONNECT_ADDRESS = argv[1];

	// Create the socket
	int listenFD = socket(AF_INET,SOCK_STREAM, 0);
	if (listenFD == -1) {
		perror("Error creating socket");
		return -1;
	}

	// Bind socket address
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT_NUM);
	inet_aton(CONNECT_ADDRESS, &address.sin_addr);

	int ret = connect(listenFD, (struct sockaddr*)&address, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("Error in connecting socket address");
		return -1;
	}
	printf("Successfully connected.\n");

	ret = pthread_create(&threads[0], NULL, &send_message,(void*)listenFD);
	if (ret == -1) {
		perror("Could not send message to server");
	}
	ret = pthread_create(&threads[1],NULL,&accept_message,(void*)listenFD);
    	if (ret == -1) {
		perror("Could not receive message from server");
	}	
	pthread_join(threads[0],NULL);
	pthread_join(threads[1],NULL);
		
	return 0;
}

