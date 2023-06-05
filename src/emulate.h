#ifndef ARMV8_32_EMULATE_H
#define ARMV8_32_EMULATE_H

#define NUM_REGISTERS 31

typedef struct {
    int8_t id; // id of register (0-32), 32 for zero register
    int64_t val; // value stored in the register
    bool mode; // 1 if 64-bit readable
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} general_purpose_register;

typedef struct {
    bool negative_condition_flag;
    bool zero_condition_flag;
    bool carry_condition_flag;
    bool overflow_condition_flag;
} p_state;

#endif //ARMV8_32_EMULATE_H
