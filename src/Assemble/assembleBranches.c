#include "assembleBranches.h"
#include "dynmap.h"
#include <string.h>
#include <stdlib.h>

#define branch_unconditional_template 335544320

extern dynmap symbol_table;
extern uint32_t program_counter;

uint32_t assemble_branch_unconditional(char *assembly_instruction) {
    // Copies assembly instruction
    unsigned long n = strlen(assembly_instruction);
    char instruction_copy[n + 1];

    for (int i = 0; i <= n; i++) {
        instruction_copy[i] = assembly_instruction[i];
    }

    // Takes the second word
    char *token = strtok(instruction_copy, " ");
    token = strtok(NULL, " ");

    int32_t offset;

    if (token[0] == '#') {
        token++;
        char *endptr;
        offset = strtol(token, &endptr, 16) - program_counter;
    } else {
        offset = dynmap_get(symbol_table, token) - program_counter;
    }

}
uint32_t assemble_branch_register(char *assembly_instruction)
uint32_t assemble_branch_conditional(char *assembly_instruction)

uint32_t assemble_branches(char *assembly_instruction) {

}
