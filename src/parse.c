#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#define MAX_INSTRUCTIONS_LENGTH 1048576

// Global variables
uint8_t memory[MAX_INSTRUCTIONS_LENGTH];
uint32_t *instructions;

typedef struct {
    uint8_t id; // id of register (0-30), if out of range, it is special
    uint64_t val; // bits stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;

// for debugging purposes
void printBinary(uint32_t value) {
    // Determine the number of bits in uint64_t
    int numBits = sizeof(uint32_t) * 8;

    // Print each bit from left to right
    for (int i = numBits - 1; i >= 0; i--) {
        uint32_t mask = 1U << i; // Create a bitmask for the current bit position

        // Check if the bit is set (1) or unset (0)
        if (value & mask)
            printf("1");
        else
            printf("0");
    }

    printf("\n");
}

// reads binary file
void readFile(char *dst) {
    FILE *ptr;
    ptr = fopen(dst, "rb");

    // Determine the file size
    fseek(ptr, 0, SEEK_END);
    long fileSize = ftell(ptr);
    fseek(ptr, 0, SEEK_SET); // moves pointer back to front of file

    // reads the file to memory byte by byte
    fread(memory, 1, fileSize, ptr);

    // Chunking them into instructions (4 bytes)
    size_t numInstructions = fileSize / sizeof(uint32_t);
    instructions = (uint32_t *)memory;

    /*
    // Debugging
    for (size_t i=0; i < numInstructions; i++) {
        printf("%zu:", i);
        printBinary(instructions[i]);
    }
    */
}

int main(int argc, char **argv) {
    // read the file
    readFile(argv[0]);

    return EXIT_SUCCESS;
}
