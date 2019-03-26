#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include "networking.h"
#include "linkedlist.h"

#define NODE_REQUEST 0
#define NODE_SYNC 1

#define FILENAME_LEN 24

int main_sockfd;
char node_name[32];

LinkedList *nodepool;
LinkedList *filepool;

void *msg_handler(void *args) {

}

void *sync_handler(void *args) {
	struct sockaddr_in addr_temp = *((struct sockaddr_in *) args);
	ssize_t result = 0;

	int node_signal = NODE_SYNC;

	result = sendto(main_sockfd, &node_signal, sizeof(node_signal), 0,
	                (const struct sockaddr *) &addr_temp, sizeof(addr_temp));

	if (result < 0) {
		perror("Can't send signal.\n");
		exit(EXIT_FAILURE);
	}

	char sendbuf[512];
	char ipstr[INET_ADDRSTRLEN];
	int portval;
	unsigned int printf_offset = 0;
	get_ip_port(main_sockfd, &portval, ipstr);
	printf_offset += sprintf(sendbuf, "%s:%s:%d:", node_name, ipstr, portval);

	char filename[FILENAME_LEN];
	if (filepool->size > 0) {
		memcpy(filename, getVal(filepool, 0), sizeof(filename));
		printf_offset += sprintf(sendbuf + printf_offset, "%s", filename);
	}
	for (size_t i = 1; i < filepool->size; ++i) {
		memcpy(filename, getVal(filepool, i), sizeof(filename));
		printf_offset += sprintf(sendbuf + printf_offset,
		                         ",%s", filename);
	}

	result = sendto(main_sockfd, sendbuf, sizeof(char) * (printf_offset + 1), 0,
	                (const struct sockaddr *) &addr_temp, sizeof(addr_temp));

	if (result < 0) {
		perror("Can't send file list.\n");
		exit(EXIT_FAILURE);
	}

	int n_known_nodes = (int) nodepool->size;
	result = sendto(main_sockfd, &n_known_nodes, sizeof(n_known_nodes), 0,
	                (const struct sockaddr *) &addr_temp, sizeof(addr_temp));

	if (result < 0) {
		perror("Can't send number of known nodes.\n");
		exit(EXIT_FAILURE);
	}

	char nodeaddr[INET_ADDRSTRLEN + 6];
	for (int j = 0; j < n_known_nodes; ++j) {
		result = sendto(main_sockfd, nodeaddr, sizeof(nodeaddr), 0,
		                (const struct sockaddr *) &addr_temp, sizeof(addr_temp));

		if (result < 0) {
			perror("Can't send node address.\n");
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[]) {
	nodepool = newLinkedList();


	return 0;
}
