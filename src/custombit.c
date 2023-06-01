#include <stdint.h>
#include <stdbool.h>

// only works for registers
int64_t get_bit_register64(int64_t num, int idx) {
    return (num >> idx) & 1;
}

int32_t get_bit_register32(int32_t num, int idx) {
    return (num >> idx) & 1;
}

// only works for instructions
uint32_t get_bits(uint32_t num, int start, int end) {
    uint32_t v = ((num >> start) & 1) << 1;
    start++;
    while (start <= end) {
        v += ((num >> start) & 1);
        v <<= 1;
        start++;
    }
    return v;
}

// only works for register
int64_t get_bits64(int64_t num, int start, int end) {
    int64_t v = ((num >> start) & 1) << 1;
    start++;
    while (start <= end) {
        v += ((num >> start) & 1);
        v <<= 1;
        start++;
    }
    return v;
}

// returns true if and only if num's start-end bits are the same as tgt
// pre: tgt is not 0
bool match_bits(uint64_t num, int64_t tgt, int idx) {
    return (((num >> idx) & tgt)) == tgt;
}

// extends a numBits-bit number to a 64-bit number, preserving the sign
int64_t extendSignBit(uint32_t num, int numBits) {
    // If sign bit is zero, just cast normally
    if (get_bit_register32(num, numBits - 1) == 0) {
        return (uint64_t) num;
    }
    // sign bit is 1
    uint64_t extended = (uint64_t) num;
    uint64_t mask = 1LL << numBits;
    for (int i = numBits; i < 64; i++) {
        extended = extended | mask;
        mask <<= 1LL;
    }
    return extended;
}