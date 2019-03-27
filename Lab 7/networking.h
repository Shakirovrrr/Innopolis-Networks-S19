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

unsigned int get_uint_ip(int sockfd);

void get_ip_port(int sockfd, int *port, char ipaddr[INET_ADDRSTRLEN]);

void get_my_ip(int *port, char *ipaddr);

int convert_address(char *ipaddress, struct sockaddr_in *addr);

int init_tcp_server(in_port_t port);

int setup_communication(int conn_socket);

int recvsend(int socket_talk, void *readbuf, size_t readlen, void *sendbuf, size_t sendlen);

int sendrecv(int conn_socket, void *sendbuf, size_t sendlen, void *readbuf, size_t readlen);

int ping(int conn_socket, int n);

#endif // NETWORKING_HELPERS
