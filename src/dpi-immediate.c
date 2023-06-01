#include <stdio.h>
#include <stdint.h>
#include "custombit.h"
#include "emulate.h"
#include "readnwrite.h"
#include "dpi-immediate.h"

#define WIDE_MOVE_FLAG_BITS 5
#define ARITHMETIC_FLAG_BITS 2
#define OPI_START_BIT 23

// retrieving global variables from emulator
extern GeneralPurposeRegister generalPurposeRegisters[NUM_REGISTERS];
extern GeneralPurposeRegister zeroRegister;
extern uint64_t programCounter;
extern PSTATE pStateRegister;

// ------------------------------------- CUSTOM HELPER FUNCTIONS -------------------------------------------------------
GeneralPurposeRegister* find_register(uint32_t key) {
    if (key == ZERO_REGISTER_ID) {
        return &zeroRegister;
    }

    for (int i = 0; i < NUM_REGISTERS; i ++) {
        if (generalPurposeRegisters[i].id == key) {
            return &generalPurposeRegisters[i];
        }
    }
    return NULL;
}

void arithmetic_helper_64(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, int32_t op2) {
    // case: opc (29-30)
    switch (get_bits(instruction, 29, 30)) {
        case ADD:
            write_64(rd, rn_val + op2);
            break;
        case ADDS:
            write_64(rd, rn_val + op2);

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
            if (res < rn_val || res < op2) {
                pStateRegister.carryConditionFlag = 1;
            } else {
                pStateRegister.carryConditionFlag = 0;
            }

            // updates pState sign overflow condition flag
            int64_t sign_rn_val = get_bit_register64(rn_val, 63);
            int64_t sign_op2 = get_bit_register64(op2, 63);
            int64_t sign_res = get_bit_register64(res, 63);
            pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                   && (sign_res != sign_rn_val);
            break;
        case SUB:
            write_64(rd, rn_val - op2);
            break;
        case SUBS:
            write_64(rd, rn_val - op2);
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
            if (res < rn_val || res < op2) {
                pStateRegister.carryConditionFlag = 1;
            } else {
                pStateRegister.carryConditionFlag = 0;
            }

            // updates pState sign overflow condition flag
            sign_rn_val = get_bit_register64(rn_val, 63);
            sign_op2 = 1;
            sign_res = get_bit_register64(res, 63);
            pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                   && (sign_res != sign_rn_val);
            break;
    }
}

void arithmetic_helper_32(GeneralPurposeRegister *rd, uint32_t instruction, int32_t rn_val, int32_t op2) {
    // case: opc (29-30)
    switch (get_bits(instruction, 29, 30)) {
        case ADD:
            write_32(rd, rn_val + op2);
            break;
        case ADDS:
            write_32(rd, rn_val + op2);

            // reads the result after operation
            int32_t res = read_32(rd);
            // updates pState flag
            pStateRegister.negativeConditionFlag = get_bit_register32(res, 31);

            // updates pState zero condition flag
            if (res == 0) {
                pStateRegister.zeroConditionFlag = 1;
            } else {
                pStateRegister.zeroConditionFlag = 0;
            }

            // updates pState carry condition flag
            if (res < rn_val || res < op2) {
                pStateRegister.carryConditionFlag = 1;
            } else {
                pStateRegister.carryConditionFlag = 0;
            }

            // updates pState sign overflow condition flag
            int64_t sign_rn_val = get_bit_register32(rn_val, 31);
            int64_t sign_op2 = get_bit_register32(op2, 31);
            int64_t sign_res = get_bit_register32(res, 31);
            pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                   && (sign_res != sign_rn_val);
            break;
        case SUB:
            write_32(rd, rn_val - op2);
            break;
        case SUBS:
            write_32(rd, rn_val - op2);
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
            if (res < rn_val || res < op2) {
                pStateRegister.carryConditionFlag = 1;
            } else {
                pStateRegister.carryConditionFlag = 0;
            }

            // updates pState sign overflow condition flag
            sign_rn_val = get_bit_register32(rn_val, 31);
            sign_op2 = 1;
            sign_res = get_bit_register32(res, 31);
            pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                   && (sign_res != sign_rn_val);
            break;
    }
}

// ------------------------------------- Executing Operations ----------------------------------------------------------
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
        // access bit-width is 64-bit
        int64_t rn_val = read_64(rn);
        arithmetic_helper_64(rd, instruction, rn_val, imm12);
        return;
    }

    if (match_bits(instruction, 0, 31)) {
        // access bit-width is 32-bit
        int32_t rn_val = read_32(rn);
        arithmetic_helper_32(rd, instruction, rn_val, imm12);
    }
}

void dpi_wide_move(uint32_t instruction) {

    GeneralPurposeRegister *rd = find_register(get_bits(instruction, 0, 4));

    uint32_t hw = get_bits(instruction, 21, 22);
    uint32_t imm16 = get_bits(instruction, 5, 20);
    uint32_t opc = get_bits(instruction, 29, 30);
    uint32_t sf = (instruction >> 31) & 1;
    int64_t op = imm16 << (hw * 16);

    int shift = (int) hw * 16;
    int end_shift = (int) hw * 16 + 15;
    int64_t left_part_64 = get_bits64((*rd).val, end_shift, 63) << end_shift;
    uint64_t middle_part = op << shift;
    uint64_t right_part = (uint64_t) get_bits64((*rd).val, 0, shift);



    if (sf == 1) {
        // bit-width access is 64-bit
        switch (opc) {
            case MOVZ :
                write_64(rd, op);
                break;
            case MOVN:
                write_64(rd, ~op); // not sure whether this is correct, check spec
                break;
            case MOVK:
                write_64(rd, left_part_64 + middle_part + right_part); // not sure about the type conversion
                break;
            default:
                break;
        }
        return;
    }

    if (sf == 0) {
        // bit-width access is 32-bit
        switch (opc) {
            case MOVZ:
                write_32(rd, op);
                break;
            case MOVN:
                write_32(rd, ~op);
                break;
            case MOVK:
                write_32(rd, middle_part + right_part);
                break;
            default:
                break;
        }
        return;
    }
}

// Assuming all registers have their bit arrays reversed
bool execute_DPIImmediate(uint32_t instruction) {
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

    // Increment Program Counter by 1
    programCounter++;
    return true;
}
