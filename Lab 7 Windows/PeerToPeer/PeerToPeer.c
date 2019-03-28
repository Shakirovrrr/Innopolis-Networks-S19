// PeerToPeer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Common.h"

char nodeName[NODENAME_LEN];
int mainPort;
HANDLE hListenerThread, hSyncerThread;
char sharedDir[MAX_DIRLEN];
char running;

LinkedList nodePool;
LinkedList filePool;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	switch (fdwCtrlType) {
	case CTRL_C_EVENT:
		running = 0;
		ExitThread(0);
		return TRUE;

	default:
		return FALSE;
	}
}

void FormNodeMessage(char *message, size_t msgCount, NetworkNode node) {
	sprintf_s(message, msgCount, "%s:%s:%d", node.name, node.ipAddr, node.port);
}

int FormFileStr(char *str, size_t strCount) {
	WIN32_FIND_DATAA data;
	HANDLE hFind = FindFirstFileA(sharedDir, &data);

	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int offset = sprintf_s(str, strCount, "[]");
	offset--;

	do {
		offset += sprintf_s(str + offset, strCount, "%s,", data.cFileName);
	} while (FindNextFileA(hFind, &data));

	offset += sprintf_s(str + offset - 1, strCount, "]");

	return offset;
}

void DownloadFile(NodeFile nodeFile) {
	long result = 0;

	SOCKET sockTalk = InitTCPClient(nodeFile.node.ipAddr, nodeFile.node.port);
	if (sockTalk == INVALID_SOCKET) {
		perror("Cannot download file (socket creation failed).\n");
		return;
	}

	int request = NODE_FILE_REQUEST;
	result = send(sockTalk, &request, sizeof(request), 0);
	if (result < 0) {
		perror("Cannot download file (unable to send request).\n");
		return;
	}

	int wordCount = 0;
	result = recv(sockTalk, &wordCount, sizeof(wordCount), 0);
	if (result < 0) {
		perror("Cannot download file (unable receive word count).\n");
		return;
	}
	if (wordCount < 0) {
		perror("Cannot download file (there\'s no file).\n");
		return;
	}

	char filePath[FILENAME_LEN * 2];
	filePath[0] = '\0';
	strcat_s(filePath, sizeof(filePath), sharedDir);
	strcat_s(filePath, sizeof(filePath), nodeFile.name);
	FILE *file = NULL;
	fopen_s(&file, filePath, "w");
	char wordBuf[WORD_LEN];
	for (size_t i = 0; i < wordCount; i++) {
		result = recv(sockTalk, wordBuf, sizeof(wordBuf), 0);
		fprintf_s(file, "%s ", wordBuf);
	}

	closesocket(sockTalk);
}

DWORD WINAPI FIleRequestHandler(LPVOID lpParam) {
	struct FileRequest request = *((struct FileRequest *)lpParam);
	SOCKET sockTalk = request.sockfd;
	long result = 0;

	char filePath[FILENAME_LEN * 2];
	filePath[0] = '\0';
	strcat_s(filePath, sizeof(filePath), sharedDir);
	strcat_s(filePath, sizeof(filePath), request.fileName);
	FILE *file = NULL;
	fopen_s(&file, filePath, "r");
	if (!file) {
		int errMsg = -1;
		result = send(sockTalk, &errMsg, sizeof(errMsg), 0);
		closesocket(sockTalk);
		ExitThread(-1);
	}

	unsigned int wordCount = 0;
	char wordBuf[WORD_LEN];
	while ((result = fscanf_s(file, "%s", wordBuf, WORD_LEN)) > 0) wordCount++;
	rewind(file);
	result = send(sockTalk, &wordCount, sizeof(wordCount), 0);
	for (size_t i = 0; i < wordCount; i++) {
		fscanf_s(file, "%s", wordBuf, WORD_LEN);
		result = send(sockTalk, wordBuf, sizeof(wordBuf), 0);
	}
	closesocket(sockTalk);

	return 0;
}

