#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define WIDE_MOVE_FLAG_BITS 5
#define ARITHMETIC_FLAG_BITS 2
#define OPI_START_BIT 23
#define MAX_FILE_SIZE 1048576 // 2^20 bytes
#define NOP 3573751839 // No operation instruction
#define HALT 2315255808 // Termination instruction

// Global variables
uint8_t memory[MAX_FILE_SIZE];
uint32_t *instructions;

typedef struct {
    int8_t id; // id of register (0-32), 32 for zero register
    int64_t val; // value stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;

// ---------------------------- READ + PARSING FILES ---------------------------
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

// ----------------------- INITIALIZING REGISTERS --------------------------

GeneralPurposeRegister generalPurposeRegisters[31];
GeneralPurposeRegister zeroRegister = { .id = 31, .val = 0, .zeroRegisterFlag = true, .programCounterFlag = false };
uint64_t programCounter = 0;
PSTATE pStateRegister = { .carryConditionFlag = false,
                          .negativeConditionFlag = false,
                          .overflowConditionFlag = false,
                          .zeroConditionFlag = false };

void initializeRegisters(void) {
    for (int i = 0; i < 31; i++) {
        GeneralPurposeRegister newRegister = { .id = i, .val = 0, .zeroRegisterFlag = false, .programCounterFlag = false };
        generalPurposeRegisters[i] = newRegister;
    }
}
// ----------------------- UTILS --------------------------------------------
uint32_t get_bit(uint32_t num, int idx) {
    return ((num >> idx) & 1);
}

uint32_t get_bits(uint32_t num, int start, int end) {
    uint32_t v = ((num >> start) & 1) << 1;
    start++;
    while (start <= end) {
        v += ((num >> start) & 1);
        v = v << 1;
        start++;
    }
    return v;
}

// returns true if and only if num's start-end bits are the same as tgt
// pre: tgt is not 0
bool match_bits(uint64_t num, uint64_t tgt, int idx) {
    return (((num >> idx) & tgt)) == tgt;
}

// ------------------------------ PARSING INSTRUCTIONS --------------------------------------------
bool emulate(void) {
    while (true) {
        uint32_t currentInstruction = instructions[programCounter / 4];
        // Special Instructions
        if (currentInstruction == HALT) {
            return true;
        }
        if (currentInstruction == NOP) {
            programCounter += 4;
            continue;
        }
        // Split into 4 cases: DPImmediate, DPRegister, Loads and Stores, Branches
        uint32_t op0 = get_bits(currentInstruction, 25, 28);
        switch(get_bits(op0, 1, 3)) {
            case 4:
                parse_DPImmediate(currentInstruction);
                break;
            case 5:
                parse_DPRegister(currentInstruction);
                break;
            default:
                if (get_bit(op0, 2) == 1 && get_bit(op0, 0) == 0) {
                    parseLS(currentInstruction);
                } else if (get_bit(op0, 2) == 1 && get_bit(op0, 1) == 0 && get_bit(op0, 0) == 1) {
                    parseBranches(currentInstruction);
                } else {
                    return false;
                }
        }
    }
}

int main(int argc, char **argv) {
    // read the file
    // readFile(argv[0]); CHANGEEEEE
    readFile("src/DataFile/start.elf");
    initializeRegisters();
    bool success = emulate();
    if (success) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
