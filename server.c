#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

struct node {
	int client;
	int uid;
	char name[20];
	struct node *next;
};
struct node *head, *sender;


struct node *find_last_node() {
	struct node *cur = head;
	struct node *next;
	while(cur) {
		next = cur->next;
		if (next == NULL) {
			return (void*)cur;
		}
		cur = cur->next;
	}
	return NULL;
}

struct node *remove_node(int clientID){
	struct node *cur = head;
	struct node *prev = head;
	if (head->client == clientID) {
		head = head->next;
		return head;
	}
	else {
	    while(cur) {
		if (cur->client == clientID) {
		    prev->next = cur->next;
		    return prev;
		}
		prev = cur;
		cur = cur->next;
	    }
	}
}

void send_message(char *message) {
    struct node *send_node = head;
        while (send_node) {
            if (send_node->client != sender->client) {	
	        int n = write(send_node->client, message, strlen(message) );
	        if (n == -1) {
	            perror("Error sending message to clients");
	            break;
	        }
             } 
             send_node = send_node->next;
        } 
}

void *check_message() {
    struct node *cur_node = head; 
    char input[1024];
    while (cur_node != NULL) {
	    bzero(input, 1024);
	    sender = cur_node;

	    // Read message from client to buffer
	    int ret = read(cur_node->client, input, 1024);
	    if (ret == -1) {
	        if (errno == EAGAIN || errno == EWOULDBLOCK) {
			//Not a real error, ignore it, do nothing in this code path
		} else {
			perror("Could not read from client");
			// Remove cur_node from list
			char* message = malloc(30);
			sprintf(message, "%s left the chat\n",cur_node->name);
			cur_node = remove_node(cur_node->client);
			send_message(message);
			free(message);
			close(sender->client);

			
		}
	    }  
	    //NOTE: If return value is equal to zero, that means that the client on the other
	    // end has hit END OF FILE and will never send anything else, we should consider
	    // it disconnected and remove it from the list. 
	    else if (ret == 0) {
		// Remove cur_node from list
		char* message = malloc(30);
		sprintf(message, "%s left the chat\n",cur_node->name);
		cur_node = remove_node(cur_node->client);
		send_message(message);
		free(message);
		close(sender->client);

	    }
	    else if (ret > 0) {
	        sender = cur_node;
	        char *message = malloc(1024);

		// check for keywords
		if (strncmp("quit", input,4) == 0) {
			printf("Closing client %s \n", cur_node->name);
			// Remove node/client from list. cur_node will be reset to the head or 
			// to the prev node so that a node won't be skipped
			sprintf(message, "%s left the chat\n",cur_node->name);
			cur_node = remove_node(cur_node->client);
			close(sender->client);
		}
		else if (strncmp("name", input, 4) == 0) {
			// change the name of the current node/client
			char* input_token = strtok(input, " ");
			char* name_input = strtok(NULL," ");
			if (name_input[1] != '\0') {
			    sprintf(message,"%s has changed their name to %s",cur_node->name, name_input);
			    printf("%s",message);
			    // strip newline character from name_input
			    name_input[strcspn(name_input,"\n")]=0;
			    sprintf(cur_node->name, "%s",name_input);
			}
		}
		else {
		// Format the message to send
		    sprintf(message, "%s: %s", sender->name, input);
		    printf("%s", message);
		}
	    
		// Send the message to all the other clients
	        send_message(message); 
	        free(message);	
            }	    
	    // Just in case removing a node makes the current one null
	    if (cur_node != NULL) {
	        cur_node = cur_node->next;
	    }
	    sleep(0.01);	
    }
}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: <port number>\n");
		return -1;
	}
	int PORT_NUM = atoi(argv[1]);

	// Variables for linked list
	struct node *cur_node;
	struct node *next_node;
	int cur_client;

	// Create the socket
	printf("Creating socket\n");
	int listenFD = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (listenFD == -1) {
		perror("Error creating socket");
		return -1;
	}

	printf("Binding Socket\n");
	// Bind socket address
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT_NUM);
	address.sin_addr.s_addr = INADDR_ANY;

	int ret = bind(listenFD, (struct sockaddr*)&address, sizeof(struct sockaddr_in));
	if (ret == -1) {
		perror("Error binding socket");
		return -1;
	}

	printf("Setting socket to listening mode\n");
	// Set socket to listening mode
	ret = listen(listenFD, 10);
	if (ret == -1) {
		perror("Error setting socket to listening mode");
		return -1;
	}

	printf("Establishing connections\n");
	// Accept connections
	int users = 0;
	char *message[1024];
	while(1) {
		int clientFD = accept4(listenFD, NULL, 0,SOCK_NONBLOCK);
		*message = NULL;
		if (clientFD == -1) {	
			if (errno == EAGAIN ||  errno == EWOULDBLOCK) {
				// Go through the client list to check each client
				check_message();

			} else {
				perror("Error with accept4");
				return -1;
			}
		}
		else if (clientFD >= 0){
			users++;
			char* message = malloc(30);
			// Create initial linked list to track all clients
			if ((void*)head == NULL) {
				head = malloc( sizeof(struct node) );
				head->client = clientFD;
				head->uid = users;
				sprintf(head->name, "User %i",users);
				head->next = NULL;
				sender = head;
			}
			// Otherwise, add a new node to the list
			else {
				cur_node = find_last_node();
				next_node = malloc( sizeof(struct node) );
				cur_node->next = next_node;
				next_node->client = clientFD;
				next_node->uid = users;
				sprintf(next_node->name, "User %i", users);
				next_node->next = NULL;
				sender = next_node;
			}
			sprintf(message, "User %i has connected\n", users);
			send_message(message);
			free(message);
			printf("User %d connected\n", users);
		}
		sleep(0.01);
	}
	printf("Outside accepting while loop\n");
	return 0;
}

