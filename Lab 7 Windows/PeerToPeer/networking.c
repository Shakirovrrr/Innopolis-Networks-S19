#include "networking.h"

void GetIPAndPort(SOCKET socket, char *ipAddr, int *port) {
	SOCKADDR_IN addr;
	int len;

	ZeroMemory(&addr, sizeof(addr));
	getpeername(socket, (SOCKADDR *) &addr, &len);

	*port = ntohs(addr.sin_port);
	InetNtopA(AF_INET, &addr.sin_addr, ipAddr, sizeof(char) * INET_ADDRSTRLEN);
}

void GetMyIPAndPort(char *ipAddr, int *port) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_port = htons(0);

	bind(sockfd, &addr, sizeof(addr));

	GetIPAndPort(sockfd, ipAddr, port);

	closesocket(sockfd);
}

SOCKET InitTCPServer(int port) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	bind(sockfd, &addr, sizeof(addr));

	return sockfd;
}

SOCKET InitTCPClient(char *ipAddr, int port) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;

	int result = 0;
	result = InetPtonA(AF_INET, ipAddr, &addr.sin_addr);
	if (result < 0) {
		return -1;
	}

	result = bind(sockfd, &addr, sizeof(addr));
	if (result < 0) {
		return -1;
	}

	return sockfd;
}