#include <stdint.h>
#include "emulate.h"

#ifndef ARMV8_32_READNWRITE_H
#define ARMV8_32_READNWRITE_H

int64_t read_64(GeneralPurposeRegister *gpr);
int32_t read_32(GeneralPurposeRegister *gpr);
bool write_64(GeneralPurposeRegister *gpr, int64_t num);
bool write_32(GeneralPurposeRegister *gpr, int32_t num);


#endif //ARMV8_32_READNWRITE_H
