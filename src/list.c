/*
 * list.c
 *
 *  Created on: Mar 29, 2014
 *      Author: jrm
 *
 *  Taken from and slightly modified from:
 *  http://c.learncodethehardway.org/book/ex34.html
 */
#include "dbg.h"
#include "list.h"

/**
 * Simply creates the main List struct.
 */
List *new_list()
{
    return calloc(1, sizeof(List));
}

/**
 * Destroys a List and any elements it might have.
 */
void list_destroy(List *list)
{
    LIST_FOREACH(list, first, next, cur) {
        if(cur->prev) {
            free(cur->prev);
        }
    }

    free(list->last);
    free(list);
}

/**
 * Convenience function for freeing the values in each node, not the nodes.
 */
void list_clear(List *list)
{
    LIST_FOREACH(list, first, next, cur) {
        free(cur->value);
    }
}

/**
 * Clears and destroys a list. It's not very efficient since it loops through them twice.
 */
void list_clear_destroy(List *list)
{
    list_clear(list);
    list_destroy(list);
}

/**
 * The first operation that demonstrates the advantage of a linked list.
 * It adds a new element to the end of the list, and because that's
 * just a couple of pointer assignments, does it very fast.
 */
void list_push(List *list, void *value)
{
    ListNode *node = calloc(1, sizeof(ListNode));
    check_mem(node);

    node->value = value;

    if(list->last == NULL) {
        list->first = node;
        list->last = node;
    } else {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

    list->count++;

error:
    return;
}

/**
 * The inverse of List_push, this takes the last element off and returns it.
 */
void *list_pop(List *list)
{
    ListNode *node = list->last;
    return node != NULL ? list_remove(list, node) : NULL;
}

/**
 * The other thing you can easily do to a linked list is add elements to the front of the list very fast.
 * In this case I call that List_unshift for lack of a better term.
 */
void list_unshift(List *list, void *value)
{
    ListNode *node = calloc(1, sizeof(ListNode));
    check_mem(node);

    node->value = value;

    if(list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
    }

    list->count++;

error:
    return;
}

/**
 * Just like List_pop, this removes the first element and returns it.
 */
void *list_shift(List *list)
{
    ListNode *node = list->first;
    return node != NULL ? list_remove(list, node) : NULL;
}


/**
 * This is actually doing all of the removal when you do List_pop or List_shift.
 * Something that seems to always be difficult in data structures is removing things,
 * and this function is no different. It has to handle quite a few conditions depending
 * on if the element being removed is at the front; the end; both front and end; or middle.
 */
void *list_remove(List *list, ListNode *node)
{
    void *result = NULL;

    check(list->first && list->last, "List is empty.");
    check(node, "node can't be NULL");

    if(node == list->first && node == list->last) {
        list->first = NULL;
        list->last = NULL;
    } else if(node == list->first) {
        list->first = node->next;
        check(list->first != NULL, "Invalid list, somehow got a first that is NULL.");
        list->first->prev = NULL;
    } else if (node == list->last) {
        list->last = node->prev;
        check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
        list->last->next = NULL;
    } else {
        ListNode *after = node->next;
        ListNode *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    list->count--;
    result = node->value;
    free(node);

error:
    return result;
}
