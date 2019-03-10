#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "networking.h"

int *nodepool;
int nnodes, poolsize;

void add_node(int sockfd) {
	int *iter = nodepool;

	iter[nnodes] = sockfd;
	nnodes++;
	if (nnodes == poolsize - 2) {
		poolsize += 10;
		nodepool = realloc(nodepool, sizeof(int) * poolsize);
		memset(nodepool + nnodes, 0, sizeof(int) * (poolsize - nnodes));
	}
}

void remove_node(int sockfd) {
	close(sockfd);
	int n = -1;
	for (int i = 0; i < poolsize; ++i) {
		if (nodepool[i] == sockfd) {
			n = i;
			break;
		}
	}

	if (n < 0) return;

	nodepool[n] = nodepool[poolsize - 1];
	nodepool[poolsize - 1] = 0;
	nnodes--;

	if (nnodes * 2 < poolsize) {
		poolsize /= 2;
		nodepool = realloc(nodepool, sizeof(int) * poolsize);
	}
}

void *connections_handler(void *args) {
	int master_socket = *((int *) args);
	int sockfd = setup_communication(master_socket);
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
		//exit(EXIT_FAILURE);
	}

	if (readbuf == 0) {
		send(sockfd, (const void *) sendbuf, sizeof(int), 0);
	}

	if (readbuf == 1) {
		int port = 0;
		char ipaddress[INET_ADDRSTRLEN];
		get_ip_port(sockfd, &port, (char **) ipaddress);
		printf("Got new node! IP: %s, port: %d", ipaddress, port);
		// TODO Add connections
	}
}

int main(int argc, char *argv[]) {
	nodepool = calloc(10, sizeof(int));
	nnodes = 0;
	poolsize = 10;
	int main_socket = init_tcp_server();
	pthread_t handler_thread = 0;
	pthread_create(&handler_thread, NULL, connections_handler, (void *) &main_socket);

	return 0;
}
