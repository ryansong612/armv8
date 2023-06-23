#include <stdint.h>
#include <stdbool.h>
#include "emulate.h"

#ifndef ARMV8_32_READNWRITE_H
#define ARMV8_32_READNWRITE_H

int64_t read_64(general_purpose_register *gpr);
int32_t read_32(general_purpose_register *gpr);
bool write_64(general_purpose_register *gpr, int64_t num);
bool write_32(general_purpose_register *gpr, int32_t num);


#endif //ARMV8_32_READNWRITE_H
