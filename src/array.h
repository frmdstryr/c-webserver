/*
 * Array.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jrm
 *
 *  A dynamic array implementation
 *
 *  Taken from and slightly modified from:
 *  http://c.learncodethehardway.org/book/ex34.html
 */
#include <stdlib.h>
#include "dbg.h"

#ifndef array_H_
#define array_H_

typedef struct Array {
	int end;
	int max;
	size_t element_size;
	size_t expand_rate;
	void **contents;
} Array;

Array *new_array(size_t element_size, size_t initial_max);
void array_destroy(Array *array);
void array_clear(Array *array);
int array_expand(Array *array);
int array_contract(Array *array);
int array_push(Array *array, void *el);
int array_index_of(Array *array, void *el);
int array_swap(Array *array, int i,int j);
void *array_pop(Array *array);
void array_clear_destroy(Array *array);

#define array_last(A) ((A)->contents[(A)->end - 1])
#define array_first(A) ((A)->contents[0])
#define array_end(A) ((A)->end)
#define array_count(A) array_end(A)
#define array_max(A) ((A)->max)

#define DEFAULT_EXPAND_RATE 300

static inline void array_set(Array *array, int i, void *el) {
	check(i < array->max, "IndexError: %i",i);
	if(i > array->end) array->end = i;
	array->contents[i] = el;
	error:
		return;
}

static inline void *array_get(Array *array, int i) {
	check(i < array->max, "IndexError: %i",i);
	return array->contents[i];
	error:
		return NULL;
}

static inline void *array_remove(Array *array, int i) {
	void *el = array->contents[i];
	array->contents[i] = NULL;
	return el;
}

static inline void *array_new(Array *array) {
	check(array->element_size > 0, "ValueError: Can't use array_new on 0 size Arrays.");
	return calloc(1, array->element_size);
	error:
		return NULL;
}

#define array_free(E) free((E))
#endif /* array_H_ */
