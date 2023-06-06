#include "dynarray.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

dynarray dynarray_create(int limit) {
    dynarray new = malloc(sizeof(struct dynarray));
    assert (new != NULL);
    new->cap = limit;
    new->data = malloc(limit * sizeof(uint8_t));
    assert (new->data != NULL);
    new->size = 0;
    return new;
}

// pre: when rebuild is being called, the size of arr >= the cap of arr
static void dynarray_rebuild(dynarray arr) {
    arr->cap += DYNARRAY_LIMIT;
    arr->data = realloc(arr->data, arr->cap * sizeof(uint8_t));
    assert (arr->data != NULL);
}

// frees all memory allocated to create the dynarray
void dynarray_free(dynarray arr) {
    free(arr->data);
    free(arr);
}

// overwrites whatever was stored at pos
void dynarray_write(dynarray arr, int pos, uint8_t element) {
    assert (pos < arr->size);
    arr->data[pos] = element;
}

// adds an element to the end of a given dynarray
void dynarray_push(dynarray arr, uint8_t element) {
    if (arr->size >= arr->cap) {
        dynarray_rebuild(arr);
    }
    arr->data[arr->size++] = element;
}

// logically removes an element from the end of a given dynarray
uint8_t dynarray_pop(dynarray arr) {
    uint8_t tail = arr->data[arr->size-1];
    arr->size--;
    return tail;
}

// prints the content of arr
void dynarray_print(dynarray arr) {
    printf("Array contents: ");
    for (int i = 0; i < arr->size; i++) {
        printf("%i", arr->data[i]);

        if (i != arr->size - 1) {
            printf(" ");
        }
    }
    printf("\n");
}
