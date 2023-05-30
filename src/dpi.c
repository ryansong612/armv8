#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "emulate.h"
#include "readnwrite.h"

#define WIDE_MOVE_FLAG_BITS 5
#define ARITHMETIC_FLAG_BITS 2
#define OPI_START_BIT 23


// retrieving global variables from emulator
extern GeneralPurposeRegister generalPurposeRegisters[NUM_REGISTERS];
extern GeneralPurposeRegister zeroRegister;
extern uint64_t programCounter;
extern PSTATE pStateRegister;


// ------------------------------------- CUSTOM HELPER FUNCTIONS ----------------------------------------------------
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
        v <<= 1;
        start++;
    }
    return v;
}

// returns true if and only if num's start-end bits are the same as tgt
// pre: tgt is not 0
bool match_bits(uint64_t num, int64_t tgt, int idx) {
    return (((num >> idx) & tgt)) == tgt;
}

GeneralPurposeRegister* find_register(uint32_t key) {
    for (int i = 0; i < NUM_REGISTERS; i ++) {
        if (generalPurposeRegisters[i].id == key) {
            return &generalPurposeRegisters[i];
        }
    }
    return NULL;
}

// ------------------------------------- Executing Operations ----------------------------------------------------
void dpi_arithmetic(uint32_t instruction) {
    GeneralPurposeRegister *rd = find_register(get_bits(instruction, 0, 4));
    GeneralPurposeRegister *rn = find_register(get_bits(instruction, 5, 9));
    uint32_t imm12_unsigned = get_bits(instruction, 10, 21);
    // Check for sh (sign for shift)
    if (match_bits(instruction, 1, 22)) {
        // shift by 12-bits is needed
        imm12_unsigned <<= 12;
    }

    int32_t imm12 = (int32_t) imm12_unsigned;

    if (match_bits(instruction, 1, 31)) {
        int64_t rn_val = read_64(rn);
        // access bit-width is 64-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_64(rd, rn_val + imm12);
                break;
            case 1:
                write_64(rd, rn_val + imm12);
                // reads the result after operation
                int64_t res = read_64(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < imm12) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                int64_t sign_rn_val = get_bit_register64(rn_val, 63);
                int64_t sign_imm12 = get_bit_register64(imm12, 63);
                int64_t sign_res = get_bit_register64(res, 63);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_imm12)
                        && (sign_res != sign_rn_val);
                break;
            case 2:
                write_64(rd, rn_val - imm12);
                break;
            case 3:
                write_64(rd, rn_val - imm12);
                // reads the result after operation
                res = read_64(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < imm12) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                sign_rn_val = get_bit_register64(rn_val, 63);
                sign_imm12 = 1;
                sign_res = get_bit_register64(res, 63);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_imm12)
                                                       && (sign_res != sign_rn_val);
                break;
        }
        return;
    }

    if (match_bits(instruction, 0, 31)) {
        int32_t rn_val = read_32(rn);
        // access bit-width is 32-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_32(rd, rn_val + imm12);
                break;
            case 1:
                write_32(rd, rn_val + imm12);
                // reads the result after operation
                int32_t res = read_32(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register32(res, NUM_REGISTERS);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < imm12) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                int32_t sign_rn_val = get_bit_register32(rn_val, 31);
                int32_t sign_imm12 = 1;
                int32_t sign_res = get_bit_register32(res, 31);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_imm12)
                                                       && (sign_res != sign_rn_val);
                break;
            case 2:
                write_32(rd, rn_val - imm12);
                break;
            case 3:
                write_32(rd, rn_val - imm12);
                // reads the result after operation
                res = read_32(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < imm12) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                sign_rn_val = get_bit_register32(rn_val, 31);
                sign_imm12 = 1;
                sign_res = get_bit_register32(res, 31);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_imm12)
                                                       && (sign_res != sign_rn_val);
                break;
        }
        return;
    }


}

void dpi_wide_move(uint32_t instruction) {

}

// Assuming all registers have their bit arrays reversed
void execute_DPImmediate(uint32_t instruction) {
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
