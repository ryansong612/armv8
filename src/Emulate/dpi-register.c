#include <stdint.h>

#include "../BitUtils/custombit.h"
#include "emulate.h"
#include "readnwrite.h"
#include "dpi-register.h"
#include "dpi-immediate.h"

#define OPI_START_BIT 23
#define M_BIT 28
#define N_BIT 21
#define OPR_BIT 24
#define MULTIPLY_FLAG 1
#define ARITHMETIC_FLAG 1
#define ACCESS_MODE_BIT 31
#define OPC_START 29
#define OPC_END 30

// shifts
#define LSL 0
#define LSR 1
#define ASR 2

// opc cases
#define OPC00 0
#define OPC01 1
#define OPC10 2
#define OPC11 3

// operations
#define AND 0
#define BIC 1
#define ORR 0
#define ORN 1
#define EOR 0
#define EON 1
#define ANDS 0
#define BICS 1
#define MULADD 0

// retrieving global variables from emulator
extern general_purpose_register *general_purpose_register_list[NUM_REGISTERS];
extern general_purpose_register zero_register;
extern uint64_t program_counter;
extern p_state p_state_register;

// ------------------------------------- SHIFTS ---------------------------------------------------
// value manipulations (lsl, lsr, asr, ror)

// -------------- logical shift left --------------------

int64_t lsl_64(int64_t val, uint32_t shift) {
    int64_t sign_bit = get_bit_register64(val, 63);
    if (sign_bit == 1) {
        uint64_t unsigned_val = (uint64_t) val;
        uint64_t res = unsigned_val << shift;
        return (int64_t) res;
    } else {
        return val << shift;
    }
    // carry happened
}

int32_t lsl_32(int32_t val, uint32_t shift) {
    int32_t sign_bit = get_bit_register32(val, 31);
    if (sign_bit == 1) {
        uint32_t unsigned_val = (uint32_t) val;
        uint32_t res = unsigned_val << shift;
        return (int32_t) res;
    } else {
        return val << shift;
    }
    // carry happened
}

// -------------- logical shift right --------------------

int64_t lsr_64(int64_t val, uint32_t shift) {
    int64_t sign_bit = get_bit_register64(val, 63);
    if (sign_bit == 1) {
        uint64_t unsigned_val = (uint64_t) val;
        uint64_t res = unsigned_val >> shift;
        return (int64_t) res;
    } else {
        return val >> shift;
    }

    // carry happened
}

int32_t lsr_32(int32_t val, uint32_t shift) {
    int32_t sign_bit = get_bit_register32(val, 31);
    if (sign_bit == 1) {
        uint32_t unsigned_val = (uint32_t) val;
        uint32_t res = unsigned_val >> shift;
        return (int32_t) res;
    } else {
        return val >> shift;
    }
    // carry happened
}

// ------------- arithmetic shift right -------------------

int64_t asr_64(int64_t val, uint32_t shift) {
    // perform right shift
    // then replace the shifted 0 to 1
    int64_t result = lsr_64(val, shift);
    int64_t sign_bit = get_bit_register64(val, 63);
    int64_t mask = 0;
    if (sign_bit) {
        mask = -1LL << (64 - shift);
    }
    return result | mask;
    // carry happened
}

int32_t asr_32(int32_t val, uint32_t shift) {
    // perform right shift
    // then replace the shifted 0 to 1
    int32_t result = lsr_32(val, shift);
    int64_t sign_bit = get_bit_register64(val, 31);
    int64_t mask = 0;
    if (sign_bit) {
        mask = -1 << (32 - shift);
    }
    return result | mask;
    // carry happened
}

// ------------ right rotate ------------------------

int64_t ror_64(int64_t val, uint32_t shift) {
    // perform right shift
    // then take the carried bits and replace the first shifted bits to those
    int64_t carry = get_bits64(val, 0, shift);
    int64_t result = lsr_64(val, shift);
    return result | (carry << (64 - shift));
}

int32_t ror_32(int32_t val, uint32_t shift) {
    // perform right shift
    // then take the carried bits and replace the first shifted bits to those
    int32_t carry = get_bits(val, 0, shift);
    int32_t result = lsr_32(val, shift);
    return result | (carry << (32 - shift));
}

