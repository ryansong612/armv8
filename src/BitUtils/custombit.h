#include <stdbool.h>
#include <stdint.h>

#ifndef ARMV8_32_CUSTOMBIT_H
#define ARMV8_32_CUSTOMBIT_H

#define BASE_16 16
#define BASE_10 10

int64_t get_bit_register64(int64_t num, int idx);
int32_t get_bit_register32(int32_t num, int idx);
uint32_t get_bits(uint32_t num, int start, int end);
int64_t get_bits64(int64_t num, int start, int end);
int64_t extend_sign_bit(uint32_t num, int num_bits);
void print_binary(uint64_t num);
uint32_t shrink32(uint32_t num, int num_bits);

#endif //ARMV8_32_CUSTOMBIT_H
