#include "pch.h"
#include "Networking.h"

const BOOL USE_HTONS = TRUE;
#ifdef KEEP_BYTE_ORDER
USE_HTONS = FALSE;
#endif // KEEP_BYTE_ORDER

unsigned long htonlO(unsigned long val) {
	if (USE_HTONS) {
		return htonl(val);
	} else {
		return val;
	}
}

unsigned long ntohlO(unsigned long val) {
	if (USE_HTONS) {
		return ntohl(val);
	} else {
		return val;
	}
}

void GetIPAndPort(SOCKET socket, char ipAddr[INET_ADDRSTRLEN], int* port) {
	SOCKADDR_IN addr;
	int len = sizeof(addr);

	ZeroMemory(&addr, sizeof(addr));
	//getpeername(socket, (SOCKADDR*) & addr, &len);
	getsockname(socket, (SOCKADDR*) & addr, &len);

	*port = ntohs(addr.sin_port);
	InetNtopA(AF_INET, &addr.sin_addr, ipAddr, sizeof(ipAddr));
}

void GetMyIPAndPort(char* ipAddr, int* port) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_port = htons(0);

	bind(sockfd, (SOCKADDR*) & addr, sizeof(addr));

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

	bind(sockfd, (SOCKADDR*) & addr, sizeof(addr));

	return sockfd;
}

SOCKET InitTCPClient(char* ipAddr, int port) {
	SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sockfd == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	SOCKADDR_IN addr;
	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	int result = 0;
	result = InetPtonA(AF_INET, ipAddr, &addr.sin_addr);
	if (result < 0) {
		return -1;
	}

	result = connect(sockfd, (SOCKADDR*) & addr, sizeof(addr));
	if (result < 0) {
		return -1;
	}

	return sockfd;
}