DWORD WINAPI SyncHandler(LPVOID lpParam) {
	SOCKET sockTalk = *((SOCKET *) lpParam);
	long result = 0;

	int nodeSignal = NODE_SYNC;

	result = send(sockTalk, &nodeSignal, sizeof(nodeSignal), 0);

	if (result < 0) {
		perror("Can't send signal.\n");
		ExitThread(-1);
	}

	char sendBuf[SENDBUF_LEN];
	char ipStr[INET_ADDRSTRLEN];
	int portVal = 0;
	unsigned int offset = 0;
	GetMyIPAndPort(ipStr, &portVal);
	offset += sprintf_s(sendBuf, sizeof(sendBuf), "%s:%s:%d:", nodeName, ipStr, portVal);

	char fileName[FILENAME_LEN];
	int fileStrSize = FormFileStr(fileName, sizeof(fileName));
	strcat_s(sendBuf, sizeof(sendBuf), fileName);

	result = send(sockTalk, sendBuf, sizeof(char) * (offset + fileStrSize + 1), 0);

	if (result < 0) {
		perror("Can't send file list.\n");
		ExitThread(-1);
	}

	int nKnownNodes = (int) nodePool->size;
	result = send(sockTalk, &nKnownNodes, sizeof(nKnownNodes), 0);
	if (result < 0) {
		perror("Can't send number of known nodes.\n");
		ExitThread(-1);
	}

	NetworkNode knownNode;
	char nodeAddr[PEERSTR_LEN];
	for (size_t i = 0; i < nKnownNodes; i++) {
		knownNode = *((NetworkNode *) getVal(nodePool, i));
		FormNodeMessage(nodeAddr, sizeof(nodeAddr), knownNode);

		result = send(sockTalk, nodeAddr, sizeof(nodeAddr), 0);

		if (result < 0) {
			perror("Can't send node address.\n");
			ExitThread(-1);
		}
	}


	return 0;
}

void ParseNode(char nodeData[SENDBUF_LEN]) {
	char delim[] = ":";
	char *context = NULL;
	char *tok = strtok_s(nodeData, delim, &context);
	NetworkNode *node = HeapAlloc(GetProcessHeap(), 0, sizeof(NetworkNode));
	sprintf_s(node->name, sizeof(node->name), "%s", tok);

	tok = strtok_s(NULL, delim, &context);
	char portVal[6];
	sprintf_s(portVal, sizeof(portVal), "%s", tok);
	node->port = (int) strtoul(portVal, NULL, 0);
	tok = strtok_s(NULL, delim, &context);

	insertVal(nodePool, 0, node);

	size_t fileListSize = strlen(tok);
	if (fileListSize > 2) {
		char fileList[SENDBUF_LEN];
		CopyMemory(fileList, tok + 1, sizeof(char) * (fileListSize - 2));
		fileList[fileListSize - 1] = '\0';

		char fileDelim[] = ",";
		tok = strtok_s(fileList, fileDelim, &context);
		while (tok) {
			NodeFile *file = HeapAlloc(GetProcessHeap(), 0, sizeof(NodeFile));
			strcpy_s(fileList, sizeof(fileList), tok);
			file->node = *node;

			if (findVal(filePool, file, sizeof(NodeFile)) < 0) {
				insertVal(filePool, 0, file);
			}

			tok = strtok_s(NULL, fileDelim, &context);
		}
	}
}

void RemoveInactiveNode(NetworkNode node) {
	long long ix = findVal(nodePool, &node, sizeof(NetworkNode));
	if (ix >= 0) {
		deleteVal(nodePool, ix);
	}
}

void ParsePeer(char peerData[PEERSTR_LEN]) {
	char delim[] = ":";
	char *context = NULL;
	char *tok = strtok_s(peerData, delim, &context);
	NetworkNode *node = HeapAlloc(GetProcessHeap(), 0, sizeof(NetworkNode));
	sprintf_s(node->name, sizeof(node->name), "%s", tok);

	tok = strtok_s(NULL, delim, &context);
	sprintf_s(node->ipAddr, sizeof(node->ipAddr), "%s", tok);

	tok = strtok_s(NULL, delim, &context);
	node->port = (int) strtoul(tok, NULL, 0);

	if (findVal(nodePool, node, sizeof(NetworkNode)) < 0) {
		insertVal(nodePool, 0, node);
	}
}

