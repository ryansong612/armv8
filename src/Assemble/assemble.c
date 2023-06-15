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
    if (assembly_instruction[0] == 'l' || assembly_instruction[0] == 's') {
        return &assemble_single_DTI;
    }
    // Rest is DPI
    return &assemble_DPI;
}

uint32_t program_counter;
dynmap symbol_table;

int main(int argc, char **argv) {
    char *line = malloc(sizeof(char) * MAX_LINE_LENGTH + 2);
    uint32_t *binary_instruction = malloc(sizeof(uint32_t));
    // dynarray output = dynarray_create(DYNARRAY_LIMIT);

    FILE *infile = fopen(argv[1], "r");
    if (infile == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    FILE *outfile = fopen(argv[2], "wb");
    if (outfile == NULL) {
        perror("File does not exist.\n");
        exit(EXIT_FAILURE);
    }

    // First pass : build the symbol table
    symbol_table = dynmap_create();
    program_counter = 0;
    while (fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
        // Check if the line is a label
        if (isalpha(line[0]) && line[strlen(line) - 2] == ':') { // len - 2 due to '\0'
            // add an entry to the table where key is label and value is address
            line[strlen(line) - 2] = '\0';
            dynmap_add(symbol_table, strdup(line), program_counter + 4);
        }
        program_counter += 4;
    }

    // fclose(infile);
    // infile = fopen(argv[1], "r");
    rewind(infile);

    program_counter = 0;
    // Second pass: assemble instructions
    while (fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
        // Determines which parsing function to be used: could be a NOP function, directive, DPI, Branch or Single DTI
        line[strlen(line) - 1] = '\0';
        printf("%s\n", line);

        if (*line == '\0') {
            break;
        }

        func_ptr parsing_function = get_function(line);
        *binary_instruction = (parsing_function)(line);
        printf("%x\n", *binary_instruction);
        fprintf(outfile, "%x\n", *binary_instruction); 
        program_counter += 4;
    }

    fclose(infile);
    fclose(outfile);
    free(line);
    free(binary_instruction);
    return EXIT_SUCCESS;
}
