#include "networking.h"

int init_tcp_connection() {
	int socket_listen = 0, socket_talk = 0;
	socket_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (socket_listen < 0) {
		perror("Responder socket creation failed.\n");
		return -1;
	}

	struct sockaddr_in addr_resp, addr_client;

	addr_resp.sin_family = AF_INET;
	addr_resp.sin_port = SERVER_PORT;
	addr_resp.sin_addr.s_addr = INADDR_ANY;

	int result = 0;

	result = bind(socket_listen, (struct sockaddr *) &addr_resp, sizeof(struct sockaddr));
	if (result < 0) {
		perror("Socket binding failed.\n");
		return -1;
	}

	result = listen(socket_listen, 8);
	if (result < 0) {
		perror("Listening error.\n");
		return -1;
	}

	socklen_t addr_len = sizeof(struct sockaddr);
	socket_talk = accept(socket_listen, (struct sockaddr *) &addr_resp, &addr_len);
	if (socket_talk < 0) {
		perror("Accept error.\n");
		return -1;
	}

	return socket_talk;
}

int recvsend(int socket_talk, void *readbuf, size_t readlen, void *sendbuf, size_t sendlen) {
	if (socket_talk < 0) {
		perror("Invalid socket value.\n");
		return -1;
	}

	int result = 0;

	result = read(socket_talk, readbuf, readlen);
	if (result < 0) {
		perror("Read error.\n");
		return -1;
	}

	result = send(socket_talk, sendbuf, sendlen, 0);
	if (result < 0) {
		perror("Send error.\n");
		return -1;
	}

	return 0;
}