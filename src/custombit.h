#ifndef ARMV8_32_CUSTOMBIT_H
#define ARMV8_32_CUSTOMBIT_H

int64_t get_bit_register64(int64_t num, int idx);
int32_t get_bit_register32(int32_t num, int idx);
uint32_t get_bits(uint32_t num, int start, int end);
int64_t get_bits64(int64_t num, int start, int end);
bool match_bits(uint64_t num, int64_t tgt, int idx);


#endif //ARMV8_32_CUSTOMBIT_H
