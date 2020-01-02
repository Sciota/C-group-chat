#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <pthread.h>

#define BACK_LOG 30


struct new_user{

	int id;
	char name[20];
	int sockfd;
};

typedef struct new_user User;

//global array of users connection, sets every .id in the array to 0
User user[BACK_LOG] = { [0 ... BACK_LOG-1] = { .id = 0 } }; 



int removeUser(int);


int check(int result, char *err){ //check function to control return value of functions

	if (result < 0){
	
		perror(err);
		exit(1);
	}

	return result;
}

void broadcast(char *msg, int senderfd){ //send everyone msg except for the sender
	
	for (size_t i = 0; i < BACK_LOG; i++){

		if (user[i].sockfd == senderfd) continue;

		else{
			send(user[i].sockfd, msg, strlen(msg), 0);
		}

	}
}

void message_handler(User messanger){ //receives and broadcast the meassge

	char msg[124];
	char format_msg[144];
	int bytes;

	while (1){

		bytes = recv(messanger.sockfd, msg, 124, 0);
		msg[bytes -1] = '\0';
		
		if (msg[0]){ //check if the message has content

			if ( strncmp("exit()", msg, 4) == 0){

				sprintf(format_msg, "<%s> left the server", messanger.name);
				
				puts(format_msg);
				broadcast(format_msg, messanger.sockfd);
				break;
			}

			sprintf(format_msg, "%s: %s", messanger.name, msg); // name: message 

			puts(format_msg); 
			broadcast(format_msg, messanger.sockfd);
		}

	}

	close(messanger.sockfd);
	removeUser(messanger.id);

}


void *client_handler(void *userPtr){
	
	User user_handle = (*(User *)userPtr);
	int clientfd = user_handle.sockfd;
	
	char msg[124];
	char name[20];
	int bytes;
	char confirm;
	
	send(user_handle.sockfd, "Welcome to the server! Enter your name to continue\n", 55, 0);

	while (1){ //register username to the server

		send(user_handle.sockfd, "Enter your name:", 17, 0);

		bytes = recv(clientfd, name, 20, 0);
		name[bytes -1] = '\0'; // -1 removes the \n char

		sprintf(msg, "Confirm <%s> y/n:", name); 
		send(clientfd, msg, strlen(msg), 0);

		bytes = recv(clientfd, msg, 10, 0);
		msg[bytes] = '\0';
		confirm = msg[0];

		if (tolower(confirm) == 'y'){
			
			strncpy(user_handle.name, name, strlen(name));
			send(user_handle.sockfd, "You're in! use the exit() command to quit the server.\n", 55, 0);
			break;
		}
			
	}
	
	char joinmsg[40];
	sprintf(joinmsg, "<%s> joined the server\n", user_handle.name);

	puts(joinmsg);
	broadcast(joinmsg, user_handle.sockfd); //sends join messages to other users

	message_handler(user_handle); //start handling the user messages

	return NULL;
}


int addUser(int fd){

	pthread_t handlerTh;

	for (size_t i = 1; i < BACK_LOG-1; i++){ //because of id managment, i starts at 1

		if (user[i].id == 0){
			
			user[i].id = i;
			user[i].sockfd = fd;

			pthread_create(&handlerTh, NULL, client_handler, &user[i]);
			return 0;
		}
	}
}

int removeUser(int userID){

	for (size_t i = 1; i < BACK_LOG-1; i++){

		if (user[i].id == userID){

			//turn all the stats to default
			user[i].id = 0;
			user[i].sockfd = 0;
			memset(user[i].name, '\0', 20);
			
			return 0;
		}
	}

	return 1;
}


int main(int argc, char *argv[]){

	printf("%d\n", user[9].sockfd);

	if (argc < 3){

		puts("Usage: ./script <host> <port>");
		return 1;
	}

	int sockaddr_size = sizeof(struct sockaddr_in);

	//thread variables
	

	//setting up the server file desc, address and clients address
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(strtod(argv[2], NULL)); 		     
	server_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	
	int c_index = 0; //will be the index for the user struct array

	//binding server
	check(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)), "Could not bind");

	//start handling the connections
	puts("Listening for connections..");	
	while (1){
		
		check(listen(sockfd, BACK_LOG), "listen error"); //listen and save clients connections

		addUser( accept(sockfd, NULL, &sockaddr_size));
		
		

	}	

	return 0;

}
