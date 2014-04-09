/*
 * array.c
 *
 *  Created on: Mar 29, 2014
 *      Author: jrm
 *
 *  Taken from and slightly modified from:
 *  http://c.learncodethehardway.org/book/ex34.html
 */
#include "dbg.h"
#include "array.h"

Array *new_array(size_t element_size, size_t initial_max) {
    Array *array = malloc(sizeof(Array));
    check_mem(array);
    array->max = initial_max;
    check(array->max > 0, "ValueError: You must set an initial_max > 0.");

    array->contents = calloc(initial_max, sizeof(void *));
    check_mem(array->contents);

    array->end = 0;
    array->element_size = element_size;
    array->expand_rate = DEFAULT_EXPAND_RATE;

    return array;

	error:
		if(array) free(array);
		return NULL;
}

void array_clear(Array *array) {
    int i = 0;
    if(array->element_size > 0) {
        for(i = 0; i < array->max; i++) {
            if(array->contents[i] != NULL) {
                free(array->contents[i]);
            }
        }
    }
}

/**
 * Returns the index of the item in the array
 * or -1 if the item doesn't exist.
 */
int array_index_of(Array *array, void *el) {
    int i = 0;
    if(array->element_size > 0) {
        for(i = 0; i < array->max; i++) {
            if(array->contents[i]==el) {
            	return i;
            }
        }
    }
    return -1;
}

static inline int array_resize(Array *array, size_t newsize) {
    array->max = newsize;
    check(array->max > 0, "The newsize must be > 0.");

    void *contents = realloc(array->contents, array->max * sizeof(void *));
    // check contents and assume realloc doesn't harm the original on error

    check_mem(contents);

    array->contents = contents;

    return 0;
	error:
		return -1;
}

int array_expand(Array *array) {
    size_t old_max = array->max;
    check(array_resize(array, array->max + array->expand_rate) == 0,
            "Failed to expand array to new size: %d",
            array->max + (int)array->expand_rate);

    memset(array->contents + old_max, 0, array->expand_rate + 1);
    return 0;

	error:
		return -1;
}

int array_contract(Array *array) {
    int new_size = array->end < (int)array->expand_rate ? (int)array->expand_rate : array->end;

    return array_resize(array, new_size + 1);
}


void array_destroy(Array *array) {
    if(array) {
        if(array->contents) free(array->contents);
        free(array);
    }
}

void array_clear_destroy(Array *array) {
    array_clear(array);
    array_destroy(array);
}

int array_push(Array *array, void *el) {
    array->contents[array->end] = el;
    array->end++;

    if(array_end(array) >= array_max(array)) {
        return array_expand(array);
    } else {
        return 0;
    }
}

void *array_pop(Array *array) {
    check(array->end - 1 >= 0, "ValueError: Attempt to pop from empty array.");

    void *el = array_remove(array, array->end - 1);
    array->end--;

    if(array_end(array) > (int)array->expand_rate && array_end(array) % array->expand_rate) {
        array_contract(array);
    }

    return el;
	error:
		return NULL;
}

int array_swap(Array *array, int i,int j) {
	if (i==j) {
		return 0; // Cant' swap same elements
	}

	// push item at i to the end of the array temporarily
	array_push(array,array_get(array,i));

	// Copy item at j to item at i
	array_set(array,i,array_get(array,j));

	// Pull temp item at end and put it at j
	array_set(array,j,array_pop(array));
	return 0;
	error:
		return -1;
}
