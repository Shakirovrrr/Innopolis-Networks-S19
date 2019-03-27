#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

#include "networking.h"
#include "linkedlist.h"
#include "common.h"

char node_name[NODENAME_LEN];
pthread_t listening_thread, syncer_thread;
char running;

LinkedList *nodepool;
LinkedList *filepool;

void form_node_msg(char *msg, NetworkNode node) {
	sprintf(msg, "%s:%s:%d", node.name, node.ipaddr, node.port);
}

void gently_shitdown(int signum) {
	running = 0;
	pthread_exit(NULL);
}

void *file_request_handler(void *args) {
	struct FileRequest request = *((struct FileRequest *) args);
	int talk_sock = request.sockfd;
	ssize_t result = 0;

	FILE *file = fopen(request.filename, "r");
	if (!file) {
		int errnum = -1;
		result = send(talk_sock, &errnum, sizeof(int), 0);
		close(talk_sock);
		pthread_exit(NULL);
	}

	unsigned int wordcount = 0;
	char wordbuf[WORD_LEN];
	while ((result = fscanf(file, "%s", wordbuf)) != 1) wordcount++;
	rewind(file);
	result = send(talk_sock, &wordcount, sizeof(unsigned int), 0);
	for (int i = 0; i < wordcount; ++i) {
		fscanf(file, "%s", wordbuf);
		result = send(talk_sock, &wordbuf, sizeof(wordbuf), 0);
	}
	// FIXME Errors are not being handled!!!11!!11

	close(talk_sock); // FIXME Should I close it?
}

void *sync_handler(void *args) {
	int talk_sock = *((int *) args);
	ssize_t result = 0;

	int node_signal = NODE_SYNC;

	result = send(talk_sock, &node_signal, sizeof(node_signal), 0);

	if (result < 0) {
		perror("Can't send signal.\n");
		pthread_exit(NULL);
	}

	char sendbuf[SENDBUF_LEN];
	char ipstr[INET_ADDRSTRLEN];
	int portval;
	unsigned int printf_offset = 0;
	get_my_ip(&portval, ipstr);
	printf_offset += sprintf(sendbuf, "%s:%s:%d:[", node_name, ipstr, portval);

	char filename[FILENAME_LEN];
	for (size_t i = 0; i < filepool->size; ++i) {
		memcpy(filename, getVal(filepool, i), sizeof(filename));
		printf_offset += sprintf(sendbuf + printf_offset,
		                         "%s,", filename);
	}
	printf_offset += sprintf(sendbuf + printf_offset - 1, "]");

	result = send(talk_sock, sendbuf, sizeof(char) * (printf_offset + 1), 0);

	if (result < 0) {
		perror("Can't send file list.\n");
		pthread_exit(NULL);
	}

	int n_known_nodes = (int) nodepool->size;
	result = send(talk_sock, &n_known_nodes, sizeof(n_known_nodes), 0);

	if (result < 0) {
		perror("Can't send number of known nodes.\n");
		pthread_exit(NULL);
	}

	NetworkNode known_node;
	char nodeaddr[NODENAME_LEN + INET_ADDRSTRLEN + 8];
	for (size_t j = 0; j < n_known_nodes; ++j) {
		memcpy(&known_node, getVal(nodepool, j), sizeof(NetworkNode));
		form_node_msg(nodeaddr, known_node);

		result = send(talk_sock, nodeaddr, sizeof(nodeaddr), 0);

		if (result < 0) {
			perror("Can't send node address.\n");
			pthread_exit(NULL);
		}
	}
}

void *syncer(void *args) {
	signal(SIGINT, gently_shitdown);
	// TODO Syncer
}

void *incoming_handler(void *args) {
	signal(SIGINT, gently_shitdown);
	running = 1;

	pthread_create(&syncer_thread, NULL, syncer, NULL);

	int in_sock = init_tcp_server(2000);
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	int talk_sock = 0;
	int request_buf = -1;
	long result;
	int *handler_sock = malloc(sizeof(int));

	while (running) {
		talk_sock = accept(in_sock, (struct sockaddr *) &their_addr, &addr_len);
		if (talk_sock < 0) {
			perror("Cannot create talk socket.\n");
			continue;
		}

		result = recv(talk_sock, &request_buf, sizeof(request_buf), 0);
		if (result < 0) {
			perror("Did not receive the request.\n");
			continue;
		}

		*handler_sock = talk_sock;
		pthread_t thread_id;
		if (request_buf == NODE_SYNC) {
			printf("Received sync request...\n");
			pthread_create(&thread_id, NULL, sync_handler, handler_sock);
		} else if (request_buf == NODE_FILE_REQUEST) {
			printf("Received file request...\n");
			// TODO Get file name
			pthread_create(&thread_id, NULL, file_request_handler, handler_sock);
		} else {
			printf("Received unknown request.\n");
		}
	}

	free(handler_sock);
	close(in_sock);
	close(talk_sock);
}

int main(int argc, char *argv[]) {
	nodepool = newLinkedList();
	pthread_create(&listening_thread, NULL, incoming_handler, NULL);
	pthread_join(listening_thread, NULL);

	return 0;
}
