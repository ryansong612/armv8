#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>

#include "assembleDPI.h"
#include "../BitUtils/custombit.h"

// Defining Constants
#define ADD 0
#define ADDS 1
#define SUB 2
#define SUBS 3
#define MOVZ 2
#define MOVN 0
#define MOVK 3
#define ZERO_REGISTER_ID 31
#define IMMEDIATE '#'
#define BIT_WIDTH_32 'w'
#define BIT_WIDTH_64 'x'
#define IMMEDIATE_SHIFT 12
#define ASR 2
#define ROR 3
#define REGISTER_ADDRESS_SIZE 5
#define IMMEDIATE_BIT_FLAG 4
#define REGISTER_BIT_FLAG 5
#define ARITHMETIC_OPI 2
#define REGISTER_ARITHMETIC_FLAG 1
#define WIDE_MOVE_OPI 5

typedef uint32_t (*func_ptr)(char *);

/*
 * parse_arithmetic takes a string of a line of assembly code with \n stripped off
 * and returns the unsigned 32-bit representation of the instruction that indicates some data to be
 * processed arithmetically
 */
static uint32_t parse_arithmetic(char *assembler_instruction) {
    // convert char* to char[] for tokenization
    unsigned long n = strlen(assembler_instruction);
    char assembler_instruction_arr[n + 1];

    strcpy(assembler_instruction_arr, assembler_instruction);

    // defining variables
    bool alias_rd = false;
    bool alias_rn = false;
    uint32_t rd;
    uint32_t rn;
    uint32_t sf;
    uint32_t m = 0;
    uint32_t opi = ARITHMETIC_OPI;
    uint32_t opc;

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "add") == 0) {
        opc = ADD;
    } else if (strcmp(token, "adds") == 0) {
        opc = ADDS;
    } else if (strcmp(token, "sub") == 0) {
        opc = SUB;
    } else if (strcmp(token, "subs") == 0) {
        opc = SUBS;
    } else if (strcmp(token, "cmp") == 0) {
        opc = SUBS;
        rd = ZERO_REGISTER_ID;
        alias_rd = true;
    } else if (strcmp(token, "cmn") == 0) {
        opc = ADDS;
        rd = ZERO_REGISTER_ID;
        alias_rd = true;
    } else if (strcmp(token, "neg") == 0) {
        opc = SUB;
        rn = ZERO_REGISTER_ID;
        alias_rn = true;
    } else if (strcmp(token, "negs") == 0) {
        opc = SUBS;
        rn = ZERO_REGISTER_ID;
        alias_rn = true;
    }

    // begin tokenization
    // extracting rd
    if (!alias_rd) {
        token = strtok(NULL, ", ");
        assert(token != NULL);
        // setting sf value
        sf = token[0] == BIT_WIDTH_64;

        if (isalpha(token[1])) {
            rd = ZERO_REGISTER_ID;
        } else {
            rd = strtol(token + 1, NULL, BASE_10);
        }
    }

    // extracting rn
    if (!alias_rn) {
        token = strtok(NULL, ", ");
        assert(token != NULL);

        sf = token[0] == BIT_WIDTH_64;

        if (isalpha(token[1])) {
            rn = ZERO_REGISTER_ID;
        } else {
            rn = strtol(token + 1, NULL, BASE_10);
        }
    }

    // extracting operand
    token = strtok(NULL, " ");

    uint32_t assembled_instruction = sf;

    switch (token[0]) {
        case IMMEDIATE: {
            // '#' flags immediate
            // extracting imm12
            uint32_t imm12;
            if (token[1] == '0') { //hex
               imm12 = strtol(token + 1, NULL, BASE_16);
            } else { // decimal
                imm12 = strtol(token + 1, NULL, BASE_10);
            }

            uint32_t sh;

            // shifts in immediate are by default lsl
            token = strtok(NULL, " ");
            if (token != NULL) {
                token = strtok(NULL, " ");
                assert (token != NULL);
                sh = strtol(token + 1, NULL, BASE_10) == IMMEDIATE_SHIFT;
            } else {
                sh = 0;
            }

            // assembling bits
            assembled_instruction <<= 2;
            assembled_instruction |= opc;

            assembled_instruction <<= 3;
            assembled_instruction |= IMMEDIATE_BIT_FLAG;

            assembled_instruction <<= 3;
            assembled_instruction |= opi;

            assembled_instruction <<= 1;
            assembled_instruction |= sh;

            assembled_instruction <<= 12;
            assembled_instruction |= imm12;

            assembled_instruction <<= REGISTER_ADDRESS_SIZE;
            assembled_instruction |= rn;

            assembled_instruction <<= REGISTER_ADDRESS_SIZE;
            assembled_instruction |= rd;

            break;
        }
        case BIT_WIDTH_64:
        case BIT_WIDTH_32: {
            // 'X' or 'W' flags register with 64 bit access
            // extracting rm
            uint32_t rm = strtol(token + 1, NULL, BASE_10);

            // extracting shift type
            token = strtok(NULL, " ");
            uint32_t shift_imm;
            uint32_t shift_type;
            if (token != NULL) {
                if (token[0] == 'l' && token[1] == 's') {
                    shift_type = token[2] == 'r';
                } else {
                    if (token[0] == 'r') {
                        shift_type = ROR;
                    } else {
                        shift_type = ASR;
                    }
                }

                // extracting shift operand
                token = strtok(NULL, " ");
                assert (token != NULL);
                if (token[1] == '0') {
                    // hex
                    shift_imm = strtol(token + 1, NULL, BASE_16);
                } else {
                    // decimal
                    shift_imm = strtol(token + 1, NULL, BASE_10);
                }
            } else {
                shift_imm = 0;
                shift_type = 0;
            }

            // assembling bits
            assembled_instruction <<= 2;
            assembled_instruction |= opc;

            assembled_instruction <<= 1;
            assembled_instruction |= m;

            assembled_instruction <<= 3;
            assembled_instruction |= REGISTER_BIT_FLAG;

            assembled_instruction <<= 1;
            assembled_instruction |= REGISTER_ARITHMETIC_FLAG;

            assembled_instruction <<= 2;
            assembled_instruction |= shift_type;

            assembled_instruction <<= 1;
            assembled_instruction |= 0;

            assembled_instruction <<= REGISTER_ADDRESS_SIZE;
            assembled_instruction |= rm;

            assembled_instruction <<= 6;
            assembled_instruction |= shift_imm;

            assembled_instruction <<= REGISTER_ADDRESS_SIZE;
            assembled_instruction |= rn;

            assembled_instruction <<= REGISTER_ADDRESS_SIZE;
            assembled_instruction |= rd;
            break;
        }
    }
    return assembled_instruction;
}

