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
#define NOP 3573751839; // No operation instruction
#define HALT 2315255808 // Termination instruction

typedef uint32_t (*func_ptr)(char *);

// -------------------------------------------- SPECIAL FUNCTIONS ------------------------------------------------------
uint32_t NOP_function(char *instruction) {
    return NOP;
}

uint32_t directive_function(char *assembly_instruction) {
    // Copies assembly instruction
    unsigned long n = strlen(assembly_instruction);
    char instruction_copy[n + 1];

    for (int i = 0; i <= n; i++) {
        instruction_copy[i] = assembly_instruction[i];
    }

    // Takes the second word
    char *token = strtok(instruction_copy, " ");
    token = strtok(NULL, " ");

    // Converts second word into a long and returns it
    char *endptr;
    return strtol(token, &endptr, 16);
}

// ---------------------------------- Mapping of instruction to function pointer ---------------------------------------
func_ptr get_function(char* assembly_instruction) {
    // Special Functions:
    // No Operation
    if (strcmp(assembly_instruction, "nop") == 0) {
        return &NOP_function;
    }
    // Directive
    if (assembly_instruction[0] == '.') {
        return &directive_function;
    }
    // Instructions:
    // Branches
    if (assembly_instruction[0] == 'b' && assembly_instruction[1] != 'i') {
        return &assemble_branches;
    }
    // Single DTI
    if (assembly_instruction[0] == 'l' || (assembly_instruction[0] == 'l' && assembly_instruction[1] == 'd')) {
        return &assemble_single_DTI;
    }
    // Rest is DPI
    return &assemble_DPI;
}


uint64_t program_counter;
dynmap symbol_table;

void remove_new_line(char* line) {
    int len = strlen(line);
    if (line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

// reads .s assembly file to lines in the form of strings in iteration
void assemble(char *in, char *out) {
    FILE *infile;
    FILE *outfile;
    char line[MAX_LINE_LENGTH];

    // dynarray output = dynarray_create(DYNARRAY_LIMIT);

    infile = fopen(in, "r");
    outfile = fopen(out, "wb");

    if (infile == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // Build the symbol table
    program_counter = 0;
    while (fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
        // Check if the line is a label
        if (isalpha(line[0]) && line[strlen(line) - 1] == ':') { // len - 2 due to '\0'
            // add an entry to the table where key is label and value is address
            line[strlen(line) - 1] = '\0';
            dynmap_add(symbol_table, line, program_counter);
        }
    }

    while (fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
        // Determines which parsing function to be used: could be a NOP function, directive, DPI, Branch or Single DTI
        func_ptr parsing_function = get_function(line);
        uint32_t binary_instruction = parsing_function(line);
        fwrite(&binary_instruction, sizeof(binary_instruction), 1, outfile); // I think it's like that not sure though
    }

    fclose(infile);
    fclose(outfile);
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
