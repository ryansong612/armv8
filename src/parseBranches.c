#include <stdint.h>
#include <stdbool.h>
#include "parseBranches.h"
#include "readnwrite.h"
#include "emulate.h"
#include "custombit.h"

// Retrieve global variables
extern GeneralPurposeRegister *generalPurposeRegisters[NUM_REGISTERS];
extern GeneralPurposeRegister zeroRegister;
extern uint64_t programCounter;
extern PSTATE pStateRegister;

bool execute_branch_unconditional(uint32_t instruction) {
    // Apply offset of simm26 * 4 (sign-extended to 64-bit) to the program counter
    int64_t offset = extendSignBit(get_bits(instruction, 0, 26) << 2, 26);
    programCounter += offset;
    return 0 < programCounter < (1LL << 20);
}

bool execute_branch_register(uint32_t instruction) {
    // Sets the program counter to the specified address
    programCounter = read_64(generalPurposeRegisters[get_bits(instruction, 5, 9)]);
    return 0 < programCounter < (1LL << 20);
}

bool execute_branch_conditional(uint32_t instruction) {
    // Applies offset of simm19 * 4 (sign-extended to 64-bit) to the program counter if the PState satisfies the
    // condition specified in bits 0-3
    int32_t cond = get_bits(instruction, 0, 4);
    bool toExecute = false;
    switch (cond) {
        case 0:
            toExecute = pStateRegister.zeroConditionFlag == 1;
            break;
        case 1:
            toExecute = pStateRegister.zeroConditionFlag == 0;
            break;
        case 10:
            toExecute = pStateRegister.negativeConditionFlag == 1;
            break;
        case 11:
            toExecute = pStateRegister.negativeConditionFlag != 1;
            break;
        case 12:
            toExecute = pStateRegister.zeroConditionFlag == 0
                    && pStateRegister.negativeConditionFlag == pStateRegister.overflowConditionFlag;
            break;
        case 13:
            toExecute = !(pStateRegister.zeroConditionFlag == 0
                        && pStateRegister.negativeConditionFlag == pStateRegister.overflowConditionFlag);
            break;
        case 14:
            toExecute = true;
            break;
    }
    if (toExecute) {
        int64_t offset = extendSignBit(get_bits(instruction, 0, 19) << 2, 26);
        programCounter += offset;
    }
    return 0 < programCounter < (1LL << 20);
}

bool execute_branches(uint32_t instruction) {
    switch (get_bits(instruction, 29, 31)) {
        case 0:
            return execute_branch_unconditional(instruction);
        case 6:
            return execute_branch_register(instruction);
        case 4:
            return execute_branch_conditional(instruction);
        default:
            return false;
    }
}
