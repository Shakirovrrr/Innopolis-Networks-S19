int *nodepool;
int nnodes, poolsize;

void add_node(int sockfd) {
	int *iter = nodepool;

	iter[nnodes] = sockfd;
	nnodes++;
	if (nnodes == poolsize - 2) {
		poolsize += 10;
		nodepool = realloc(nodepool, sizeof(int) * poolsize);
		memset(nodepool + nnodes, 0, sizeof(int) * (poolsize - nnodes));
	}
}

void remove_node(int sockfd) {
	close(sockfd);
	int n = -1;
	for (int i = 0; i < poolsize; ++i) {
		if (nodepool[i] == sockfd) {
			n = i;
			break;
		}
	}

	if (n < 0) return;

	nodepool[n] = nodepool[poolsize - 1];
	nodepool[poolsize - 1] = 0;
	nnodes--;

	if (nnodes * 2 < poolsize) {
		poolsize /= 2;
		nodepool = realloc(nodepool, sizeof(int) * poolsize);
	}
}