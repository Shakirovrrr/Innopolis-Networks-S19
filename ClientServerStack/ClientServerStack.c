#include <Windows.h>
#include <stdio.h>

const LPCWSTR PIPENAME = L"\\\\.\\pipe\\StackPipe351";

#define SERVERHELP -2
#define SERVEREXIT -1
#define STACKPICK 0
#define STACKPOP 1
#define STACKPUSH 2
#define STACKNEW 3
#define STACKSIZE 4
#define STACKISEMPTY 5
#define STACKDISPLAY 6

#pragma region Stack

typedef struct StackNode {
	struct StackNode *prev;
	void *data;
} StackNode;

typedef struct Stack {
	struct StackNode *bottom;
	struct StackNode *top;
	size_t size;
} Stack;

Stack emptyStack() {
	Stack empty;
	empty.bottom = NULL;
	empty.top = NULL;
	empty.size = 0;

	return empty;
}

char stackPush(Stack *stack, void *data) {
	if (!data) {
		return -1;
	}

	StackNode *newNode = (StackNode*) HeapAlloc(GetProcessHeap(), 0, sizeof(StackNode));
	if (!newNode) {
		return -1;
	}

	newNode->data = data;

	if (stack->size == 0) {
		newNode->prev = NULL;
		stack->bottom = newNode;
		stack->top = newNode;
		stack->size = 1;

		return 0;
	}

	newNode->prev = stack->top;
	stack->top = newNode;
	stack->size++;

	return 0;
}

void *stackPick(Stack *stack) {
	if (stack->top) {
		return stack->top->data;
	}
	return NULL;
}

void *stackPop(Stack *stack) {
	if (stack->top == NULL) {
		return NULL;
	}

	void *picked = stackPick(stack);

	StackNode *old = stack->top;
	stack->top = stack->top->prev;
	HeapFree(GetProcessHeap(), 0, old);
	stack->size--;

	return picked;
}

void stackClear(Stack *stack) {
	while (stack->size > 0) {
		stackPop(stack);
	}
}

char stackPushInt(Stack *stack, int val) {
	int *data = (int*) HeapAlloc(GetProcessHeap(), 0, sizeof(int));
	if (!data) {
		return -1;
	}

	*data = val;

	return stackPush(stack, data);
}

int stackPickInt(Stack *stack) {
	int *picked = (int*) stackPick(stack);
	if (!picked) {
		return -1;
	}

	return *picked;
}

int stackPopint(Stack *stack) {
	int *picked = (int*) stackPop(stack);
	if (!picked) {
		return -1;
	}

	return *picked;
}

#pragma endregion

void printHelp() {
	printf("\t\nList of available commands:\n");
	printf("\t\'pick\' -> Pick the top element.\n");
	printf("\t\'pop\' -> Pop the top element.\n");
	printf("\t\'push\' -> Push the element to the stack. You will be asked for the value.\n");
	printf("\t\'new\' -> Create new stack.\n");
	printf("\t\'size\' -> Get the stack size.\n");
	printf("\t\'isempty\' -> Check for emptiness.\n");
	printf("\t\'display\' -> Print entire stack.\n");
	printf("\t\'exit\' -> Close session.\n\n");
}

void displaySatckOfInts(Stack *stack) {
	StackNode *current = stack->top;
	printf("\nSTACK TOP\n____________\n");
	while (current) {
		printf("%d\n", *(int*) current->data);
		current = current->prev;
	}
	printf("____________\nSTACK BOTTOM\n\n");
}

