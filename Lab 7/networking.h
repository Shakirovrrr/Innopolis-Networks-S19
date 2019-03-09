#ifndef NETWORKING_HELPERS
#define NETWORKING_HELPERS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER_PORT 2000

int init_tcp_connection();

int recvsend(int socket_talk, void *readbuf, size_t readlen, void *sendbuf, size_t sendlen);

#endif // NETWORKING_HELPERS
