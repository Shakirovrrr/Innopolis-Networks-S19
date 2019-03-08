#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "common.h"

struct client_connection {
	int sockfd;
	struct sockaddr_in addr;
};

void *server_job(void *args) {
	struct client_connection conn = *((struct client_connection *) args);
	int sockfd = conn.sockfd;
	struct sockaddr_in addr_client = conn.addr;
	int result = 0;
	free(args);

	printf("Serving client from %s.\n", inet_ntoa(addr_client.sin_addr));

	sleep(SLEEP_TIME);

	result = sendto(sockfd, (const void *)pthread_self(), sizeof(pthread_t), 0,
		   (const struct sockaddr *)&addr_client, sizeof(addr_client));
	if (result < 0) {
		printf("Cannot send datagram.");
	} else {
		printf("Sent %d bytes.\n", result);
	}
}

int main() {
	int sockfd;
	char buf[BUF_SIZE];
	int result, length;
	struct sockaddr_in addr_serv, addr_client;
	char hello[13] = "Hello server!";

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("Socket failed.");
		return EXIT_FAILURE;
	}

	memset(&addr_serv, 0, sizeof(addr_serv));
	memset(&addr_client, 0, sizeof(addr_client));

	addr_serv.sin_family    = AF_INET;
	addr_serv.sin_addr.s_addr = INADDR_ANY;
	addr_serv.sin_port = htons(PORT);

	result = bind(sockfd, (const struct sockaddr *)&addr_serv, sizeof(addr_serv));
	if(result < 0) {
		perror("Bind failed.");
		return EXIT_FAILURE;
	}

	length = sizeof(addr_client);

	while(1) {
		result = recvfrom(sockfd, (char *)buf, BUF_SIZE, 0,
						  (struct sockaddr *)&addr_client, &length);
		printf("Got client! %s\n", inet_ntoa(addr_client.sin_addr));

		buf[result] = '\0';
		printf("<Client>: %s\n", buf);

		struct client_connection *conn = malloc(sizeof(struct client_connection));
		conn->addr = addr_client;
		conn->sockfd = sockfd;

		pthread_t t_id;
		pthread_create(&t_id, NULL, server_job, (void *)conn);
	}

	close(sockfd);

	return 0;
}
