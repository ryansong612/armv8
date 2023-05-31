#include <stdio.h>
#include <stdint.h>
#include "custombit.h"
#include "emulate.h"
#include "readnwrite.h"
#include "dpi-register.h"
#include "dpi-immediate.h"

#define M_BIT 28
#define OPR_BIT 24
#define MULTIPLY_FLAG 1
#define ARITHMETIC_FLAG 1
#define ZERO_REGISTER_BIT 31

// retrieving global variables from emulator
extern GeneralPurposeRegister generalPurposeRegisters[NUM_REGISTERS];
extern GeneralPurposeRegister zeroRegister;
extern uint64_t programCounter;
extern PSTATE pStateRegister;

// ------------------------------------- SHIFTS ---------------------------------------------------
// value manipulations (lsl, lsr, asr, ror)
// for lsl and lsr, if negative, then take the positive value before shift

// -------------- logical shift left --------------------

int64_t lsl_64(int64_t val, int shift) {
    int64_t sign_bit = get_bit_register64(val, 63);
    if (sign_bit == 1) {
        uint64_t unsigned_val = *(uint64_t*) & val;
        uint64_t res = unsigned_val << shift;
        return *(int64_t*) & res;
    } else {
        return val << shift;
    }

    // carry happened
}

int64_t lsl_32(int32_t val, int shift) {
    int32_t sign_bit = get_bit_register32(val, 31);
    if (sign_bit == 1) {
        uint32_t unsigned_val = *(uint32_t*) & val;
        uint32_t res = unsigned_val << shift;
        return *(int32_t*) & res;
    } else {
        return val << shift;
    }

    // carry happened
}

// -------------- logical shift right --------------------

int64_t lsr_64(int64_t val, int shift) {
    int64_t sign_bit = get_bit_register64(val, 63);
    if (sign_bit == 1) {
        uint64_t unsigned_val = *(uint64_t*) & val;
        uint64_t res = unsigned_val >> shift;
        return *(int64_t*) & res;
    } else {
        return val << shift;
    }

    // carry happened
}

int64_t lsr_32(int32_t val, int shift) {
    int32_t sign_bit = get_bit_register32(val, 31);
    if (sign_bit == 1) {
        uint32_t unsigned_val = *(uint32_t*) & val;
        uint32_t res = unsigned_val >> shift;
        return *(int32_t*) & res;
    } else {
        return val << shift;
    }

    // carry happened
}

// ------------- arithmetic shift right -------------------

int64_t asr_64(int64_t val, int shift) {
    // perform right shift
    // then replace the first 5 digits to 1 if sign bit = 1

    int64_t sign_bit = get_bit_register64(val, 63);
    int64_t result = lsr_64(val, shift);

    if (sign_bit == 1) {
        for (int i = 63; i > 58; i--) {
            int64_t mask = 1LL << i;
            result |= mask;
        }
    }

    return result;

    // carry happened
}

int32_t asr_32(int32_t val, int shift) {
    // perform right shift
    // then replace the first 5 digits to 1 if sign bit = 1

    int32_t sign_bit = get_bit_register32(val, 31);
    int32_t result = lsr_32(val, shift);

    if (sign_bit == 1) {
        for (int i = 63; i > 58; i--) {
            int32_t mask = 1LL << i;
            result |= mask;
        }
    }

    return result;

    // carry happened
}

// ------------ right rotate ------------------------

int64_t ror_64(int64_t val, int shift) {
    // perform right shift
    // then take the carried bits and replace the first shifted bits to those

    int64_t result = lsr_64(val, shift);

    for (int i = 0; i < shift; i++) {
        // if the value is 1, we will replace the 0 at that position to 1
        // if the value is 0, just keep 0

        if (get_bit_register64(val, i) == 1) {
            int64_t mask = 1LL << (63 - i);
            result |= mask;
        }
    }

    return result;
}

int32_t ror_32(int32_t val, int shift) {
    // perform right shift
    // then take the carried bits and replace the first shifted bits to those

    int32_t result = lsr_32(val, shift);

    for (int i = 0; i < shift; i++) {
        // if the value is 1, we will replace the 0 at that position to 1
        // if the value is 0, just keep 0

        if (get_bit_register32(val, i) == 1) {
            int32_t mask = 1LL << (31 - i);
            result |= mask;
        }
    }

    return result;
}

