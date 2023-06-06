#include <stdint.h>

#ifndef ARMV8_32_DYNARRAY_H
#define ARMV8_32_DYNARRAY_H

#define DYNARRAY_LIMIT 16

struct dynarray {
    uint8_t *data;
    int cap;
    int size;
};

typedef struct dynarray *dynarray;

dynarray dynarray_create(int limit);
void dynarray_free(dynarray arr);
void dynarray_write(dynarray arr, int pos, uint8_t element);
void dynarray_push(dynarray arr, uint8_t element);
uint8_t dynarray_pop(dynarray arr);
void dynarray_print(dynarray arr);

#endif //ARMV8_32_DYNARRAY_H
