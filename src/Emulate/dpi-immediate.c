#include <stdio.h>
#include <stdint.h>

#include "../BitUtils/custombit.h"
#include "dpi-immediate.h"
#include "emulate.h"
#include "readnwrite.h"

// Operation Constants (Flags)
#define ADD 0
#define ADDS 1
#define SUB 2
#define SUBS 3
#define MOVZ 2
#define MOVN 0
#define MOVK 3

// Indices
#define OPI_START_BIT 23
#define ZERO_REGISTER_ID 31
#define SHIFT_BIT 22
#define ACCESS_MODE_BIT 31
#define RD_START 0
#define RD_END 4
#define RN_START 5
#define RN_END 9
#define IMM12_START 10
#define IMM12_END 21
#define HW_START 21
#define HW_END 22
#define IMM16_START 5
#define IMM16_END 20
#define OPC_START 29
#define OPC_END 30

// Util Constants
#define ZERO_EXTEND_MASK 0xFFFFFFFFLL

// retrieving global variables from emulator
extern general_purpose_register * general_purpose_register_list[NUM_REGISTERS];
extern general_purpose_register zero_register;
extern uint64_t program_counter;
extern p_state p_state_register;

// ------------------------------------- CUSTOM HELPER FUNCTIONS -------------------------------------------------------
/*
 * With a provided key, this function will loop through the list of registers and return a pointer towards
 * the register with its id equal to the  key
*/
general_purpose_register* find_register(uint32_t key) {
    if (key == ZERO_REGISTER_ID) {
        return &zero_register;
    }

    for (int i = 0; i < NUM_REGISTERS; i++) {
        if (general_purpose_register_list[i]->id == key) {
            return general_purpose_register_list[i];
        }
    }
    return NULL;
}

/*
 * A helper function to parse instructions to determine which of the 4 available operations to proceed with
 * the function will take the two operands for the operation and write the result to the destination register
 * if an update to the pState register is required, the flags will also be updated within this function.
 *
 * Note: this helper is only used when bit-width access is 64-bit
*/
void arithmetic_helper_64(general_purpose_register *rd, uint32_t instruction, int64_t rn_val, int64_t op2) {
    // case: opc (29-30)
    switch (get_bits(instruction, OPC_START, OPC_END)) {
        case ADD:
            write_64(rd, rn_val + op2);
            break;
        case ADDS: {
            int64_t res = rn_val + op2;
            write_64(rd, res);

            // updates pState flag
            p_state_register.negative_condition_flag = get_bit_register64(res, 63);

            // updates pState zero condition flag
            p_state_register.zero_condition_flag = res == 0;

            // updates pState carry condition flag
            p_state_register.carry_condition_flag = (uint64_t) res < (uint64_t) rn_val
                                                && (uint64_t) res < (uint64_t) op2;

            // updates pState sign overflow condition flag
            p_state_register.overflow_condition_flag = ((op2 > 0) && rn_val > (INT64_MAX - op2))
                                                   || ((op2 < 0) && rn_val < (INT64_MIN - op2));
            break;
        }
        case SUB:
            write_64(rd, rn_val - op2);
            break;
        case SUBS: {
            int64_t res = rn_val - op2;
            write_64(rd, res);

            // updates pState flag
            p_state_register.negative_condition_flag = get_bit_register64(res, 63);

            // updates pState zero condition flag
            p_state_register.zero_condition_flag = res == 0;

            // updates pState carry condition flag
            p_state_register.carry_condition_flag = (uint64_t) rn_val >= (uint64_t) op2;

            // updates pState sign overflow condition flag
            p_state_register.overflow_condition_flag = ((op2 > 0) && rn_val < (INT64_MIN + op2))
                                                  || ((op2 < 0) && rn_val > (INT64_MAX + op2));
            break;
        }
    }
}

/*
 * A helper function to parse instructions to determine which of the 4 available operations to proceed with
 * the function will take the two operands for the operation and write the result to the destination register
 * if an update to the pState register is required, the flags will also be updated within this function.
 *
 * Note: this helper is only used when bit-width access is 32-bit
*/