// ------------------------------------- CUSTOM HELPER FUNCTIONS ---------------------------------------------------
int64_t shift_64(uint32_t instruction, int64_t rm_val, uint32_t shift) {
    // case: shift (22-23)
    switch (get_bits(instruction, 22, 23)) {
        case LSL:
            return lsl_64(rm_val, shift);
        case LSR:
            return lsr_64(rm_val, shift);
        case ASR:
            return asr_64(rm_val, shift);
        default:
            return ror_64(rm_val, shift);
    }
}

int32_t shift_32(uint32_t instruction, int32_t rm_val, uint32_t shift) {
    // case : shift (22-23)
    switch (get_bits(instruction, 22, 23)) {
        case LSL:
            return lsl_32(rm_val, shift);
        case LSR:
            return lsr_32(rm_val, shift);
        case ASR:
            return asr_32(rm_val, shift);
        default:
            return ror_32(rm_val, shift);
    }
}

// ------------------------------------- Executing Operations ----------------------------------------------------

void dpi_register_arithmetic(uint32_t instruction) {
    general_purpose_register *rd = find_register(get_bits(instruction, 0, 4));
    general_purpose_register *rn = find_register(get_bits(instruction, 5, 9));
    general_purpose_register *rm = find_register(get_bits(instruction, 16, 20));
    // amount of shift needed (operand)
    uint32_t shift = get_bits(instruction, 10, 15);

    if (get_bit_register32(instruction, ACCESS_MODE_BIT) == 1) {
        // 64 bit register
        int64_t rm_val = read_64(rm);
        int64_t rn_val = read_64(rn);

        // perform shifts
        int64_t op2 = shift_64(instruction, rm_val, shift);
        // perform arithmetic operations
        arithmetic_helper_64(rd, instruction, rn_val, op2);
    } else {
        // 32 bit register
        int32_t rm_val = read_32(rm);
        int32_t rn_val = read_32(rn);

        // perform shifts
        int32_t op2 = shift_32(instruction, rm_val, shift);
        // perform arithmetic operations
        arithmetic_helper_32(rd, instruction, rn_val, op2);
    }
}

