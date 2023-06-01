#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "emulate.h"
#include "custombit.h"
#include "dpi-immediate.h"
#include "dpi-register.h"
#include "parseLS.h"
#include "parseBranches.h"
#include "readnwrite.h"


#define MAX_FILE_SIZE 1048576 // 2^20 bytes
#define NOP 3573751839 // No operation instruction
#define HALT 2315255808 // Termination instruction
#define NUM_REGISTERS 31

// Global variables
uint8_t memory[MAX_FILE_SIZE];
uint32_t *instructions;

// ------------------------------------------------- READ + PARSING FILES ----------------------------------------------
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
    // size_t numInstructions = fileSize / sizeof(uint32_t);
    instructions = (uint32_t *)memory;

    /*
    // Debugging
    for (size_t i=0; i < numInstructions; i++) {
        printf("%zu:", i);
        printBinary(instructions[i]);
    }
    */
}

// ---------------------------------------------- INITIALIZING REGISTERS -----------------------------------------------

GeneralPurposeRegister *generalPurposeRegisters[NUM_REGISTERS];
GeneralPurposeRegister zeroRegister = { .id = NUM_REGISTERS,
        .val = 0,
        .zeroRegisterFlag = true,
        .programCounterFlag = false };
uint64_t programCounter = 0;
PSTATE pStateRegister = { .carryConditionFlag = false,
        .negativeConditionFlag = false,
        .overflowConditionFlag = false,
        .zeroConditionFlag = false };

void initializeRegisters(void) {
    for (int8_t i = 0; i < 31; i++) {
        GeneralPurposeRegister newRegister = { .id = i,
                .val = 0,
                .zeroRegisterFlag = false,
                .programCounterFlag = false };
        GeneralPurposeRegister *gpr = malloc(sizeof(GeneralPurposeRegister));
        *gpr = newRegister;
        generalPurposeRegisters[i] = gpr;
    }
}

bool emulate(void) {
    bool error;
    while (true) {
        error = true;
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
                error = execute_DPIImmediate(currentInstruction);
                break;
            case 5:
                error = execute_branches(currentInstruction);
                break;
            default:
                if (get_bits(op0, 2, 2) == 1 && get_bits(op0, 0, 0) == 0) {
                    error = singleDTI(currentInstruction);
                } else {
                    error = execute_branches(currentInstruction);
                }
        }
        if (!error) {
            printf("Error Detected");
        }
    }
}
// -------------------------------------------------- TERMINATION ------------------------------------------------------
void terminate(void) {
    // Create file
    FILE *out = fopen( ".out", "w" );

    // print all register values
    fputs("Registers:\n", out);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        GeneralPurposeRegister currRegister = *generalPurposeRegisters[i];

        // Mode
        if (currRegister.mode) {
            fputs("X", out);
        } else {
            fputs("W", out);
        }

        // Name
        char name[2];
        sprintf(name, "%i", i);
        if (i < 10) {
            fputs("0", out);
        }
        fputs(name, out);

        fputs(" = ", out);

        char str[16];
        sprintf(str, "%lx", read_64(generalPurposeRegisters[i]));
        fputs(str, out); putc( '\n', out);
    }
    fputs("PC = ", out);
    char pc[16];
    sprintf(pc, "%lx", programCounter);
    fputs(pc, out); putc( '\n', out);

    // print out PSTATE
    fputs("PSTATE: ", out);
    if (pStateRegister.negativeConditionFlag) {
        fputs("N", out);
    } else {
        fputs("-", out);
    }
    if (pStateRegister.zeroConditionFlag) {
        fputs("Z", out);
    } else {
        fputs("-", out);
    }
    if (pStateRegister.carryConditionFlag) {
        fputs("C", out);
    } else {
        fputs("-", out);
    }
    if (pStateRegister.overflowConditionFlag) {
        fputs("V", out);
    } else {
        fputs("-", out);
    }
    fputs("\n", out);

    // print out all the rest of the memory
    fputs("Non-zero memory:\n", out);
    for (int i = 0; i < MAX_FILE_SIZE; i++) {
        if (memory[i] != 0) {
            char name[8];
            sprintf(name, "0x%x: ", i);
            fputs(name, out);
            char str[64];
            sprintf(str, "0x%x", memory[i]);
            fputs(str, out); putc( '\n', out);
        }
    }

    fclose(out);
}

int main(int argc, char **argv) {
    // read the file
    // readFile(argv[0]); CHANGEEEEE
    readFile("src/DataFile/start.elf");
    initializeRegisters();
    //emulate();
    terminate();
    return EXIT_SUCCESS;
}
