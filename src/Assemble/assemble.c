#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "dynarray.h"
#include "assembleBranches.h"
#include "assembleDPI.h"
#include "assembleSingleDTI.h"

#define MAX_LINE_LENGTH 256

dynarray arr_output;



// reads .s assembly file to lines in the form of strings in iteration
void read_file(char *dst) {
    FILE *input;
    char line[MAX_LINE_LENGTH];

    dynarray output = dynarray_create();

    input = fopen(dst, "r");

    if (input == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
    }

    fclose(input);
}

int main(int argc, char **argv) {
    arr_output = dynarray_create();
    dynarray_push(arr_output, 1);
    dynarray_print(arr_output);
    dynarray_push(arr_output, 2);
    dynarray_print(arr_output);
    for (int i = 3; i <= 21; i++) {
        // testing dynamic property
        dynarray_push(arr_output, i);
    }
    dynarray_print(arr_output);
    printf("%i\n", dynarray_pop(arr_output));
    dynarray_print(arr_output);
    read_file(argv[1]);
    dynarray_free(arr_output);
    return EXIT_SUCCESS;
}