/*
 * parse_logic takes a string of a line of assembly code with \n stripped off
 * and returns the unsigned 32-bit representation of the instruction that indicates some data to be
 * processed logically with shifts
 */
static uint32_t parse_logic(char *assembler_instruction) {
    // convert char* to char[] for tokenization
    unsigned long instruction_length = strlen(assembler_instruction);
    char assembler_instruction_arr[instruction_length + 1];

    strcpy(assembler_instruction_arr, assembler_instruction);

    uint32_t sf;
    uint32_t opc;
    uint32_t m = 0;
    uint32_t n;
    uint32_t opr = 0;
    uint32_t shift;
    uint32_t rm;
    uint32_t rn;
    uint32_t rd;
    uint32_t imm6;
    bool alias_rd = false;
    bool alias_rn = false;

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "and") == 0) {
        opc = 0;
        n = 0;
    } else if (strcmp(token, "bic") == 0) {
        opc = 0;
        n = 1;
    } else if (strcmp(token, "orr") == 0) {
        opc = 1;
        n = 0;
    } else if (strcmp(token, "orn") == 0) {
        opc = 1;
        n = 1;
    } else if (strcmp(token, "eor") == 0) {
        opc = 2;
        n = 0;
    } else if (strcmp(token, "eon") == 0) {
        opc = 2;
        n = 1;
    } else if (strcmp(token, "ands") == 0) {
        opc = 3;
        n = 0;
    } else if (strcmp(token, "bics") == 0) {
        opc = 3;
        n = 1;
    } else if (strcmp(token, "tst") == 0) {
        opc = 3;
        n = 0;
        rd = ZERO_REGISTER_ID;
        alias_rd = true;
    } else if (strcmp(token, "mvn") == 0) {
        opc = 1;
        n = 1;
        rn = ZERO_REGISTER_ID;
        alias_rn = true;
    } else if (strcmp(token, "mov") == 0) {
        opc = 1;
        n = 0;
        rn = ZERO_REGISTER_ID;
        alias_rn = true;
    }

    // extracting Rd
    if (!alias_rd) {
        token = strtok(NULL, ", ");
        assert(token != NULL);
        sf = token[0] == BIT_WIDTH_64;
        if (isalpha(token[1])) {
            rd = ZERO_REGISTER_ID;
        } else {
            rd = strtol(token + 1, NULL, BASE_10);
        }
    }

    // extracting Rn
    if (!alias_rn) {
        token = strtok(NULL, ", ");
        assert(token != NULL);
        sf = token[0] == BIT_WIDTH_64;
        if (isalpha(token[1])) {
            rn = ZERO_REGISTER_ID;
        } else {
            rn = strtol(token + 1, NULL, BASE_10);
        }
    }

    // extracting Rm
    token = strtok(NULL, ", ");
    assert (token != NULL);
    if (isalpha(token[1])) {
        rm = ZERO_REGISTER_ID;
    } else {
        rm = strtol(token + 1, NULL, BASE_10);
    }

    // extracting shift type
    token = strtok(NULL, " ");
    if (token != NULL) {
        if (token[0] == 'l') {
            // shift is 1 if lsr, 0 if lsl
            shift = token[2] == 'r';
        } else {
            if (token[0] == 'r') {
                shift = ROR;
            } else {
                shift = ASR;
            }
        }
    } else {
        shift = 0;
        imm6 = 0;
    }

    opr <<= 2;
    opr |= shift;
    opr <<= 1;
    opr |= n;

    token = strtok(NULL, " ");
    if (token != NULL) {
        if (token[1] == '0') {
            imm6 = strtol(token + 1, NULL, BASE_16);
        } else {
            imm6 = strtol(token + 1, NULL, BASE_10);
        }
    }

    uint32_t assembled_instruction = sf;

    assembled_instruction <<= 2;
    assembled_instruction |= opc;

    assembled_instruction <<= 1;
    assembled_instruction |= m;

    assembled_instruction <<= 3;
    assembled_instruction |= REGISTER_BIT_FLAG;

    assembled_instruction <<= 4;
    assembled_instruction |= opr;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rm;

    assembled_instruction <<= 6;
    assembled_instruction |= imm6;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rn;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rd;

    return assembled_instruction;
}

