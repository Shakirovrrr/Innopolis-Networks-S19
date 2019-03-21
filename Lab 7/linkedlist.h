//
// Created by Ruslani on 15-Mar-19.
//

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include <string.h>

typedef struct Node {
	void *val;
	struct Node *next;
	struct Node *prev;
} Node;

typedef struct LinkedList {
	Node *head;
	Node *tail;
	size_t size;
} LinkedList;

LinkedList *newList();

char insertVal(LinkedList *list, size_t ix, void *val);

char deleteVal(LinkedList *list, size_t ix);

void *getVal(LinkedList *list, size_t ix);

size_t findVal(LinkedList *list, void *val, size_t valsize);

#endif //LINKEDLIST_H
