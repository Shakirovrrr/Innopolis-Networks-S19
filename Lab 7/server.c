#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "networking.h"
#include "linkedlist.h"

#define NODE_SEND_INFO 1
#define NODE_GET_LIST 2
#define NODE_GET_PEERS 3

LinkedList *nodepool;
LinkedList *filepool;

void *node_handler(void *args) {

}

void *connections_handler() {
	int master_socket = init_tcp_server();

}

void *node_job(void *args) {
	struct sockaddr_in *addr = (struct sockaddr_in *) args;

}

int main(int argc, char *argv[]) {
	nodepool = newList();

	pthread_t handler_thread = 0;
	pthread_t node_thread = 0;
	struct sockaddr_in *addr = NULL;
	if (argc > 1) {
		if (strcmp(argv[1], "-main") == 0) {
			pthread_create(&handler_thread, NULL, connections_handler, NULL);
		} else {
			addr = malloc(sizeof(struct sockaddr_in));
			if (convert_address(argv[1], addr) < 0) {
				perror("Invalid address.\n");
			}
			pthread_create(&node_thread, NULL, node_job, (void *) addr);
		}
	}

	if (handler_thread) {
		pthread_join(handler_thread, NULL);
	}
	if (node_thread) {
		pthread_join(node_thread, NULL);
	}

	if (addr) {
		free(addr);
	}

	return 0;
}