void arithmetic_helper_32(general_purpose_register *rd, uint32_t instruction, int32_t rn_val, int32_t op2) {
    // case: opc (29-30)
    switch (get_bits(instruction, OPC_START, OPC_END)) {
        case ADD:
            write_32(rd, rn_val + op2);
            break;
        case ADDS: {
            int32_t res = rn_val + op2;
            write_32(rd, res);
            // updates pState flag
            p_state_register.negative_condition_flag = get_bit_register32(res, 31);
            // updates pState zero condition flag
            p_state_register.zero_condition_flag = res == 0;
            // updates pState carry condition flag
            p_state_register.carry_condition_flag = (uint32_t) res < (uint32_t) rn_val && (uint32_t) res < (uint32_t) op2;

            // updates pState sign overflow condition flag
            p_state_register.overflow_condition_flag = ((op2 > 0) && rn_val > (INT64_MAX - op2))
                                                  || ((op2 < 0) && rn_val < (INT64_MIN - op2));
            break;
        }
        case SUB:
            write_32(rd, rn_val - op2);
            break;
        case SUBS: {
            int32_t res = rn_val - op2;
            write_32(rd, res);

            // updates pState flag
            p_state_register.negative_condition_flag = get_bit_register32(res, 31);

            // updates pState zero condition flag
            p_state_register.zero_condition_flag = res == 0;

            // updates pState carry condition flag
            p_state_register.carry_condition_flag = (uint32_t) rn_val >= (uint32_t) op2;

            // updates pState sign overflow condition flag
            p_state_register.overflow_condition_flag = ((op2 > 0) && rn_val < (INT32_MIN + op2))
                                                  || ((op2 < 0) && rn_val > INT32_MAX + op2);
            break;
        }
    }
}

// ------------------------------------- Executing Operations ----------------------------------------------------------
/*
 * This function takes an instruction represented by an unsigned 32-bit integer as an input, parses it to the arithmetic
 * helpers to determine whether or how to write the arithmetically calculated values to destination registers
 */
void dpi_arithmetic(uint32_t instruction) {
    general_purpose_register *rd = find_register(get_bits(instruction, RD_START, RD_END));
    general_purpose_register *rn = find_register(get_bits(instruction, RN_START, RN_END));
    uint32_t imm12 = get_bits(instruction, IMM12_START, IMM12_END);
    // Check for sh (sign for shift)
    if (get_bits(instruction, SHIFT_BIT, SHIFT_BIT) == 1) {
        // shift by 12-bits is needed
        imm12 <<= 12;
    }

    if (get_bits(instruction, ACCESS_MODE_BIT, ACCESS_MODE_BIT) == 1) {
        // access bit-width is 64-bit
        int64_t rn_val = read_64(rn);
        arithmetic_helper_64(rd, instruction, rn_val, imm12 & ZERO_EXTEND_MASK);
        return;
    }

    if (get_bits(instruction, ACCESS_MODE_BIT, ACCESS_MODE_BIT) == 0) {
        // access bit-width is 32-bit
        int32_t rn_val = read_32(rn);
        arithmetic_helper_32(rd, instruction, rn_val, imm12);
    }
}

/*
 * This function takes an instruction represented by an unsigned 32-bit integer as an input, parses it to the arithmetic
 * helpers to determine whether or how to write the bit-wise modified values to destination registers
 */
void dpi_wide_move(uint32_t instruction) {

    general_purpose_register *rd = find_register(get_bits(instruction, 0, 4));

    uint32_t hw = get_bits(instruction, HW_START, HW_END);
    uint32_t imm16 = get_bits(instruction, IMM16_START, IMM16_END);
    uint32_t opc = get_bits(instruction, OPC_START, OPC_END);
    uint32_t sf = (instruction >> 31) & 1;
    int64_t op = (imm16 & ZERO_EXTEND_MASK) << (hw * 16);

    uint64_t shift = hw * 16;

    uint64_t mask = ~(0xFFFFULL << shift);
    uint64_t masked = ((rd -> val) & mask) | (op & (~mask));

    if (sf == 1) {
        // bit-width access is 64-bit
        switch (opc) {
            case MOVZ:
                write_64(rd, op);
                break;
            case MOVN:
                write_64(rd, ~op);
                break;
            case MOVK:
                write_64(rd, masked);
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
                write_32(rd, masked);
                break;
            default:
                break;
        }
    }
}

/*
 * This function will be called by the main function in emulate.c
 * It takes an instruction represented by an unsigned 32-bit integer as an input, and determines whether an arithmetic
 * or wide-move operation should be made based on the opi code of the bits. Then it will pass the instruction to the
 * arithmetic and wide-move parsers to further the process
 */
void execute_DPIImmediate(uint32_t instruction) {
    // determining operation
    // CASE 1: when opi is 010 -> arithmetic
    if (get_bits(instruction, OPI_START_BIT, OPI_START_BIT) == 0
    &&  get_bits(instruction, OPI_START_BIT + 1, OPI_START_BIT + 1) == 1
    &&  get_bits(instruction, OPI_START_BIT + 2, OPI_START_BIT + 2) == 0) {
        dpi_arithmetic(instruction);
    }
    // CASE 2: when opi is 101 -> wide move
    else if (get_bits(instruction, OPI_START_BIT, OPI_START_BIT) == 1
        &&  get_bits(instruction, OPI_START_BIT + 1, OPI_START_BIT + 1) == 0
        &&  get_bits(instruction, OPI_START_BIT + 2, OPI_START_BIT + 2) == 1) {
        dpi_wide_move(instruction);
    }

    // DO NOTHING for other values of opi

    // Increment Program Counter by 4
    program_counter += 4;
}
