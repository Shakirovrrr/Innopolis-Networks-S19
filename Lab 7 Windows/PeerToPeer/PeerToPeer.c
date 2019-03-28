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
	WIN32_FIND_DATA data;
	HANDLE hFind = FindFirstFile(sharedDir, &data);

	if (hFind == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int offset = sprintf_s(str, strCount, "[]");
	offset--;

	do {
		offset += sprintf_s(str + offset, "%s,", data.cFileName);
	} while (FindNextFile(hFind, &data));

	offset += sprintf_s(str + offset - 1, strCount, "]");

	return offset;
}

void DownloadFile(NodeFile nodeFile) {
	long int result = 0;

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
	fopen_s(file, filePath, "w");
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
	long int result = 0;

	char filePath[FILENAME_LEN * 2];
	filePath[0] = '\0';
	strcat_s(filePath, sizeof(filePath), sharedDir);
	strcat_s(filePath, sizeof(filePath), request.fileName);
	FILE *file = NULL;
	fopen_s(file, filePath, "r");
	if (!file) {
		int errMsg = -1;
		result = send(sockTalk, &errMsg, sizeof(errMsg), 0);
		closesocket(sockTalk);
		ExitThread(-1);
	}

	unsigned int wordCount = 0;
	char wordBuf[WORD_LEN];
	while ((result = fscanf_s(file, "%s", wordBuf, sizeof(wordBuf))) > 0) wordCount++;
	rewind(file);
	result = send(sockTalk, &wordCount, sizeof(wordCount), 0);
	for (size_t i = 0; i < wordCount; i++) {
		fscanf_s(file, "%s", wordBuf, sizeof(wordBuf));
		result = send(sockTalk, wordBuf, sizeof(wordBuf), 0);
	}
	closesocket(sockTalk);
}



int main() {
	return 0;
}