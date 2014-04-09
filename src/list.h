/*
 * list.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jrm
 *
 *  Linked list implementation
 *
 *  Taken from and slightly modified from:
 *  http://c.learncodethehardway.org/book/ex32.html
 */

#ifndef list_H_
#define list_H_

#include <stdlib.h>

struct ListNode;

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
    void *value;
} ListNode;

typedef struct List {
    int count;
    ListNode *first;
    ListNode *last;
} List;

List *new_list();
void list_destroy(List *list);
void list_clear(List *list);
void list_clear_destroy(List *list);

/**
 * Returns the number of elements in the list, which is maintained as elements are added and removed.
 */
#define list_count(A) ((A)->count)

/**
 * Returns the first element of the list, but does not remove it.
 */
#define list_first(A) ((A)->first != NULL ? (A)->first->value : NULL)

/**
 * Returns the last element of the list, but does not remove it.
 */
#define list_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void list_push(List *list, void *value);
void *list_pop(List *list);
void list_unshift(List *list, void *value);
void *list_shift(List *list);
void *list_remove(List *list, ListNode *node);

/**
 * Iterates over the elements in the list.
 */
#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)

#endif /* list_H_ */
