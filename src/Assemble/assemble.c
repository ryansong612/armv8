#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "dynmap.h"
#include "assembleBranches.h"
#include "assembleDPI.h"
#include "assembleSingleDTI.h"

#define MAX_LINE_LENGTH 256

typedef int32_t (*assembleDPI)(char* assembly_instruction);
typedef int32_t (*assembleDPI)(char* assembly_instruction);
typedef int32_t (*assembleDPI)(char* assembly_instruction);

uint64_t program_counter;
dynmap symbol_table;

void remove_new_line(char* line) {
    int len = strlen(line);
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

// reads .s assembly file to lines in the form of strings in iteration
void read_file(char *dst) {
    FILE *input;
    char line[MAX_LINE_LENGTH];

    // dynarray output = dynarray_create(DYNARRAY_LIMIT);

    input = fopen(dst, "r");

    if (input == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // Build the symbol table
    program_counter = 0;
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        // Check if the line is a label
        if (isalpha(line[0]) && line[strlen(line) - 1] == ':') { // len - 2 due to '\0'
            // add an entry to the table where key is label and value is address
            line[strlen(line) - 1] = '\0';
            dynmap_add(symbol_table, line, program_counter);
        }
    }

    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        remove_new_line(line);
        //assembleInstruction(line);
    }

    fclose(input);
}

int main(int argc, char **argv) {
//    arr_output = dynarray_create(DYNARRAY_LIMIT);
//    dynarray_push(arr_output, 1);
//    dynarray_print(arr_output);
//    dynarray_push(arr_output, 2);
//    dynarray_print(arr_output);
//    for (int i = 3; i <= 21; i++) {
//        // testing dynamic property
//        dynarray_push(arr_output, i);
//    }
//    dynarray_print(arr_output);
//    printf("%i\n", dynarray_pop(arr_output));
//    dynarray_print(arr_output);
//    read_file(argv[1]);
//    dynarray_free(arr_output);

    return EXIT_SUCCESS;
}
