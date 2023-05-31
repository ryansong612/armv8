#include <stdbool.h>
#include <stdint.h>
#include "emulate.h"
#include "custombit.h"

// Global variables
extern GeneralPurposeRegister* generalPurposeRegisters[NUM_REGISTERS];
extern GeneralPurposeRegister zeroRegister;
extern uint64_t programCounter;
extern PSTATE pStateRegister;
extern uint8_t memory[MAX_FILE_SIZE];

void load_literal(uint32_t instruction, GeneralPurposeRegister *targetRegister, int numBytesToAccess) {
    // gets address PC + simms19 * 4
    int32_t simms19 = get_bits(instruction, 5, 23) >> 5;
    int64_t offset = extendSignBit(simms19 * 4, 32); // sign extend it
    int64_t address = programCounter + offset;

    int64_t value = 0;

    // Access 4 bytes memory
    for (int i = 0; i < numBytesToAccess; i++) {
        value += memory[address + i] << (i * 8);
    }

    // writes this to the target register
    (*targetRegister).val = value;
}

void unsigned_immediate_offset(uint32_t instruction, GeneralPurposeRegister *targetRegister, int8_t xn, int numBytesToAccess) {
    uint64_t imm12 = get_bits(instruction, 10, 21) >> 10;
    int64_t address = (*generalPurposeRegisters[xn]).val + imm12;

    for (int i = 0; i < numBytesToAccess; i++) {
        memory[address + i] = (*targetRegister).val >> (i * 8);
    }
}

void pre_indexed(uint32_t instruction, GeneralPurposeRegister *targetRegister, int8_t xn, int numBytesToAccess) {
    // Calculates value of address
    int32_t simm9 = get_bits(instruction, 12, 20) >> 12;
    int64_t value = (*generalPurposeRegisters[xn]).val;
    int64_t address = value + simm9;

    //writes back to register
    (*generalPurposeRegisters[xn]).val = address;

    // Access this address in memory and write this to target register
    for (int i = 0; i < numBytesToAccess; i++) {
        // Access using updated value
        memory[address + i] = (*targetRegister).val >> (i * 8);
    }
}


void post_indexed(uint32_t instruction, GeneralPurposeRegister *targetRegister, int8_t xn, int numBytesToAccess) {
    // Calculates value of address
    int32_t simm9 = get_bits(instruction, 12, 20) >> 12;
    int64_t value = (*generalPurposeRegisters[xn]).val;
    int64_t address = value + simm9;

    //writes back to register
    (*generalPurposeRegisters[xn]).val = address;

    // Access this address in memory and write this to target register
    for (int i = 0; i < numBytesToAccess; i++) {
        // Access using original value
        memory[value + i] = (*targetRegister).val >> (i * 8);
    }
}

void registerOffset(uint32_t instruction, GeneralPurposeRegister *targetRegister, int8_t xn, int numBytesToAccess) {
    // Add the two values of registers
    int8_t xm = get_bits(instruction, 16, 20) >> 16;
    int64_t address = (*generalPurposeRegisters[xm]).val + (*generalPurposeRegisters[xn]).val;

    // Access this address in memory and write this to target register
    for (int i = 0; i < numBytesToAccess; i++) {
        // Access using original value
        memory[address + i] = (*targetRegister).val >> (i * 8);
    }
}

void singleDTI(uint32_t instruction) {
    // get target register
    uint32_t target = get_bits(instruction,0, 4);
    GeneralPurposeRegister *targetRegister = generalPurposeRegisters[target];

    int numBytesToAccess;
    if ((*targetRegister).mode) {
        numBytesToAccess = 7;
    } else {
        numBytesToAccess = 4;
    }

    // Determine type of data transfer
    // It is a load literal
    if (get_bits(instruction, 31, 31) == 0) {
        load_literal(instruction, targetRegister, numBytesToAccess);
    }
        // It is a single data transfer
    else {
        int8_t xn = get_bits(instruction, 5, 9) >> 5;
        if (get_bits(instruction, 24, 24) != 0) { // U = 1
            unsigned_immediate_offset(instruction, targetRegister, xn, numBytesToAccess);
        } else if (get_bits(instruction, 22, 22) != 0) { // L = 1
            registerOffset(instruction, targetRegister, xn, numBytesToAccess);
        } else {
            if (get_bits(instruction, 11, 11) == 1) {
                pre_indexed(instruction, targetRegister, xn, numBytesToAccess);
            } else {
                post_indexed(instruction, targetRegister, xn, numBytesToAccess);
            }
        }
    }

    // Modify PC
    programCounter += 1;
}