// ------------------------------------- CUSTOM HELPER FUNCTIONS ---------------------------------------------------
GeneralPurposeRegister* find_register(uint32_t key) {
    for (int i = 0; i < NUM_REGISTERS; i ++) {
        if (generalPurposeRegisters[i].id == key) {
            return &generalPurposeRegisters[i];
        }
    }
    return NULL;
}

int64_t shift_64(uint32_t instruction, int64_t rm_val, uint32_t shift) {
    // case: shift (22-23)
    switch (get_bits(instruction, 22, 23)) {
        case 0b00:
            return lsl_64(rm_val, shift);
        case 0b01:
            return lsr_64(rm_val, shift);
        case 0b10:
            return asr_64(rm_val, shift);
        case 0b11:
            return ror_64(rm_val, shift);
    }
}

int32_t shift_32(uint32_t instruction, int32_t rm_val, uint32_t shift) {
    // case : shift (22-23)
    switch (get_bits(instruction, 22, 23)) {
        case 0b00:
            return lsl_32(rm_val, shift);
        case 0b01:
            return lsr_32(rm_val, shift);
        case 0b10:
            return asr_32(rm_val, shift);
        case 0b11:
            return ror_32(rm_val, shift);
    }
}

// ------------------------------------- Executing Operations ----------------------------------------------------

void dpi_register_arithmetic(uint32_t instruction) {
    GeneralPurposeRegister *rd = find_register(get_bits(instruction, 0, 4));
    GeneralPurposeRegister *rn = find_register(get_bits(instruction, 5, 9));
    GeneralPurposeRegister *rm = find_register(get_bits(instruction, 16, 20));

    // amount of shift needed (operand)
    uint32_t shift = get_bits(instruction, 10, 15);


    // 64 bit register
    if (match_bits(instruction, 1, 31)) {
        int64_t rm_val = read_64(rm);
        int64_t rn_val = read_64(rn);

        // perform shifts
        int64_t op2 = shift_64(instruction, rm_val, shift);

        arithmetic_helper_64(rd, instruction, rn_val, op2);
    }

    // 32 bit register
    if (match_bits(instruction, 0, 31)) {
        int32_t rm_val = read_32(rm);
        int32_t rn_val = read_32(rn);

        int32_t op2 = shift_32(instruction, rm_val, shift);

        // case: opc (29-30)
        arithmetic_helper_32(rd, instruction, rn_val, op2);
    }

    return;
}

