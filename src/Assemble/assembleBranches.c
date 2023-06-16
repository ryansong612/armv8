#include "assembleBranches.h"
#include "dynmap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "../BitUtils/custombit.h"

#define BRANCH_UNCONDITIONAL_TEMPLATE 335544320
#define BRANCH_REGISTER_TEMPLATE 3592355840
#define BRANCH_CONDITIONAL_TEMPLATE 5704253440
#define EQ 0
#define NE 1
#define GE 10
#define LT 11
#define GT 12
#define LE 13
#define AL 14

extern dynmap symbol_table;
extern uint32_t program_counter;

typedef enum {UNCONDITIONAL, REGISTER, CONDITIONAL} branch_state;

struct branch_IR {
    branch_state state;
    int32_t arg; // Second argument, could be either a register or an offset
    int32_t condition;
};

typedef struct branch_IR *branch_IR;

static branch_IR build_branch_IR(char *assembly_instruction) {
    // Copies assembly instruction
    unsigned long n = strlen(assembly_instruction);
    char instruction_copy[n + 1];

    strcpy(instruction_copy, assembly_instruction);

    // Tokenize instruction
    char *first = strtok(instruction_copy, " ");
    char *second = strtok(NULL, " ");

    branch_IR ir = malloc(sizeof(struct branch_IR));
    if (strlen(first) == 1) {
        ir->state = UNCONDITIONAL;
    } else if (first[1] == 'r') {
        ir->state = REGISTER;
    } else {
        ir->state = CONDITIONAL;
    }

    // Second word can be in the form of a literal, branch or register

    if (ir->state == REGISTER) {
        // Second word is a register
        second++;
        ir->arg = atoi(second);
    } else {
        if (isdigit(second[0])) {
            // Second word is a non-label literal
            if (strlen(second) >= 2 && second[1] == 'x') {
                // Literal is represented in hex
                second += 2;
                char *endptr;
                ir->arg = strtol(second, &endptr, 16) - program_counter;
            } else {
                // Literal is represented in decimal
                ir->arg = atoi(second) - program_counter;
            }
        } else {
            // Second word is a label
            printf("Branches get key: %s\n", second);
            printf("int: %i\n", dynmap_get(symbol_table, second) - program_counter);
            ir->arg = dynmap_get(symbol_table, second) - program_counter;
        }
    }

    // Determines condition
    if (ir->state == CONDITIONAL) {
        char *third = first + 2;

        printf("%s\n", third);
        if (strcmp(third, "eq") == 0) {
            ir->condition = EQ;
        } else if (strcmp(third, "ne") == 0) {
            ir->condition = NE;
        } else if (strcmp(third, "ge") == 0) {
            ir->condition = GE;
        } else if (strcmp(third, "lt") == 0) {
            ir->condition = LT;
        } else if (strcmp(third, "gt") == 0) {
            ir->condition = GT;
        } else if (strcmp(third, "le") == 0) {
            ir->condition = LE;
        } else {
            ir->condition = AL;
        }
    }
    return ir;
}

uint32_t assemble_branches(char *assembly_instruction) {
    branch_IR ir = build_branch_IR(assembly_instruction);
    switch (ir->state) {
        case UNCONDITIONAL:
            return BRANCH_UNCONDITIONAL_TEMPLATE | shrink32(ir->arg / 4, 26);
        case REGISTER:
            return BRANCH_REGISTER_TEMPLATE | (shrink32(ir->arg, 5) << 5);
        case CONDITIONAL:
            return BRANCH_CONDITIONAL_TEMPLATE | (shrink32(ir->arg / 4, 19) << 5) | ir->condition;
    }
}
