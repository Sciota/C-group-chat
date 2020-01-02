#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <pthread.h>

int check(int result, char *err){

	if (result < 0){
	
		perror(err);
		exit(1);
	}

	return result;
}

void *recv_msg(void * fd){

	int sockfd = *((int *)fd);
	char msg[124];

	while (1){ //start listening for messages and print

		int bytes = recv(sockfd, msg, 124, 0);
		msg[bytes] = '\0';

		puts(msg);
	}

	return NULL;
}

void send_msg(int sockfd){

	char msg[124];
	
	while (1){

		fgets(msg, 124, stdin);
		send(sockfd, msg, strlen(msg), 0);
	}
}


int main(void){
	
	pthread_t recvTh;
	
	//setting up the connection file des, address and connect
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in connection;

	connection.sin_family = AF_INET;
	connection.sin_port = htons(1234);
	connection.sin_addr.s_addr = inet_addr("127.0.0.1");

	check(connect(sockfd, (struct sockaddr *)&connection, sizeof(connection)), "Could not connect");

	pthread_create(&recvTh, NULL, recv_msg, (void *)&sockfd); //thread for incoming messages
	send_msg(sockfd); //sending messages on main thread

}