DWORD WINAPI ServerJob(LPVOID lpParam) {
	HANDLE hPipe = CreateNamedPipe(
		PIPENAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_READMODE_BYTE,
		1, 0, 0, 0, NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE || hPipe == NULL) {
		printf("<Server>: Failed to create pipe.\n");
		ExitThread(-1);
	}

	BOOL connectionResult = ConnectNamedPipe(hPipe, NULL);
	if (!connectionResult) {
		printf("<Server>: Failed to connect to pipe.\n");
		CloseHandle(hPipe);
		ExitThread(-1);
	}

	printf("<Server>: Waiting for client...\n");

	char respond = 0;
	DWORD bytesRead = 0;
	BOOL readResult = ReadFile(hPipe, &respond, sizeof(char), &bytesRead, NULL);
	if (!readResult || respond != 1) {
		printf("<Server>: Failed to connect to client.\n");
		CloseHandle(hPipe);
		ExitThread(-1);
	}

	Stack *stack = (Stack*) HeapAlloc(GetProcessHeap(), 0, sizeof(Stack));
	if (!stack) {
		printf("<Server>: Failed to allocate memory for stack.\n");
		CloseHandle(hPipe);
		ExitThread(-1);
	}
	*stack = emptyStack();

	int requestBuffer[2];
	bytesRead = 0;
	readResult = FALSE;
	char stackOpResult = 0;

	printf("<Server>: Ready.\n");

	while (TRUE) {
		readResult = ReadFile(hPipe, requestBuffer, sizeof(int) * 2, &bytesRead, NULL);

		if (readResult) {
			switch (requestBuffer[0]) {
			case STACKPICK:
				printf("<Server>: Pick -> %d\n", stackPickInt(stack));
				break;

			case STACKPOP:
				printf("<Server>: Pop -> %d\n", stackPopint(stack));
				break;

			case STACKPUSH:
				stackOpResult = stackPushInt(stack, requestBuffer[1]);
				if (stackOpResult == -1) {
					printf("<Server>: -> Failed to push.\n");
				} else {
					printf("<Server>: -> Pushed %d to stack.\n", stackPickInt(stack));
				}
				break;

			case STACKNEW:
				stackClear(stack);
				*stack = emptyStack();
				printf("<Server>: -> Created new stack.\n");
				break;

			case STACKSIZE:
				printf("<Server>: Stack size -> %llu\n", stack->size);
				break;

			case STACKISEMPTY:
				printf("<Server>: Stack is %s.\n", (stack->size > 0 ? "not empty" : "empty"));
				break;

			case STACKDISPLAY:
				printf("<Server>:\n");
				displaySatckOfInts(stack);
				break;

			case SERVERHELP:
				printf("<Server>:");
				printHelp();
				break;

			case SERVEREXIT:
				printf("<Server>: Server stopped.\n");
				CloseHandle(hPipe);
				ExitThread(0);
			}

			respond = 1;
			DWORD bytesWritten = 0;
			BOOL writeResult = WriteFile(hPipe, &respond, sizeof(char), &bytesWritten, NULL);
			if (!writeResult) {
				printf("<Server>: Failed to send respont to client.\n");
				CloseHandle(hPipe);
				ExitThread(-1);
			}

		} else {
			printf("<Server>: Failed to read from pipe.\n");
			CloseHandle(hPipe);
			ExitThread(-1);
		}
	}
}

void ClientJob(LPVOID lpParam) {
	HANDLE hPipe = CreateFile(PIPENAME, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPipe == INVALID_HANDLE_VALUE || hPipe == NULL) {
		printf("<Client>: Failed to connect to pipe.\n");
		return;
	}

	char respond = 1;
	DWORD bytesWritten = 0;
	BOOL writeResult = WriteFile(hPipe, &respond, sizeof(char), &bytesWritten, NULL);
	if (!writeResult) {
		printf("<Client>: Failed to connect to the server.\n");
		CloseHandle(hPipe);
		return;
	}

	printf("<Client>: Connected to the server.\n");

	int requestBuffer[2] = {0,0};
	char inputBuffer[64];
	respond = 0;
	writeResult = FALSE;
	bytesWritten = 0;
	DWORD bytesRead = 0;
	while (1) {
		printf(" > ");
		scanf_s("%s", inputBuffer, (unsigned int) sizeof(char) * 64);
		printf("\n");
		inputBuffer[63] = '\0';

		if (lstrcmpiA(inputBuffer, "PICK") == 0) {
			requestBuffer[0] = STACKPICK;
		} else if (lstrcmpiA(inputBuffer, "POP") == 0) {
			requestBuffer[0] = STACKPOP;
		} else if (lstrcmpiA(inputBuffer, "PUSH") == 0) {
			requestBuffer[0] = STACKPUSH;
			printf("Value to push? > ");
			scanf_s("%d", &requestBuffer[1]);
		} else if (lstrcmpiA(inputBuffer, "NEW") == 0) {
			requestBuffer[0] = STACKNEW;
		} else if (lstrcmpiA(inputBuffer, "SIZE") == 0) {
			requestBuffer[0] = STACKSIZE;
		} else if (lstrcmpiA(inputBuffer, "ISEMPTY") == 0) {
			requestBuffer[0] = STACKISEMPTY;
		} else if (lstrcmpiA(inputBuffer, "DISPLAY") == 0) {
			requestBuffer[0] = STACKDISPLAY;
		} else if (lstrcmpiA(inputBuffer, "HELP") == 0) {
			requestBuffer[0] = SERVERHELP;
		} else if (lstrcmpiA(inputBuffer, "EXIT") == 0) {
			requestBuffer[0] = SERVEREXIT;
			CloseHandle(hPipe);
			return;
		} else {
			printf("<Client>: Unknown command.\n");
			continue;
		}

		writeResult = WriteFile(hPipe, requestBuffer, sizeof(int) * 2, &bytesWritten, NULL);
		if (!writeResult) {
			printf("<Client>: Failed to write to pipe.\n");
			return;
		}

		BOOL readResult = ReadFile(hPipe, &respond, sizeof(char), &bytesRead, NULL);
		if (!readResult) {
			printf("<Client>: Cannot get respond from server.\n");
			CloseHandle(hPipe);
			return;
		}
		if (respond != 1) {
			printf("<Client>: Received wrong respond from server.\n");
			CloseHandle(hPipe);
			return;
		}
	}
}

int main() {
	printf("Client-server stack.\nRuslan Shakirov, B17-2, Innopolis University, 2019\n");
	printf("Type \'help\' for the list of commands.\n\n");

	CreateThread(NULL, 0, &ServerJob, NULL, 0, NULL);
	Sleep(1000);
	ClientJob(NULL);

	return 0;
}