/*
 * parse_wide_move takes a string of a line of assembly code with \n stripped off
 * and returns the unsigned 32-bit representation of the instruction that indicates some data to be
 * processed with a wide move action
 */
static uint32_t parse_wide_move(char *assembler_instruction) {
    unsigned long instruction_length = strlen(assembler_instruction);
    char assembler_instruction_arr[instruction_length + 1];

    strcpy(assembler_instruction_arr, assembler_instruction);

    uint32_t sf;
    uint32_t opc;
    uint32_t opi = WIDE_MOVE_OPI;
    uint32_t rd;
    uint32_t hw;
    uint32_t imm16;

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "movn") == 0) {
        opc = MOVN;
    } else if (strcmp(token, "movz") == 0) {
        opc = MOVZ;
    } else if (strcmp(token, "movk") == 0) {
        opc = MOVK;
    }

    // extracting Rd
    token = strtok(NULL, ", ");
    assert (token != NULL);
    sf = token[0] == BIT_WIDTH_64;
    if (isalpha(token[1])) {
        rd = ZERO_REGISTER_ID;
    } else {
        rd = strtol(token + 1, NULL, BASE_10);
    }

    // extracting imm16
    token = strtok(NULL, ", ");
    assert (token != NULL);
    if (token[1] == '0') { // #0x..
        imm16 = strtol(token + 1, NULL, BASE_16);
    } else {
        imm16 = strtol(token + 1, NULL, BASE_10);
    }


    // determining and extracting shifts
    token = strtok(NULL, ", ");
    if (token == NULL) {
        // no shifts
        hw = 0;
    } else {
        // now token should be lsl
        token = strtok(NULL, " ");
        // Must either be 0 or 16 so hw = 1
        uint32_t shift_amount;
        if (token[1] == '0') { // #0x..
            shift_amount = strtol(token + 1, NULL, BASE_16);
        } else {
            shift_amount = strtol(token + 1, NULL, BASE_10);
        }
        hw = shift_amount / 16;
    }

    uint32_t assembled_instruction = sf;

    assembled_instruction <<= 2;
    assembled_instruction |= opc;

    assembled_instruction <<= 3;
    assembled_instruction |= IMMEDIATE_BIT_FLAG;

    assembled_instruction <<= 3;
    assembled_instruction |= opi;

    assembled_instruction <<= 2;
    assembled_instruction |= hw;

    assembled_instruction <<= 16;
    assembled_instruction |= imm16;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rd;

    return assembled_instruction;
}

/*
 * parse_multiply takes a string of a line of assembly code with \n stripped off
 * and returns the unsigned 32-bit representation of the instruction that indicates some data to be
 * processed in a multiplicative manner
 */