void dpi_register_logic(uint32_t instruction) {
    GeneralPurposeRegister *rd = find_register(get_bits(instruction, 0, 4));
    GeneralPurposeRegister *rn = find_register(get_bits(instruction, 5, 9));
    GeneralPurposeRegister *rm = find_register(get_bits(instruction, 16, 20));

    // amount of shift needed (operand)
    uint32_t shift = get_bits(instruction, 10, 15);

    // 64 bit register
    if (match_bits(instruction, 1, 31)) {
        int64_t rm_val = read_64(rm);
        int64_t rn_val = read_64(rn);

        // perform shifts
        int64_t op2 = shift_64(instruction, rm_val, shift);

        switch (get_bits(instruction, 29, 30)) {
            case 0b00:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise AND
                    write_64(rd, rn_val & op2);
                } else {
                    // bitwise bit clear
                    write_64(rd, rn_val & ~op2);
                }
                break;
            case 0b01:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise incl. OR
                    write_64(rd, rn_val | op2);
                } else {
                    // bitwise incl. OR NOT
                    write_64(rd, rn_val | ~op2);
                }
                break;
            case 0b10:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise excl. OR NOT
                    write_64(rd, rn_val ^ ~op2);
                } else {
                    // bitwise excl. OR
                    write_64(rd, rn_val ^ op2);
                }
                break;
            case 0b11:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise AND + setting flags
                    write_64(rd, rn_val & op2);
                } else {
                    // bitwise bit clear + setting flags
                    write_64(rd, rn_val & ~op2);
                }

                // updating pState flag
                // reads the result after operation
                int64_t res = read_64(rd);

                // updates negative cond. flag
                pStateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pState zero cond. flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry cond. flag
                pStateRegister.carryConditionFlag = 0;
                // updates pState sign overflow cond. flag
                pStateRegister.overflowConditionFlag = 0;
                break;
        }
    }

    // 32 bit register
    if (match_bits(instruction, 0, 31)) {
        int64_t rm_val = read_32(rm);
        int64_t rn_val = read_32(rn);

        // perform shifts
        int64_t op2 = shift_32(instruction, rm_val, shift);

        switch (get_bits(instruction, 29, 30)) {
            case 0b00:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise AND
                    write_32(rd, rn_val & op2);
                } else {
                    // bitwise bit clear
                    write_32(rd, rn_val & ~op2);
                }
                break;
            case 0b01:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise incl. OR
                    write_32(rd, rn_val | op2);
                } else {
                    // bitwise incl. OR NOT
                    write_32(rd, rn_val | ~op2);
                }
                break;
            case 0b10:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise excl. OR NOT
                    write_32(rd, rn_val ^ ~op2);
                } else {
                    // bitwise excl. OR
                    write_32(rd, rn_val ^ op2);
                }
                break;
            case 0b11:
                if (match_bits(instruction, 0, 21)) {
                    // bitwise AND + setting flags
                    write_32(rd, rn_val & op2);
                } else {
                    // bitwise bit clear + setting flags
                    write_32(rd, rn_val & ~op2);
                }

                // updating pState flag
                // reads the result after operation
                int64_t res = read_32(rd);

                // updates negative cond. flag
                pStateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pState zero cond. flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry cond. flag
                pStateRegister.carryConditionFlag = 0;
                // updates pState sign overflow cond. flag
                pStateRegister.overflowConditionFlag = 0;
                break;
        }
    }

    return;
}

void dpi_register_multiply(uint32_t instruction) {
    GeneralPurposeRegister *rd = find_register(get_bits(instruction, 0, 4));
    GeneralPurposeRegister *rn = find_register(get_bits(instruction, 5, 9));
    GeneralPurposeRegister *ra = find_register(get_bits(instruction, 10, 14));
    GeneralPurposeRegister *rm = find_register(get_bits(instruction, 16, 20));

    // 64 bit register
    if (match_bits(instruction, 1, 31)) {
        int64_t rn_val = read_64(rn);
        int64_t ra_val = read_64(ra);
        int64_t rm_val = read_64(rm);

        // 11111 encodes 0
        if (ra_val == ZERO_REGISTER_BIT) {
            ra_val = 0;
        }

        if (match_bits(instruction, MULADD, 15)) {
            // multiply add
            write_64(rd, ra_val + (rn_val * rm_val));
        }

        if (match_bits(instruction, MULSUB, 15)){
            // multiply sub
            write_64(rd, ra_val - (rn_val * rm_val));
        }
    }

    // 32 bit register
    if (match_bits(instruction, 0, 31)) {
        int64_t rn_val = read_32(rn);
        int64_t ra_val = read_32(ra);
        int64_t rm_val = read_32(rm);

        // 11111 encodes 0
        if (ra_val == ZERO_REGISTER_BIT) {
            ra_val = 0;
        }

        if (match_bits(instruction, 0, 15)) {
            // multiply add
            write_32(rd, ra_val + (rn_val * rm_val));
        } else {
            // multiply sub
            write_32(rd, ra_val - (rn_val * rm_val));
        }
    }

}


// Assuming all registers have their bit arrays reversed
    bool execute_DPIRegister(uint32_t instruction) {
        // determining operation

        // CASE 1: when M is 1 = multiply
        // CASE 2: when M is 0
        //   - SUB CASE 1: if OPR 1xx0 = arithmetic
        //   - SUB CASE 0: if OPR 0xxx = logic
        if (match_bits(instruction, MULTIPLY_FLAG, M_BIT)) {
            dpi_register_multiply(instruction);
        } else {
            if (match_bits(instruction, ARITHMETIC_FLAG, OPR_BIT)) {
                dpi_register_arithmetic(instruction);
            } else {
                dpi_register_logic(instruction);
            }
        }

        // Increment Program Counter by 1
        programCounter++;
        return true;
    }
