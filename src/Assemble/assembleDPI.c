#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <_ctype.h>

#define ADD 0
#define ADDS 1
#define SUB 2
#define SUBS 3
#define ZERO_REGISTER_ID 31
#define IMMEDIATE '#'
#define REGISTER_32 'X'
#define REGISTER_64 'W'
#define IMMEDIATE_SHIFT 12
#define ASR 2
#define REGISTER_ADDRESS_SIZE 5
#define IMMEDIATE_BIT_FLAG 4
#define REGISTER_BIT_FLAG 5
#define ARITHMETIC_OPI 2
#define WIDE_MOVE_OPI 5

typedef uint32_t (*func_ptr)(char *);

uint32_t parse_arithmetic(char *assembler_instruction) {
    // convert char* to char[] for tokenization
    unsigned long n = strlen(assembler_instruction);
    char assembler_instruction_arr[n + 1];
    for (int i = 0; i <= n; i++) {
        // retrieving every instruction after operand
        assembler_instruction_arr[i] = assembler_instruction[i];
    }

    uint32_t opc;

    char *token = strtok(assembler_instruction_arr, ", ");

    if (strcmp(token, "add") == 0) {
        opc = ADD;
    } else if (strcmp(token, "adds") == 0) {
        opc = ADDS;
    } else if (strcmp(token, "sub") == 0) {
        opc = SUB;
    } else if (strcmp(token, "subs") == 0) {
        opc = SUBS;
    }

    // defining variables
    uint32_t rd;
    uint32_t rn;
    uint32_t sf;
    uint32_t opi = 2;

    // begin tokenization
    // extracting rd
    token = strtok(assembler_instruction_arr, ", ");
    assert (token != NULL);

    // setting sf value
    sf = token[0] == REGISTER_64;

    if (isalpha(token[1])) {
        rd = ZERO_REGISTER_ID;
    } else {
        rd = strtol(token + 1, NULL, 10);
    }



    // extracting rn
    token = strtok(NULL, ", ");
    assert (token != NULL);
    if (isalpha(token[1])) {
        rn = ZERO_REGISTER_ID;
    } else {
        rn = strtol(token + 1, NULL, 10);
    }

    // extracting operand
    token = strtok(NULL, " ");
    assert (token != NULL);

    uint32_t assembled_instruction = 0;

    switch (token[0]) {
        case IMMEDIATE: {
            // '#' flags immediate
            // extracting imm12
            uint32_t imm12 = strtol(token + 1, NULL, 16);

            // shifts in immediate are by default lsl
            token = strtok(NULL, " ");
            assert (token != NULL);

            // extracting num_of_bits to shift
            token = strtok(NULL, " ");
            assert (token != NULL);
            uint32_t sh = strtol(token + 1, NULL, 16) == IMMEDIATE_SHIFT;

            // assembling bits
            assembled_instruction += sf;
            assembled_instruction <<= 1;

            assembled_instruction += opc;
            assembled_instruction <<= 2;

            assembled_instruction += IMMEDIATE_BIT_FLAG;
            assembled_instruction <<= 3;

            assembled_instruction += ARITHMETIC_OPI;
            assembled_instruction <<= 3;

            assembled_instruction += sh;
            assembled_instruction <<= 1;

            assembled_instruction += imm12;
            assembled_instruction <<= 12;

            assembled_instruction += rn;
            assembled_instruction <<= REGISTER_ADDRESS_SIZE;

            assembled_instruction += rd;
            break;
        }
        case REGISTER_64:
        case REGISTER_32: {
            // 'X' or 'W' flags register with 64 bit access
            // extracting rm
            uint32_t rm = strtol(token + 1, NULL, 10);

            // extracting shift type
            token = strtok(NULL, " ");
            assert (token != NULL);
            uint32_t shift_type;
            if (token[0] == 'l' && token[1] == 's') {
                shift_type = token[2] == 'r';
            } else {
                shift_type = ASR;
            }

            // extracting shift operand
            token = strtok(NULL, " ");
            assert (token != NULL);
            uint32_t shift_imm = strtol(token + 1, NULL, 16);

            // assembling bits
            assembled_instruction += sf;
            assembled_instruction <<= 1;

            assembled_instruction += opc;
            assembled_instruction <<= 2;

            // M = 0 for arithmetic in dpi-register
            assembled_instruction += 0;
            assembled_instruction <<= 1;

            assembled_instruction += REGISTER_BIT_FLAG;
            assembled_instruction <<= 3;

            assembled_instruction += 1;
            assembled_instruction <<= 1;

            assembled_instruction += shift_type;
            assembled_instruction <<= 2;

            assembled_instruction += 0;
            assembled_instruction <<= 1;

            assembled_instruction += rm;
            assembled_instruction += REGISTER_ADDRESS_SIZE;

            assembled_instruction += shift_imm;
            assembled_instruction <<= 6;

            assembled_instruction += rn;
            assembled_instruction <<= 5;

            assembled_instruction += rd;
            assembled_instruction <<= 5;
            break;
        }
    }
    return assembled_instruction;
}

func_ptr parse_dpi(char *assembler_instruction) {
    unsigned long n = strlen(assembler_instruction);
    char assembler_instruction_arr[n + 1];
    for (int i = 0; i <= n; i++) {
        assembler_instruction_arr[i] = assembler_instruction[i];
    }

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "add") == 0) {
        return &parse_arithmetic;
    }
}

int main(void) {
    parse_dpi("add xor, x2, #1642, lsl #12");
}