static uint32_t parse_multiply(char *assembler_instruction) {
    unsigned long instruction_length = strlen(assembler_instruction);
    char assembler_instruction_arr[instruction_length + 1];

    strcpy(assembler_instruction_arr, assembler_instruction);

    uint32_t sf;
    uint32_t opc = 0;
    uint32_t multiply_encoding_21_28 = 216;
    uint32_t rm;
    uint32_t x;
    uint32_t ra;
    uint32_t rn;
    uint32_t rd;

    bool alias_ra = false;

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "madd") == 0) {
        x = 0;
    } else if (strcmp(token, "msub") == 0) {
        x = 1;
    } else if (strcmp(token, "mul") == 0) {
        x = 0;
        alias_ra = true;
        ra = ZERO_REGISTER_ID;
    } else if (strcmp(token, "mneg") == 0) {
        x = 1;
        alias_ra = true;
        ra = ZERO_REGISTER_ID;
    }

    // extracting Rd
    token = strtok(NULL, ", ");
    assert (token != NULL);
    sf = token[0] == BIT_WIDTH_64;
    if (isalpha(token[1])) {
        rd = ZERO_REGISTER_ID;
    } else {
        rd = strtol(token + 1, NULL, BASE_10);
    }

    // extracting Rn
    token = strtok(NULL, ", ");
    assert(token != NULL);
    if (isalpha(token[1])) {
        rn = ZERO_REGISTER_ID;
    } else {
        rn = strtol(token + 1, NULL, BASE_10);
    }

    // extracting Rm
    token = strtok(NULL, ", ");
    assert(token != NULL);
    if (isalpha(token[1])) {
        rm = ZERO_REGISTER_ID;
    } else {
        rm = strtol(token + 1, NULL, BASE_10);
    }

    // extracting Ra
    token = strtok(NULL, ", ");
    if (!alias_ra) {
        assert(token != NULL);
        if (isalpha(token[1])) {
            ra = ZERO_REGISTER_ID;
        } else {
            ra = strtol(token + 1, NULL, BASE_10);
        }
    }

    // Assembling instructions into binary
    uint32_t assembled_instruction = sf;

    assembled_instruction <<= 2;
    assembled_instruction |= opc;

    assembled_instruction <<= 8;
    assembled_instruction |= multiply_encoding_21_28;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rm;

    assembled_instruction <<= 1;
    assembled_instruction |= x;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= ra;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rn;

    assembled_instruction <<= REGISTER_ADDRESS_SIZE;
    assembled_instruction |= rd;

    return assembled_instruction;
}

/*
 * parse_dpi takes a string of a line of assembly code with \n stripped off
 * and returns the function pointer towards the designated static parser for the data processing instructions
 */
static func_ptr parse_dpi(char *assembler_instruction) {
    unsigned long n = strlen(assembler_instruction);
    char assembler_instruction_arr[n + 1];

    strcpy(assembler_instruction_arr, assembler_instruction);

    char *token = strtok(assembler_instruction_arr, " ");

    if (strcmp(token, "add") == 0
    || strcmp(token, "adds") == 0
    || strcmp(token, "sub") == 0
    || strcmp(token, "subs") == 0
    || strcmp(token, "cmp") == 0
    || strcmp(token, "cmn") == 0
    || strcmp(token, "neg") == 0
    || strcmp(token, "negs") == 0) {
        return &parse_arithmetic;
    }

    if (strcmp(token, "and") == 0
    || strcmp(token, "ands") == 0
    || strcmp(token, "bic") == 0
    || strcmp(token, "bics") == 0
    || strcmp(token, "eor") == 0
    || strcmp(token, "orr") == 0
    || strcmp(token, "eon") == 0
    || strcmp(token, "orn") == 0
    || strcmp(token, "tst") == 0
    || strcmp(token, "mvn") == 0
    || strcmp(token, "mov") == 0) {
        return &parse_logic;
    }

    if (strcmp(token, "movn") == 0
    || strcmp(token ,"movk") == 0
    || strcmp(token, "movz") == 0) {
        return &parse_wide_move;
    }

    if (strcmp(token, "madd") == 0
        || strcmp(token, "msub") == 0
        || strcmp(token ,"mul") == 0
        || strcmp(token, "mneg") == 0) {
        return &parse_multiply;
    }

    return NULL;
}

/*
 * assemble_DPI takes a string of a line of assembly code with \n stripped off
 * and returns the 32-bit instruction required
 *
 * This will be the function that will be used in assemble.c
 */
uint32_t assemble_DPI(char *assembler_instruction) {
    func_ptr parse_func = parse_dpi(assembler_instruction);
    return (parse_func)(assembler_instruction);
}