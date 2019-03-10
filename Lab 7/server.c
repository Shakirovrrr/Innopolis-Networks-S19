#include <stdio.h>
#include <pthread.h>
#include "networking.h"

void *responser(void *args) {
	int sockfd = *((int *) args);
	if (sockfd < 0) {
		perror("Cannon establish TCP connection.\n");
		exit(EXIT_FAILURE);
	}
	
	int result = 0;

	int readbuf = 0;
	int sendbuf = 0;
	result = recvsend(sockfd, (void *) &readbuf, sizeof(int), &sendbuf, sizeof(int));
	close(sockfd);
	if (result < 0) {
		perror("Cannot recieve/send message.\n");
		exit(EXIT_FAILURE);
	}
}

int ping(int conn_socket, int n) {
	int success = 0;
	int sockfd = 0;
	int result = 0;

	int sendbuf = 0, readbuf = -1;
	for(size_t i = 0; i < n; i++) {
		sockfd = setup_communication(conn_socket);
		if (sockfd < 0) {
			printf("Cannot ping.\n");
			sockfd = 0;
			continue;
		}

		result = sendrecv(sockfd, &sendbuf, sizeof(int), &readbuf, sizeof(int));
		if (result < 0) {
			printf("Cannot send/recieve message.\n");
			close(sockfd);
			result = 0;
			continue;
		}

		if (readbuf == 0) {
			success++;
			printf("Sent and recieved %lu bytes.\n", sizeof(int));
		}

		close(sockfd);
		readbuf = -1;
	}
	
	return success;
}

int main(int argc, char *argv[]) {
	int main_socket = init_tcp_server();
	pthread_t resp_thread = 0;
	pthread_create(&resp_thread, NULL, responser, (void *) &main_socket);

	return 0;
}
