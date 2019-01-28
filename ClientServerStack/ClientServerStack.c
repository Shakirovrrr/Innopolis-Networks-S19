#include <Windows.h>
#include <stdio.h>

const char *PIPENAME = L"\\\\.\\pipe\\StackPipe351";

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
	return stack->top->data;
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

void displaySatckOfInts(Stack *stack) {
	StackNode *current = stack->top;
	printf("\nSTACK TOP\n____________\n");
	while (current) {
		printf("%d\n", *(int*) current->data);
		current = current->prev;
	}
	printf("____________\nSTACK BOTTOM\n\n");
}

void serverJob(LPVOID lpParam) {
	HANDLE hPipe = CreateNamedPipe(
		PIPENAME,
		PIPE_ACCESS_DUPLEX,
		PIPE_READMODE_BYTE,
		1, 0, 0, 0, NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE || hPipe == NULL) {
		printf("Server: Failed to create pipe.\n");
		ExitThread(-1);
	}

	printf("Server: Waiting for client...\n");

	BOOL connectionResult = ConnectNamedPipe(hPipe, NULL);
	if (!connectionResult) {
		printf("Server: Failed to connect to pipe.\n");
		CloseHandle(hPipe);
		ExitThread(-1);
	}

	Stack *stack = (Stack*) HeapAlloc(GetProcessHeap(), 0, sizeof(Stack));
	*stack = emptyStack();

	char requestBuffer[2];
	DWORD bytesRead = 0;
	BOOL readResult = ReadFile(hPipe, requestBuffer, sizeof(char) * 2, &bytesRead, NULL);
	char stackOpResult = 0;
	if (readResult) {
		switch (requestBuffer[0]) {
		case STACKPICK:
			printf("Server: Pick -> %d\n", stackPickInt(stack));
			break;
		case STACKPOP:
			printf("Server: Pop -> %d\n", stackPopint(stack));
			break;
		case STACKPUSH:
			stackOpResult = stackPushInt(stack, requestBuffer[1]);
			if (stackOpResult == -1) {
				printf("Server: Failed to push.\n");
			} else {
				printf("Server: Pushed %d to stack.\n", stackPickInt(stack));
			}
			break;
		case STACKNEW:
			stackClear(stack);
			*stack = emptyStack();
			printf("Server: Created new stack.\n");
			break;
		case STACKSIZE:
			printf("Server: Stack size: %d\n", stack->size);
			break;
		case STACKISEMPTY:
			printf("Server: Stack is %s.", (stack->size > 0 ? "not empty" : "empty"));
			break;
		case STACKDISPLAY:
			printf("Server:\n");
			displaySatckOfInts(stack);
			break;
		}
	}
}

void clientJob(LPVOID lpParam) {

}

int main() {
	Stack *stack = (Stack*) HeapAlloc(GetProcessHeap(), 0, sizeof(Stack));
	*stack = emptyStack();

	stackPushInt(stack, 124);
	stackPushInt(stack, 162);

	printf("%d\n", stackPickInt(stack));
	printf("%d\n", stackPopint(stack));
	printf("%d\n", stackPopint(stack));
	printf("%d\n", stackPopint(stack));

	HeapFree(GetProcessHeap(), 0, stack);

	return 0;
}