#include <Windows.h>
#include <stdio.h>

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

	StackNode *newNode = (StackNode*) malloc(sizeof(StackNode));
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

char stackPushInt(Stack *stack, int val) {
	int *data = malloc(sizeof(int));
	*data = val;

	return stackPush(stack, data);
}

int stackPickInt(Stack *stack) {
	int *picked = (int*) stackPick(stack);
	return *picked;
}

int main() {
	Stack stack = emptyStack();
	stackPushInt(&stack, 124);

	printf("%d", stackPickInt(&stack));

	return 0;
}