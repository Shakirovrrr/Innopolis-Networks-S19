//
// Created by Ruslani on 15-Mar-19.
//

#include "pch.h"
#include "LinkedList.h"

Node *__newNode(void *val) {
	Node *new = (Node *) malloc(sizeof(struct Node));
	if (!new) return NULL;

	new->val = val;
	new->prev = 0;
	new->next = 0;

	return new;
}

LinkedList newLinkedList() {
	LinkedList newList = malloc(sizeof(struct LinkedList));
	if (!newList) return NULL;

	newList->size = 0;
	newList->head = NULL;
	newList->tail = NULL;

	return newList;
}

void __initList(LinkedList list, Node *newNode) {
	newNode->prev = NULL;
	newNode->next = NULL;

	list->head = newNode;
	list->tail = newNode;
	list->size++;
}

Node *__jumpToNode(LinkedList list, size_t ix) {
	Node *current = list->head;

	for (int i = 0; i < ix; ++i) {
		current = current->next;
	}

	return current;
}

void __appendNode(LinkedList list, Node *val) {
	Node *end = list->tail;
	val->prev = end;
	end->next = val;

	list->tail = val;
	list->size++;
}

void __prependNode(LinkedList list, Node *val) {
	Node *begin = list->head;
	val->next = begin;
	begin->prev = val;

	list->head = val;
	list->size++;
}

char insertVal(LinkedList list, size_t ix, void *val) {
	if ((ix > list->size && list->size != 0) || ix < 0) return 1;

	Node *new = __newNode(val);
	if (!new) return 1;

	if (list->size == 0) {
		__initList(list, new);
		return 0;
	}

	if (ix == 0) {
		__prependNode(list, new);
		return 0;
	}

	if (ix == list->size) {
		__appendNode(list, new);
		return 0;
	}

	Node *current = __jumpToNode(list, ix);
	new->prev = current->prev;
	new->next = current;
	current->prev->next = new;
	current->prev = new;
	list->size++;

	return 0;
}

char deleteVal(LinkedList list, size_t ix) {
	if ((ix >= list->size && list->size != 0) || ix < 0) return 1;

	Node *current;
	if (ix == 0) {
		current = list->head;
		current->next->prev = 0;
		list->head = current->next;
	} else if (ix == list->size - 1) {
		current = list->tail;
		current->prev->next = 0;
		list->tail = current->prev;
	} else {
		current = __jumpToNode(list, ix);
		current->prev->next = current->next;
		current->next->prev = current->prev;
	}

	free(current);
	list->size--;

	return 0;
}

void *getVal(LinkedList list, size_t ix) {
	if ((ix >= list->size && list->size != 0) || ix < 0) return NULL;

	Node *current = __jumpToNode(list, ix);

	return current->val;
}

long long findVal(LinkedList list, void *val, size_t valsize) {
	Node *current = list->head;

	for (size_t i = 0; i < list->size; ++i, current = current->next) {
		if (memcmp(val, current->val, valsize) == 0) {
			return i;
		}
	}

	return -1;
}
