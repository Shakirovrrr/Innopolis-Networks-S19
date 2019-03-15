//
// Created by Ruslani on 15-Mar-19.
//

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>

typedef struct Node {
	unsigned int val;
	struct Node *next;
	struct Node *prev;
} Node;

typedef struct LinkedList {
	Node *head;
	Node *tail;
	size_t size;
} LinkedList;

LinkedList *newList();

char insertVal(LinkedList *list, size_t ix, unsigned int val);

char deleteVal(LinkedList *list, size_t ix);

unsigned int getVal(LinkedList *list, size_t ix);

size_t findVal(LinkedList *list, unsigned int val);

#endif //LINKEDLIST_H
