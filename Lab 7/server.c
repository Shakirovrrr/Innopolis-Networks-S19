#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 2000

void *responser(void *args) {
	int result = 0;
	int sockfd = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sockfd < 0) {
		perror("Responder socket creation failed.\n");
		exit(EXIT_ERROR);
	}

	struct sockaddr_in addr_resp, addr_client;

	addr_resp.sin_family = AF_INET;
	addr_resp.sin_port = SERVER_PORT;
	addr_resp.sin_addr.s_addr = INADDR_ANY;

	result = bind(sockfd, (struct sockaddr *) &addr_resp, sizeof(struct sockaddr));
	if (result < 0) {
		perror("Binding responder socket failed.\n");
		exit(EXIT_ERROR);
	}
}

int main(int argc, char *argv[]) {
	return 0;
}