void dpi_register_logic(uint32_t instruction) {
    general_purpose_register *rd = find_register(get_bits(instruction, 0, 4));
    general_purpose_register *rn = find_register(get_bits(instruction, 5, 9));
    general_purpose_register *rm = find_register(get_bits(instruction, 16, 20));

    // amount of shift needed (operand)
    uint32_t shift = get_bits(instruction, 10, 15);

    if (get_bit_register32(instruction, ACCESS_MODE_BIT) == 1) {
        // 64 bit register
        int64_t rm_val = read_64(rm);
        int64_t rn_val = read_64(rn);

        // perform shifts
        int64_t op2 = shift_64(instruction, rm_val, shift);

        // case of opc(29-30)
        switch (get_bits(instruction, OPC_START, OPC_END)) {
            case OPC00:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case AND:
                        // bitwise AND
                        write_64(rd, (rn_val & op2));
                        break;
                    case BIC:
                        // bitwise bit clear
                        write_64(rd, rn_val & ~op2);
                        break;
                }
                break;
            case OPC01:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case ORR:
                        // bitwise incl. OR
                        write_64(rd, rn_val | op2);
                        break;
                    case ORN:
                        // bitwise excl. OR NOT
                        write_64(rd, rn_val | ~op2);
                        break;
                }
                break;
            case OPC10:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case EOR:
                        // bitwise excl. OR NOT
                        write_64(rd, rn_val ^ op2);
                        break;
                    case EON:
                        // bitwise excl. OR
                        write_64(rd, rn_val ^ ~op2);
                        break;
                }
                break;
            case OPC11: {
                int64_t res;
                switch (get_bit_register32(instruction, N_BIT)) {
                    case ANDS: {
                        res = rn_val & op2;
                        // bitwise AND + setting flags
                        write_64(rd, res);
                        break;
                    }
                    case BICS: {
                        res = rn_val & ~op2;
                        // bitwise bit clear + setting flags
                        write_64(rd, res);
                        break;
                    }
                }

                // updates negative cond. flag
                p_state_register.negative_condition_flag = get_bit_register64(res, 63);
                // updates pState zero cond. flag
                p_state_register.zero_condition_flag = res == 0;
                // updates pState carry cond. flag
                p_state_register.carry_condition_flag = 0;
                // updates pState sign overflow cond. flag
                p_state_register.overflow_condition_flag = 0;
                break;
            }
        }
    } else {
        // 32 bit register
        int32_t rm_val = read_32(rm);
        int32_t rn_val = read_32(rn);

        // perform shifts
        int32_t op2 = shift_32(instruction, rm_val, shift);

        // case of opc(29-30)
        switch (get_bits(instruction, OPC_START, OPC_END)) {
            case OPC00:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case AND:
                        // bitwise AND
                        write_32(rd, rn_val & op2);
                        break;
                    case BIC:
                        // bitwise bit clear
                        write_32(rd, rn_val & ~op2);
                        break;
                }
                break;
            case OPC01:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case ORR:
                        // bitwise incl. OR
                        write_32(rd, rn_val | op2);
                        break;
                    case ORN:
                        // bitwise excl. OR NOT
                        write_32(rd, rn_val | ~op2);
                        break;
                }
                break;
            case OPC10:
                switch (get_bit_register32(instruction, N_BIT)) {
                    case EOR:
                        // bitwise excl. OR NOT
                        write_32(rd, rn_val ^ op2);
                        break;
                    case EON:
                        // bitwise excl. OR
                        write_32(rd, rn_val ^ ~op2);
                        break;
                }
                break;
            case OPC11: {
                int32_t res;
                switch (get_bit_register32(instruction, N_BIT)) {
                    case ANDS: {
                        res = rn_val & op2;
                        // bitwise AND + setting flags
                        write_32(rd, res);
                        break;
                    }
                    case BICS: {
                        res = rn_val & ~op2;
                        // bitwise bit clear + setting flags
                        write_32(rd, res);
                        break;
                    }
                }

                // updates negative cond. flag
                p_state_register.negative_condition_flag = get_bit_register32(res, 31);
                // updates pState zero cond. flag
                p_state_register.zero_condition_flag = res == 0;
                // updates pState carry cond. flag
                p_state_register.carry_condition_flag = 0;
                // updates pState sign overflow cond. flag
                p_state_register.overflow_condition_flag = 0;
                break;
            }
        }
    }
}

void dpi_register_multiply(uint32_t instruction) {
    general_purpose_register *rd = find_register(get_bits(instruction, 0, 4));
    general_purpose_register *rn = find_register(get_bits(instruction, 5, 9));
    general_purpose_register *ra = find_register(get_bits(instruction, 10, 14));
    general_purpose_register *rm = find_register(get_bits(instruction, 16, 20));

    if (get_bit_register32(instruction, ACCESS_MODE_BIT) == 1) {
        // 64 bit register
        int64_t rn_val = read_64(rn);
        int64_t ra_val = read_64(ra);
        int64_t rm_val = read_64(rm);

        if (get_bit_register32(instruction, 15) == MULADD) {
            // multiply add
            write_64(rd, ra_val + (rn_val * rm_val));
        } else {
            // multiply sub
            write_64(rd, ra_val - (rn_val * rm_val));
        }

    } else {
        // 32 bit register
        int32_t rn_val = read_32(rn);
        int32_t ra_val = read_32(ra);
        int32_t rm_val = read_32(rm);

        if (get_bit_register32(instruction, 15) == MULADD) {
            // multiply add
            write_32(rd, ra_val + (rn_val * rm_val));
        } else {
            // multiply sub
            write_32(rd, ra_val - (rn_val * rm_val));
        }
    }
}


// Assuming all registers have their bit arrays reversed
void execute_DPIRegister(uint32_t instruction) {
    // determining operation

    // CASE 1: when M is 1 = multiply
    // CASE 2: when M is 0
    //   - SUB CASE 1: if OPR 1xx0 = arithmetic
    //   - SUB CASE 0: if OPR 0xxx = logic
    if (get_bit_register32(instruction, M_BIT) == MULTIPLY_FLAG) {
        dpi_register_multiply(instruction);
    } else {
        if (get_bit_register32(instruction, OPR_BIT) == ARITHMETIC_FLAG) {
            dpi_register_arithmetic(instruction);
        } else {
            dpi_register_logic(instruction);
        }
    }

    // Increment Program Counter
    program_counter += 4;
}
