#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "dynmap.h"
#include "assembleBranches.h"
#include "assembleDPI.h"
#include "assembleSingleDTI.h"
#include "../BitUtils/custombit.h"

#define MAX_LINE_LENGTH 256
#define NOP 3573751839; // No operation instruction

typedef uint32_t (*func_ptr)(char *);

// -------------------------------------------- SPECIAL FUNCTIONS ------------------------------------------------------
uint32_t NOP_function(char *instruction) {
    return NOP;
}

uint32_t directive_function(char *assembly_instruction) {
    // Copies assembly instruction
    char instruction_copy[strlen(assembly_instruction) + 1];
    strcpy(instruction_copy, assembly_instruction);

    // Converts second word into a long and returns it
    char *directive = strtok(instruction_copy, " ");
    directive = strtok(NULL, " ");
    if (directive[0] == '0') { // 0x..
        return strtol(directive, NULL, BASE_16);
    } else {
        return strtol(directive, NULL, BASE_10);
    }
}

// ---------------------------------- Mapping of instruction to function pointer ---------------------------------------
func_ptr get_function(char* assembly_instruction) {
    // Special Functions:
    // No Operation
    if (strcmp(assembly_instruction, "nop") == 0) {
        return &NOP_function;
    }
    // Directive
    if (strstr(assembly_instruction, ".int") != NULL) {
        return &directive_function;
    }
    // Instructions:
    // Branches
    if (assembly_instruction[0] == 'b' && assembly_instruction[1] != 'i') {
        return &assemble_branches;
    }
    // Single DTI
    if (assembly_instruction[0] == 'l' || (assembly_instruction[0] == 's' && assembly_instruction[1] == 't')) {
        return &assemble_single_DTI;
    }
    // Rest is DPI
    return &assemble_DPI;
}

// Global variables to be accessed elsewhere in the program
uint32_t program_counter;
dynmap symbol_table;

// -------------------------------------------- ENTRY POINT OF PROGRAMME ---------------------------------------
int main(int argc, char **argv) {
    // Initializing variables
    char *line = malloc(sizeof(char) * MAX_LINE_LENGTH + 2);
    uint32_t binary_instruction;

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
        // Skips empty lines         
        if (*line == '\n') {
            continue;
        }

        // Check if the line is a label
        if (strchr(line, ':') != NULL) {
            // add an entry to the table where key is label and value is address
            line[strchr(line, ':') - line] = '\0';
            dynmap_add(symbol_table, line, program_counter);
            continue;
        }
        program_counter += 4;
    }

    // Go back to the beginning of infile
    rewind(infile);

    // Second pass: assemble instructions
    program_counter = 0;
    while (fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
        // Remove '\n'
        line[strlen(line) - 1] = '\0';

        // Skips empty lines  
        char *temp = line;
        while (isspace(*temp)) temp++;
        if (*temp == '\0') {
            continue;
        }

        // Skips labels
        if (strchr(temp, ':') != NULL) {
            continue;
        }

        // Determines which parsing function to be used: could be a NOP function, directive, DPI, Branch or Single DTI
        func_ptr parsing_function = get_function(temp);
        binary_instruction = (parsing_function)(temp);

        fwrite(&binary_instruction, sizeof(uint32_t), 1, outfile); 

        program_counter += 4;
    }
    fclose(infile);
    fclose(outfile);
    free(line);
    return EXIT_SUCCESS;
}
