#include <stdint.h>
#include <stdbool.h>

#include "parseBranches.h"
#include "readnwrite.h"
#include "emulate.h"
#include "../BitUtils/custombit.h"

#define BRANCH_TYPE_START 29
#define BRANCH_TYPE_END 31
#define UNCONDITIONAL_OFFSET_START 0
#define UNCONDITIONAL_OFFSET_END 25
#define XN_START 5
#define XN_END 9
#define CONDITIONAL_OFFSET_START 5
#define CONDITIONAL_OFFSET_END 23
#define CONDITION_START 0
#define CONDITION_END 3
#define UNCONDITIONAL 0
#define REGISTER 6
#define CONDITIONAL 2
#define EQ 0
#define NE 1
#define GE 10
#define LT 11
#define GT 12
#define LE 13
#define AL 14

// Retrieve global variables
extern general_purpose_register *general_purpose_register_list[NUM_REGISTERS];
extern general_purpose_register zero_register;
extern uint64_t program_counter;
extern p_state p_state_register;

/*
 * Executes the unconditional type branch instruction that is indicated by the first 3 bits of the instruction being
 * 000. This applies an offset of simm26 * 4 (after sign-extending it to a 64-bit number) to the program counter.
 */
void execute_branch_unconditional(uint32_t instruction) {
    int64_t offset = extend_sign_bit(get_bits(instruction,
                                            UNCONDITIONAL_OFFSET_START,
                                            UNCONDITIONAL_OFFSET_END) * 4,
                                   26);
    program_counter += offset;
}

/*
 * Executes the register type branch instruction that is indicated by the first 3 bits of the instruction being
 * 110. This sets the program counter to the address contained in the register that is encoded in Xn.
 */
void execute_branch_register(uint32_t instruction) {
    program_counter = read_64(general_purpose_register_list[get_bits(instruction, XN_START, XN_END)]);
}

/*
 * Executes the conditional type branch instruction that is indicated by the first 3 bits of the instruction being
 * 010. This applies an offset of simm19 * 4 (after sign-extending it to a 64-bit number) to the program counter, if
 * the condition enconded in cond is satisfied. Otherwise, just increment the program counter by 4.
 */
void execute_branch_conditional(uint32_t instruction) {
    uint32_t cond = get_bits(instruction, CONDITION_START, CONDITION_END);

    bool to_execute = false;
    switch (cond) {
        case EQ:
            to_execute = p_state_register.zero_condition_flag == 1;
            break;
        case NE:
            to_execute = p_state_register.zero_condition_flag == 0;
            break;
        case GE:
            to_execute = p_state_register.negative_condition_flag == p_state_register.overflow_condition_flag;
            break;
        case LT:
            to_execute = p_state_register.negative_condition_flag != p_state_register.overflow_condition_flag;
            break;
        case GT:
            to_execute = p_state_register.zero_condition_flag == 0
                         && p_state_register.negative_condition_flag == p_state_register.overflow_condition_flag;
            break;
        case LE:
            to_execute = !(p_state_register.zero_condition_flag == 0
                           && p_state_register.negative_condition_flag == p_state_register.overflow_condition_flag);
            break;
        case AL:
            to_execute = true;
            break;
        default:
            to_execute = false;
    }
    if (to_execute) {
        int64_t offset = extend_sign_bit(get_bits(instruction,
                                                CONDITIONAL_OFFSET_START,
                                                CONDITIONAL_OFFSET_END) * 4,
                                       19);
        program_counter += offset;
    } else {
        program_counter += 4;
    }
}

void execute_branches(uint32_t instruction) {
    switch (get_bits(instruction, BRANCH_TYPE_START, BRANCH_TYPE_END)) {
        case UNCONDITIONAL:
            execute_branch_unconditional(instruction);
            break;
        case REGISTER:
            execute_branch_register(instruction);
            break;
        case CONDITIONAL:
            execute_branch_conditional(instruction);
            break;
    }
}
