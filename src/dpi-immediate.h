#include <stdint.h>
#include <stdbool.h>
#include "emulate.h"

#ifndef ARMV8_32_DPI_IMMEDIATE_H
#define ARMV8_32_DPI_IMMEDIATE_H

void execute_DPIImmediate(uint32_t instruction);
general_purpose_register * find_register(uint32_t key);
void arithmetic_helper_64(general_purpose_register *rd, uint32_t instruction, int64_t rn_val, int64_t op2);
void arithmetic_helper_32(general_purpose_register *rd, uint32_t instruction, int32_t rn_val, int32_t op2);

#endif //ARMV8_32_DPI_H
