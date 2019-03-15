#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "networking.h"
#include "linkedlist.h"

LinkedList *nodepool;

void *node_handler(void *args) {
	int conn_socket = *((int *) args);
	int alive = 1;


	while (alive) {
		alive = ping(conn_socket, 1);
		sleep(1);
	}

	int port;
	char ipaddr[INET_ADDRSTRLEN];
	get_ip_port(conn_socket, &port, (char **) ipaddr);
	printf("Node %s:%d left.\n", ipaddr, port);
	close(conn_socket);

	unsigned int ipuint32 = get_uint_ip(conn_socket);
	deleteVal(nodepool, findVal(nodepool, ipuint32));
}

void *connections_handler() {
	int master_socket = init_tcp_server();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
	while (1) {
		int sockfd = setup_communication(master_socket);
		if (sockfd < 0) {
			perror("Cannot establish TCP connection.\n");
			continue;
		}

		int result = 0;

		int readbuf = 0;
		int sendbuf = 0;
		result = recvsend(sockfd, (void *) &readbuf, sizeof(int), &sendbuf, sizeof(int));
		if (result < 0) {
			perror("Cannot recieve/send message.\n");
			//exit(EXIT_FAILURE);
		}

		if (readbuf == 0) {
			send(sockfd, (const void *) &sendbuf, sizeof(int), 0);
		}

		if (readbuf == 1) {
			int port = 0;
			char ipaddress[INET_ADDRSTRLEN];
			get_ip_port(sockfd, &port, (char **) ipaddress);
			printf("Got new node! IP: %s, port: %d", ipaddress, port);
			pthread_t new_thread;
			pthread_create(&new_thread, NULL, node_handler, (void *) &sockfd);
			insertVal(nodepool, 0, get_uint_ip(sockfd));
		}
	}
#pragma clang diagnostic pop
}

void *node_job(void *args) {

}

int main(int argc, char *argv[]) {
	nodepool = newList();

	pthread_t handler_thread = 0;
	pthread_t node_thread = 0;
	if (argc > 1) {
		if (strcmp(argv[1], "-main") == 0) {
			pthread_create(&handler_thread, NULL, connections_handler, NULL);
		} else {
			struct sockaddr_in *addr = malloc(sizeof(struct sockaddr_in));
			if (convert_address(argv[1], addr) < 0) {
				perror("Invalid address.\n");
			}
			pthread_create(&node_thread, NULL, node_job, (void *) addr);
		}
	}

	pthread_join(node_thread, NULL);
	pthread_join(handler_thread, NULL);

	return 0;
}
