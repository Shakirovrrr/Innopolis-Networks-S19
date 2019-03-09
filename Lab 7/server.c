#include <stdio.h>
#include "networking.h"

void *responser(void *args) {
	int sockfd = init_tcp_connection();
	if (sockfd < 0) {
		perror("Cannon establish TCP connection.\n");
		exit(EXIT_FAILURE);
	}
	
	int result = 0;

	int readbuf = 0;
	int sendbuf = 0;
	result = recvsend(sockfd, (void *) &readbuf, sizeof(int), &sendbuf, sizeof(int));
}

int main(int argc, char *argv[]) {
	return 0;
}
