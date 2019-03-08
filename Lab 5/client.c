#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "common.h"

int main() {
	int sockfd, result, length;
	pthread_t buffer = 0;
	const char hello[13] = "Hello client!";
	struct sockaddr_in addr_serv;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("Socket failed.");
		return EXIT_FAILURE;
	}

	memset(&addr_serv, 0, sizeof(addr_serv));

	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(PORT);
	addr_serv.sin_addr.s_addr = inet_addr(SERVER_IP);

	result = sendto(sockfd, (const char *)hello, sizeof(hello), 0,
		   (const struct sockaddr *) &addr_serv, sizeof(addr_serv));
	if (result < 0) {
		perror("Cannot send datagram.");
		return EXIT_FAILURE;
	} else {
		printf("Sent %d bytes.\n", result);
	}

	length = sizeof(addr_serv);
	result = recvfrom(sockfd, (pthread_t *)&buffer, sizeof(buffer), 0,
					  (struct sockaddr *) &addr_serv, &length);

	printf("<Server>: Thread %u has been assigned to you.\n", buffer);

	close(sockfd);

	return 0;
}
