#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define MAX_LINE_LENGTH 256
#define DYNARRAY_LIMIT 16

struct dynarray {
    uint8_t *data;
    int cap;
    int size;
};

typedef struct dynarray *dynarray;

dynarray arr_output;

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

// reads .s assembly file to lines in the form of strings in iteration
void read_file(char *dst) {
    FILE *input;
    char line[MAX_LINE_LENGTH];

    input = fopen(dst, "r");

    if (input == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        printf("%s", line);
    }

    fclose(input);
}

int main(int argc, char **argv) {
    arr_output = dynarray_create(DYNARRAY_LIMIT);
    read_file(argv[1]);
    return EXIT_SUCCESS;
}
