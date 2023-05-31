#include <stdint.h>
#include <stdbool.h>
#include "emulate.h"

#ifndef ARMV8_32_DPI_H
#define ARMV8_32_DPI_H

#define ADD 0b00
#define ADDS 0b01
#define SUB 0b10
#define SUBS 0b11
#define MOVZ 0b00
#define MOVN 0b10
#define MOVK 0b11

#define ZERO_REGISTER_ID 11111

bool execute_DPIImmediate(uint32_t instruction);
void arithmetic_helper_64(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, uint32_t op2);
void arithmetic_helper_32(GeneralPurposeRegister *rd, uint32_t instruction, int32_t rn_val, uint32_t op2);

#endif //ARMV8_32_DPI_H
