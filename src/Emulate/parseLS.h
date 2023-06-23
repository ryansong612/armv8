#ifndef ARMV8_32_PARSELS_H
#define ARMV8_32_PARSELS_H

#define MAX_FILE_SIZE 2097152
#define BYTES_IN_X_MODE_REGISTER 8
#define BYTES_IN_W_MODE_REGISTER 4

void execute_DTI(uint8_t *memory, uint32_t instruction);

#endif //ARMV8_32_PARSELS_H
