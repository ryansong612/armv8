#include <stdint.h>
#include <stdio.h>

// only works for registers
int64_t get_bit_register64(int64_t num, int idx) {
    return (num >> idx) & 1;
}

int32_t get_bit_register32(int32_t num, int idx) {
    return (num >> idx) & 1;
}

// only works for instructions
uint32_t get_bits(uint32_t num, int start, int end) {
    uint32_t mask = ((1L << (end - start + 1)) - 1) << start;
    return (num & mask) >> start;
}

// only works for register
int64_t get_bits64(int64_t num, int start, int end) {
    int64_t mask = ((1LL << (end - start + 1)) - 1) << start;
    return (num & mask) >> start;
}

// extends a num_bits-bit number to a 64-bit number, preserving the sign
int64_t extend_sign_bit(uint32_t num, int num_bits) {
    int32_t signed_num = num;
    // If sign bit is zero, just cast normally
    if (get_bit_register32(signed_num, num_bits - 1) == 0) {
        return num;
    }
    // sign bit is 1
    int64_t extended = num;
    int64_t mask = 1LL << num_bits;
    for (int i = num_bits; i < 64; i++) {
        extended = extended | mask;
        mask <<= 1LL;
    }
    return extended;
}

// for debugging purposes
void print_binary(int64_t number) {
    // Iterate over each bit of the number
    for (int i = 63; i >= 0; i--) {
        // Right shift the number by 'i' bits and perform bitwise AND with 1
        int64_t bit = (number >> i) & 1;
        printf("%lx", bit);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void print_instruction(uint32_t number) {
    // Iterate over each bit of the number
    for (int i = 31; i >= 0; i--) {
        // Right shift the number by 'i' bits and perform bitwise AND with 1
        uint32_t bit = (number >> i) & 1;
        printf("%x", bit);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

// Shrinks a number expressed in 32-bits to a number expressed in num_bits bits. This is the opposite of extend_sign_bit
uint32_t shrink32(uint32_t num, int num_bits) {
    uint32_t mask = (1 << num_bits) - 1;
    return num & mask;
}
