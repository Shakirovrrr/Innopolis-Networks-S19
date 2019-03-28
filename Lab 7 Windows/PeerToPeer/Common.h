#pragma once

#ifndef COMMON_H
#define COMMON_H

#include "Networking.h"

#define NODE_FILE_REQUEST 0
#define NODE_SYNC 1

#define FILENAME_LEN 24
#define NODENAME_LEN 32
#define WORD_LEN 32
#define SENDBUF_LEN 512
#define MAX_DIRLEN 32
#define PEERSTR_LEN (NODENAME_LEN + INET_ADDRSTRLEN + 8)

typedef struct NetworkNode {
	char name[NODENAME_LEN];
	char ipAddr[INET_ADDRSTRLEN];
	int port;
} NetworkNode;

typedef struct NodeFile {
	char name[FILENAME_LEN];
	NetworkNode node;
} NodeFile;

struct FileRequest {
	int sockfd;
	char fileName[FILENAME_LEN];
};

#endif // COMMON_H
