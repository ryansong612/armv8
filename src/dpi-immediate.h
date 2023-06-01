#include <stdint.h>
#include <stdbool.h>
#include "emulate.h"

#ifndef ARMV8_32_DPI_H
#define ARMV8_32_DPI_H

#define ADD 0
#define ADDS 1
#define SUB 2
#define SUBS 3
#define MOVZ 0
#define MOVN 2
#define MOVK 3

#define ZERO_REGISTER_ID 31

bool execute_DPIImmediate(uint32_t instruction);
GeneralPurposeRegister* find_register(uint32_t key);
void arithmetic_helper_64(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, uint32_t op2);
void arithmetic_helper_32(GeneralPurposeRegister *rd, uint32_t instruction, int32_t rn_val, uint32_t op2);

#endif //ARMV8_32_DPI_H
