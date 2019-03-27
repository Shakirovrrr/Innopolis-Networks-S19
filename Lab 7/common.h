//
// Created by Ruslani on 27-Mar-19.
//

#ifndef LAB_7_COMMON_H
#define LAB_7_COMMON_H

#define NODE_FILE_REQUEST 0
#define NODE_SYNC 1

#define FILENAME_LEN 24
#define NODENAME_LEN 32
#define WORD_LEN 32
#define SENDBUF_LEN 512

typedef struct NetworkNode {
	char name[NODENAME_LEN];
	char ipaddr[INET_ADDRSTRLEN];
	int port;
} NetworkNode;

typedef struct NodeFile {
	char name[FILENAME_LEN];
	NetworkNode node;
} NodeFile;

struct FileRequest {
	int sockfd;
	char filename[FILENAME_LEN];
};

#endif //LAB_7_COMMON_H
