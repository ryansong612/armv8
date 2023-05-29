#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define WIDE_MOVE_FLAG_BITS 5
#define ARITHMETIC_FLAG_BITS 2
#define OPI_START_BIT 23

typedef struct {
    uint32_t id; // id of register (0-32), 32 for zero register
    int64_t val; // value stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;

PSTATE pstateRegister = {0, 0, 0, 0};

int32_t read_register_32(GeneralPurposeRegister *gpr) {
    if ((*gpr).id == 31) {
        return 0;
    }
    int32_t val = (*gpr).val;
    return val;
}

int64_t read_register_64(GeneralPurposeRegister *gpr) {
    if ((*gpr).id == 31) {
        return 0;
    }
    return (*gpr).val;
}

void write_register_with32(GeneralPurposeRegister *gpr, int32_t v) {
    if ((*gpr).id == 31) {
        return;
    }
    (*gpr).val = v;
}

void write_register_with64(GeneralPurposeRegister *gpr, int64_t v) {
    if ((*gpr).id == 31) {
        return;
    }
    (*gpr).val = v;
}

// only works for registers
int64_t get_bit_register64(int64_t num, int idx) {
    return (num >> idx) & 1;
}

int32_t get_bit_register32(int32_t num, int idx) {
    return (num >> idx) & 1;
}

// only works for instructions
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
bool match_bits(uint64_t num, int64_t tgt, int idx) {
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
        int64_t val = read_register_64(gpr);
        // access bit-width is 64-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with64(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with64(gpr, (*gpr).val + imm12);
                // reads the result after operation
                int64_t res = read_register_64(gpr);
                // updates pstate flag
                pstateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pstate zero condition flag
                if (res == 0) {
                    pstateRegister.zeroConditionFlag = 1;
                } else {
                    pstateRegister.zeroConditionFlag = 0;
                }

                // updates pstate carry condition flag
                if (res < val || res < imm12) {
                    pstateRegister.carryConditionFlag = 1;
                } else {
                    pstateRegister.carryConditionFlag = 0;
                }

                // updates pstate sign overflow condition flag
                int64_t sign_val = get_bit_register64(val, 63);
                int64_t sign_imm12 = get_bit_register64(imm12, 63);
                int64_t sign_res = get_bit_register64(res, 63);
                pstateRegister.overflowConditionFlag = (sign_val != sign_imm12)
                        && (sign_res != sign_val);
                break;
            case 2:
                write_register_with64(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with64(gpr, (*gpr).val - imm12);
                // reads the result after operation
                res = read_register_64(gpr);
                // updates pstate flag
                pstateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pstate zero condition flag
                if (res == 0) {
                    pstateRegister.zeroConditionFlag = 1;
                } else {
                    pstateRegister.zeroConditionFlag = 0;
                }

                // updates pstate carry condition flag
                if (res < val || res < imm12) {
                    pstateRegister.carryConditionFlag = 1;
                } else {
                    pstateRegister.carryConditionFlag = 0;
                }

                // updates pstate sign overflow condition flag
                sign_val = get_bit_register64(val, 63);
                sign_imm12 = 1;
                sign_res = get_bit_register64(res, 63);
                pstateRegister.overflowConditionFlag = (sign_val != sign_imm12)
                                                       && (sign_res != sign_val);
                break;
        }
        return;
    }

    if (match_bits(instruction, 0, 31)) {
        int32_t val = read_register_32(gpr);
        // access bit-width is 32-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with32(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with32(gpr, (*gpr).val + imm12);
                // reads the result after operation
                int32_t res = read_register_32(gpr);
                // updates pstate flag
                pstateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pstate zero condition flag
                if (res == 0) {
                    pstateRegister.zeroConditionFlag = 1;
                } else {
                    pstateRegister.zeroConditionFlag = 0;
                }

                // updates pstate carry condition flag
                if (res < val || res < imm12) {
                    pstateRegister.carryConditionFlag = 1;
                } else {
                    pstateRegister.carryConditionFlag = 0;
                }

                // updates pstate sign overflow condition flag
                int32_t sign_val = get_bit_register32(val, 31);
                int32_t sign_imm12 = 1;
                int32_t sign_res = get_bit_register32(res, 31);
                pstateRegister.overflowConditionFlag = (sign_val != sign_imm12)
                                                       && (sign_res != sign_val);
                break;
            case 2:
                write_register_with32(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with32(gpr, (*gpr).val - imm12);
                // reads the result after operation
                res = read_register_32(gpr);
                // updates pstate flag
                pstateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pstate zero condition flag
                if (res == 0) {
                    pstateRegister.zeroConditionFlag = 1;
                } else {
                    pstateRegister.zeroConditionFlag = 0;
                }

                // updates pstate carry condition flag
                if (res < val || res < imm12) {
                    pstateRegister.carryConditionFlag = 1;
                } else {
                    pstateRegister.carryConditionFlag = 0;
                }

                // updates pstate sign overflow condition flag
                sign_val = get_bit_register32(val, 31);
                sign_imm12 = 1;
                sign_res = get_bit_register32(res, 31);
                pstateRegister.overflowConditionFlag = (sign_val != sign_imm12)
                                                       && (sign_res != sign_val);
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
    printf("%lli\n", get_bit_register64(i, 0));
    return 0;
}
