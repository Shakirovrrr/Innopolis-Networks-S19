#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#include "networking.h"
#include "linkedlist.h"
#include "common.h"

char node_name[NODENAME_LEN];
in_port_t main_port;
pthread_t listening_thread, syncer_thread;
char shared_dir[MAX_DIRLEN];
char running;

LinkedList *nodepool;
LinkedList *filepool;

void form_node_msg(char *msg, NetworkNode node) {
	sprintf(msg, "%s:%s:%d", node.name, node.ipaddr, node.port);
}

int form_file_str(char *str) {
	DIR *shared = opendir(shared_dir);
	if (!shared) {
		return -1;
	}

	int str_offset = sprintf(str, "[]");
	str_offset--;

	struct dirent *entry;
	while ((entry = readdir(shared))) {
		str_offset += sprintf(str + str_offset, "%s,", entry->d_name);
	}
	str_offset += sprintf(str + str_offset - 1, "]");

	return str_offset;
}

void gently_shitdown(int signum) {
	running = 0;
	pthread_exit(NULL);
}

void download_file(NodeFile node_file) {
	long int result = 0;

	int sock_talk = init_tcp_client(node_file.node.ipaddr, node_file.node.port);
	if (sock_talk < 0) {
		perror("Cannot download file (socket creation failed).\n");
		return;
	}

	int request = NODE_FILE_REQUEST;
	result = send(sock_talk, &request, sizeof(request), 0);
	if (result < 0) {
		perror("Cannot download file (unable to send request).\n");
		return;
	}

	int wordcount = 0;
	result = recv(sock_talk, &wordcount, sizeof(wordcount), 0);
	if (result < 0) {
		perror("Cannot download file (unable receive word count).\n");
		return;
	}
	if (wordcount < 0) {
		perror("Cannot download file (there\'s no file).\n");
		return;
	}

	FILE *file = fopen(node_file.name, "w");
	char wordbuf[WORD_LEN];
	for (int i = 0; i < wordcount; ++i) {
		result = recv(sock_talk, wordbuf, sizeof(wordbuf), 0);
		fprintf(file, "%s ", wordbuf);
	}

	close(sock_talk);
}