void SendSync(int sockSync, NetworkNode node) {
	long int result = 0;
	int message = NODE_SYNC;
	result = send(sockSync, &message, sizeof(message), 0);
	if (result < 0) {
		perror("Cannot send message.\n");
		RemoveInactiveNode(node);
		return;
	}

	message = -1;
	result = recv(sockSync, &message, sizeof(message), 0);
	if (result < 0 || message < 0) {
		perror("Cannot get sync message.\n");
		RemoveInactiveNode(node);
		return;
	}

	char buffer[SENDBUF_LEN];
	result = recv(sockSync, buffer, sizeof(buffer), 0);
	if (result < 0) {
		perror("Cannot receive node data.\n");
		RemoveInactiveNode(node);
		return;
	}
	ParseNode(buffer);

	message = 0;
	result = recv(sockSync, &message, sizeof(message), 0);
	if (result < 0) {
		perror("Cannot receive number of known nodes.\n");
		return;
	}

	char peerBuf[PEERSTR_LEN];
	for (int i = 0; i < message; ++i) {
		result = recv(sockSync, peerBuf, sizeof(peerBuf), 0);
		ParsePeer(peerBuf);
	}
}

DWORD WINAPI Syncer() {
	SetConsoleCtrlHandler(CtrlHandler, TRUE);

	int sockSync = 0;
	NetworkNode node;
	while (running) {
		for (size_t i = 0; i < nodePool->size; i++) {
			node = *((NetworkNode *) getVal(nodePool, i));
			sockSync = InitTCPClient(node.ipAddr, node.port);
			SendSync(sockSync, node);
			closesocket(sockSync);
			Sleep(750);
		}
	}
}

DWORD WINAPI IncomingHandler() {
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
	running = 1;

	CreateThread(NULL, 0, &Syncer, NULL, 0, NULL);

	int inSock = InitTCPServer(mainPort);

	char addrStr[INET_ADDRSTRLEN];
	int portVal;
	GetIPAndPort(inSock, addrStr, &portVal);
	printf_s("Running on %s:%d\n", addrStr, portVal);

	SOCKADDR_STORAGE theirAddr;
	socklen_t addrLen;
	SOCKET sockTalk = 0;
	int requestBuf = -1;
	long result = 0;
	int *handlerSock = HeapAlloc(GetProcessHeap(), 0, sizeof(int));

	while (running) {
		sockTalk = accept(inSock, (SOCKADDR *) &theirAddr, &addrLen);
		if (sockTalk < 0) {
			continue;
		}

		result = recv(sockTalk, &requestBuf, sizeof(requestBuf), 0);
		if (result < 0) {
			perror("Did not receive the request.\n");
			continue;
		}

		*handlerSock = sockTalk;
		switch (requestBuf) {
		case NODE_SYNC:
			printf_s("Received sync request...\n");
			CreateThread(NULL, 0, &SyncHandler, handlerSock, 0, NULL);
			break;

		case NODE_FILE_REQUEST:
			printf_s("Received file request...\n");
			char fileName[FILENAME_LEN];
			result = recv(sockTalk, fileName, sizeof(fileName), 0);
			if (result < 0) {
				perror("Cannot receive file name.\n");
				continue;
			}

			struct FileRequest fileRequest;
			strcpy_s(fileRequest.fileName, sizeof(fileRequest.fileName), fileName);
			fileRequest.sockfd = sockTalk;
			CreateThread(NULL, 0, &FIleRequestHandler, &fileRequest, 0, NULL);

			break;

		default:
			printf_s("Received unknown request.\n");
			break;
		}
	}

	HeapFree(GetProcessHeap(), 0, handlerSock);
	closesocket(inSock);
	closesocket(sockTalk);

	return 0;
}

int main() {
	return 0;
}