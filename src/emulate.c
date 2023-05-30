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
#define NUM_REGISTERS 31

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

GeneralPurposeRegister generalPurposeRegisters[NUM_REGISTERS];
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

uint32_t read_register_32(GeneralPurposeRegister *gpr) {
    uint32_t val = (*gpr).val;
    return val;
}

uint64_t read_register_64(GeneralPurposeRegister *gpr) {
    return (*gpr).val;
}

void write_register_with32(GeneralPurposeRegister *gpr, uint32_t v) {
    (*gpr).val = v;
}

void write_register_with64(GeneralPurposeRegister *gpr, uint64_t v) {
    (*gpr).val = v;
}

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

void dpi_arithmetic(uint32_t instruction) {
    GeneralPurposeRegister *gpr;
    uint32_t imm12 = get_bits(instruction, 10, 21);
    // Check for sh (sign for shift)
    if (match_bits(instruction, 1, 22)) {
        // shift by 12-bits is needed
        imm12 <<= 12;
    }

    if (match_bits(instruction, 1, 31)) {
        // access bit-width is 64-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with64(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with64(gpr, (*gpr).val + imm12);
                do something;
                break;
            case 2:
                write_register_with64(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with64(gpr, (*gpr).val - imm12);
                do something;
                break;
        }
        return;
    }

    if (match_bits(instruction, 0, 31)) {
        // access bit-width is 32-bit
        switch (get_bits(instruction, 29, 30)) {
            case 0:
                write_register_with32(gpr, (*gpr).val + imm12);
                break;
            case 1:
                write_register_with32(gpr, (*gpr).val + imm12);
                do something;
                break;
            case 2:
                write_register_with32(gpr, (*gpr).val - imm12);
                break;
            case 3:
                write_register_with32(gpr, (*gpr).val - imm12);
                do something;
                break;
        }
        return;
    }


}

void dpi_wide_move(uint32_t instruction) {
    printf("Wide Move!");
}

// Assuming all registers have their bit arrays reversed
void parse_DPImmediate(uint32_t instruction) {
    // determining operation

    // CASE 1: when opi is 010 -> arithmetic
    if (match_bits(instruction, ARITHMETIC_FLAG_BITS, OPI_START_BIT)) {
        dpi_arithmetic(instruction);
    }
        // CASE 2: when opi is 101 -> wide move
    else if (match_bits(instruction, WIDE_MOVE_FLAG_BITS, OPI_START_BIT)) {
        dpi_wide_move(instruction);
    }

    // DO NOTHING for other values of opi
}

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
    readFile("src/DataFile/start.elf");
    initializeRegisters();
    bool success = emulate();
    if (success) {
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}