void *file_request_handler(void *args) {
	struct FileRequest request = *((struct FileRequest *) args);
	int sock_talk = request.sockfd;
	ssize_t result = 0;

	FILE *file = fopen(request.filename, "r");
	if (!file) {
		int errnum = -1;
		result = send(sock_talk, &errnum, sizeof(int), 0);
		close(sock_talk);
		pthread_exit(NULL);
	}

	unsigned int wordcount = 0;
	char wordbuf[WORD_LEN];
	while ((result = fscanf(file, "%s", wordbuf)) != 1) wordcount++;
	rewind(file);
	result = send(sock_talk, &wordcount, sizeof(wordcount), 0);
	for (int i = 0; i < wordcount; ++i) {
		fscanf(file, "%s", wordbuf);
		result = send(sock_talk, &wordbuf, sizeof(wordbuf), 0);
	}
	// FIXME Errors are not being handled!!!11!!11

	close(sock_talk); // FIXME Should I close it?
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
	printf_offset += sprintf(sendbuf, "%s:%s:%d:", node_name, ipstr, portval);

	char filename[FILENAME_LEN];
	int file_str_size = form_file_str(filename);
	strcat(sendbuf, filename);

	result = send(talk_sock, sendbuf, sizeof(char) * (printf_offset + file_str_size + 1), 0);

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
	char nodeaddr[PEERSTR_LEN];
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

void parse_node(char nodedata[SENDBUF_LEN]) {
	char delim[] = ":";
	char *tok = strtok(nodedata, delim);
	NetworkNode *node = malloc(sizeof(NetworkNode));
	sprintf(node->name, "%s", tok);

	tok = strtok(NULL, delim);
	sprintf(node->ipaddr, "%s", tok);

	tok = strtok(NULL, delim);
	char portnum[6];
	sprintf(portnum, "%s", tok);
	node->port = (in_port_t) strtoul(portnum, NULL, 0);
	tok = strtok(NULL, delim);

	insertVal(nodepool, 0, node);

	size_t filelist_len = strlen(tok);
	if (filelist_len > 2) {
		char filelist[SENDBUF_LEN];
		memcpy(filelist, tok + 1, sizeof(char) * (filelist_len - 2));
		filelist[filelist_len - 1] = '\0';

		char filedelim[] = ",";
		tok = strtok(filelist, filedelim);
		while (tok) {
			NodeFile *file = malloc(sizeof(NodeFile));
			strcpy(file->name, tok);
			file->node = *node;

			if (findVal(filepool, file, sizeof(NodeFile)) < 0) {
				insertVal(filepool, 0, file);
			}

			tok = strtok(NULL, filedelim);
		}
	}
}

void remove_inactive_node(NetworkNode node) {
	for (size_t i = 0; i < nodepool->size; ++i) {
		if (memcmp(&node, getVal(nodepool, i), sizeof(NetworkNode)) == 0) {
			deleteVal(nodepool, i);
			break;
		}
	}
}

void parse_peer(char peerdata[PEERSTR_LEN]) {
	char delim[] = ":";
	char *tok = strtok(peerdata, delim);
	NetworkNode *node = malloc(sizeof(NetworkNode));
	sprintf(node->name, "%s", tok);

	tok = strtok(NULL, delim);
	sprintf(node->ipaddr, "%s", tok);

	tok = strtok(NULL, delim);
	node->port = (in_port_t) strtoul(tok, NULL, 0);

	if (findVal(nodepool, node, sizeof(NetworkNode)) < 0) {
		insertVal(nodepool, 0, node);
	}
}

void send_sync(int sync_sock, NetworkNode node) {
	long int result = 0;
	int message = NODE_SYNC;
	result = send(sync_sock, &message, sizeof(message), 0);

	message = -1;
	result = recv(sync_sock, &message, sizeof(message), 0);
	if (result < 0 || message < 0) {
		perror("Cannot get sync message.\n");
		remove_inactive_node(node);
		return;
	}

	char buffer[SENDBUF_LEN];
	result = recv(sync_sock, buffer, sizeof(buffer), 0);
	parse_node(buffer);

	message = 0;
	result = recv(sync_sock, &message, sizeof(message), 0);
	if (result < 0) {
		perror("Cannot receive number of known nodes.\n");
		return;
	}

	char peerbuf[PEERSTR_LEN];
	for (int i = 0; i < message; ++i) {
		result = recv(sync_sock, peerbuf, sizeof(peerbuf), 0);
		parse_peer(peerbuf);
	}

}

void *syncer(void *args) {
	signal(SIGINT, gently_shitdown);

	int sync_sock = 0;
	NetworkNode node;
	while (running) {
		for (size_t i = 0; i < nodepool->size; ++i) {
			node = *((NetworkNode *) getVal(nodepool, i));
			sync_sock = init_tcp_client(node.ipaddr, node.port);
			send_sync(sync_sock, node);
			close(sync_sock);
			sleep(1);
		}
		sleep(1);
	}
}

void *incoming_handler(void *args) {
	signal(SIGINT, gently_shitdown);
	running = 1;

	pthread_create(&syncer_thread, NULL, syncer, NULL);

	int in_sock = init_tcp_server(main_port);
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

		switch (request_buf) {
			case NODE_SYNC:
				printf("Received sync request...\n");
				pthread_create(&thread_id, NULL, sync_handler, handler_sock);

				break;

			case NODE_FILE_REQUEST:
				printf("Received file request...\n");
				char filename[FILENAME_LEN];
				result = recv(talk_sock, filename, sizeof(filename), 0);
				if (result < 0) {
					perror("Cannot receive file name.\n");
					continue;
				}

				struct FileRequest file_request;
				strcpy(file_request.filename, filename);
				file_request.sockfd = talk_sock;
				pthread_create(&thread_id, NULL, file_request_handler, &file_request);

				break;

			default:
				printf("Received unknown request.\n");
				break;
		}
	}

	free(handler_sock);
	close(in_sock);
	close(talk_sock);
}

void retrieve_file(char filename[FILENAME_LEN]) {
	FILE *file = fopen(filename, "r");
	if (!file) {

	}
}

void *user_repl(void *args) {
	while (running) {
		printf("File to retrieve: ? ");
		char fileretr[FILENAME_LEN];
		scanf("%s", fileretr);
		printf("We will try...\n");


	}
}

int main(int argc, char *argv[]) {
	printf("Files directory to share: ? ");
	scanf("%s", shared_dir);
	printf("\n");

	char decided = 0;
	while (!decided) {
		printf("Connect to another node: (y/n)? ");
		char ans[4];
		scanf("%s", ans);
		if (ans[0] == 'y' || ans[0] == 'Y') {
			char addr[INET_ADDRSTRLEN];
			printf("IPv4 address: ? ");
			scanf("%s", addr);
			printf("\n");

			in_port_t port = 0;
			while (port == 0) {
				printf("Port: ? ");
				char portbuf[6];
				scanf("%s", portbuf);
				port = (in_port_t) strtoul(portbuf, NULL, 0);
				printf("\n");
			}
			decided = 1;
		} else if (ans[0] == 'n' || ans[0] == 'N') {
			decided = 1;
		} else {
			printf("Yes/No: ? ");
		}
	}

	nodepool = newLinkedList();
	filepool = newLinkedList();

	pthread_create(&listening_thread, NULL, incoming_handler, NULL);
	pthread_join(listening_thread, NULL);

	return 0;
}
