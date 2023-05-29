#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define WIDE_MOVE_FLAG_BITS 5
#define ARITHMETIC_FLAG_BITS 2
#define OPI_START_BIT 23

typedef struct {
    int8_t id; // id of register (0-32), 32 for zero register
    uint64_t val; // value stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;

uint32_t read_register_32(GeneralPurposeRegister *gpr) {
    uint32_t val = (*gpr).val;
    return val;
}

uint64_t read_register_64(GeneralPurposeRegister *gpr) {
    return (*gpr).val;
}

void write_register_with32(GeneralPurposeRegister *gpr, uint32_t v) {
    (*gpr).val = v;
}

void write_register_with64(GeneralPurposeRegister *gpr, uint64_t v) {
    (*gpr).val = v;
}

uint32_t get_bit(uint32_t num, int idx) {
    return ((num >> idx) & 1);
}

uint32_t get_bits(uint32_t num, int start, int end) {
    uint32_t v = ((num >> start) & 1) << 1;
    start++;
    while (start <= end) {
        v += ((num >> start) & 1);
        v = v << 1;
        start++;
    }
    return v;
}

// returns true if and only if num's start-end bits are the same as tgt
// pre: tgt is not 0
bool match_bits(uint64_t num, uint64_t tgt, int idx) {
    return (((num >> idx) & tgt)) == tgt;
}

void dpi_arithmetic(uint32_t instruction) {
    GeneralPurposeRegister *gpr;
    uint32_t imm12 = get_bits(instruction, 10, 21);
    // Check for sh (sign for shift)
    if (match_bits(instruction, 1, 22)) {
        // shift by 12-bits is needed
        imm12 <<= 12;
    }

    if (match_bits(instruction, 1, 31)) {
        // access bit-width is 64-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with64(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with64(gpr, (*gpr).val + imm12);
                do something;
                break;
            case 2:
                write_register_with64(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with64(gpr, (*gpr).val - imm12);
                do something;
                break;
        }
        return;
    }

    if (match_bits(instruction, 0, 31)) {
        // access bit-width is 32-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with32(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with32(gpr, (*gpr).val + imm12);
                do something;
                break;
            case 2:
                write_register_with32(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with32(gpr, (*gpr).val - imm12);
                do something;
                break;
        }
        return;
    }


}

void dpi_wide_move(uint32_t instruction) {
    printf("Wide Move!");
}

// Assuming all registers have their bit arrays reversed
void parse_DPImmediate(uint32_t instruction) {
    // determining operation

    // CASE 1: when opi is 010 -> arithmetic
    if (match_bits(instruction, ARITHMETIC_FLAG_BITS, OPI_START_BIT)) {
        dpi_arithmetic(instruction);
    }
    // CASE 2: when opi is 101 -> wide move
    else if (match_bits(instruction, WIDE_MOVE_FLAG_BITS, OPI_START_BIT)) {
        dpi_wide_move(instruction);
    }

    // DO NOTHING for other values of opi
}

int main(int argc, char **argv) {
    uint64_t i = 84; // 1010100
    printf("%i\n", match_bits(i, 5, 1));
    return 0;
}
