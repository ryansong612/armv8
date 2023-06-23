#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "emulate.h"
#include "../BitUtils/custombit.h"
#include "dpi-immediate.h"
#include "dpi-register.h"
#include "parseLS.h"
#include "parseBranches.h"
#include "readnwrite.h"

#define MAX_FILE_SIZE 2097152 // 2MB
#define NOP 3573751839 // No operation instruction
#define HALT 2315255808 // Termination instruction
#define NUM_REGISTERS 31

// ------------------------------------------------- READ + PARSING FILES ----------------------------------------------

// reads binary file
void read_file(char *dst, uint8_t *memory) {
    FILE *ptr = fopen(dst, "rb");
    if (ptr == NULL) {
        perror("File does not exist.\n");
        exit(139);
    }

    fread(memory, 1, MAX_FILE_SIZE, ptr);
    fclose(ptr);
}

// ---------------------------------------------- INITIALIZING REGISTERS -----------------------------------------------

general_purpose_register *general_purpose_register_list[NUM_REGISTERS];
general_purpose_register zero_register = { .id = NUM_REGISTERS,
        .val = 0,
        .mode = true,
        .zeroRegisterFlag = true,
        .programCounterFlag = false };
uint64_t program_counter = 0;
p_state p_state_register = { .negative_condition_flag = false,
                          .zero_condition_flag = true,
                          .carry_condition_flag = false,
                          .overflow_condition_flag = false };

void initialize_registers(void) {
    for (int8_t i = 0; i < 31; i++) {
        general_purpose_register new_register = { .id = i,
                .val = 0,
                .mode = true,
                .zeroRegisterFlag = false,
                .programCounterFlag = false };
        general_purpose_register *gpr = malloc(sizeof(general_purpose_register));
        *gpr = new_register;
        general_purpose_register_list[i] = gpr;
    }
}

/*
 * Emulator loop. Takes an instruction with consists of 4 bytes of memory, and send to different functions based on
 * op0. The instructions can be categorised into DPI Immediate, DPI Register, Loads and Stores and Branches. Special
 * Instructions include HALT, which terminates the emulator and NOP, which just moves on to the next instruction.
 */
void emulate(uint8_t *memory) {
    while (true) {
        uint32_t current_instruction = 0;
        for (int i = 0; i < 4; i++ ) {
            current_instruction += ((uint32_t) memory[program_counter + i]) << (i * 8);
        }
        // Special Instructions
        if (current_instruction == HALT) {
            return;
        }
        if (current_instruction == NOP) {
            program_counter += 4;
            continue;
        }
        // Split into 4 cases: DPImmediate, DPRegister, Loads and Stores, Branches
        uint32_t op0 = get_bits(current_instruction, 25, 28);
        switch(get_bits(op0, 1, 3)) {
            case 4:
                printf("dpi-immediate\n");
                execute_DPIImmediate(current_instruction);
                break;
            case 5:
                printf("branches\n");
                execute_branches(current_instruction);
                break;
            default:
                if (get_bits(op0, 2, 2) == 1 && get_bits(op0, 0, 0) == 0) {
                    printf("dti\n");
                    execute_DTI(memory, current_instruction);
                } else {
                    printf("register\n");
                    execute_DPIRegister(current_instruction);
                }
        }
    }
}
// -------------------------------------------------- TERMINATION ------------------------------------------------------
/*
 * Prints the output of the emulator into the file specified by the output_path.
 */
void terminate(uint8_t *memory, char *output_path) {
    // Create file
    FILE *out = fopen(output_path, "w");

    // print all register values
    fputs("Registers:\n", out);
    for (int i = 0; i < NUM_REGISTERS; i++) {
        general_purpose_register *current_register = general_purpose_register_list[i];

        // Mode
        if (current_register->mode) {
            fputs("X", out);
        } else {
            fputs("W", out);
        }

        // Name
        char name[6];
        sprintf(name, "%i", i);
        if (i < 10) {
            fputs("0", out);
        }
        fputs(name, out);

        fputs(" = ", out);

        char str[17];
        sprintf(str, "%016lx", read_64(general_purpose_register_list[i]));
        fputs(str, out); putc( '\n', out);
    }
    
    // print out PC
    fputs("PC = ", out);
    char pc[17];
    sprintf(pc, "%016lx", program_counter);
    fputs(pc, out); putc( '\n', out);

    // print out PSTATE
    fputs("PSTATE: ", out);
    if (p_state_register.negative_condition_flag) {
        fputs("N", out);
    } else {
        fputs("-", out);
    }
    if (p_state_register.zero_condition_flag) {
        fputs("Z", out);
    } else {
        fputs("-", out);
    }
    if (p_state_register.carry_condition_flag) {
        fputs("C", out);
    } else {
        fputs("-", out);
    }
    if (p_state_register.overflow_condition_flag) {
        fputs("V", out);
    } else {
        fputs("-", out);
    }
    fputs("\n", out);

    // print out all the rest of the memory
    fputs("Non-zero memory:\n", out);
    for (int i = 0; i < MAX_FILE_SIZE; i += 4) {
        int32_t aligned = 0;
        for (int j = i; j < i + 4; j++) {
            aligned |= memory[j] << ((j - i) * 8);
        }
        if (aligned != 0) {
            char name[8];
            sprintf(name, "0x%x: ", i);
            fputs(name, out);
            char str[64];
            sprintf(str, "0x%x", aligned);
            fputs(str, out); putc( '\n', out);
        }
    }
    fclose(out);
}

int main(int argc, char **argv) {
    char *output_path = argv[2];
    // Global variables
    uint8_t *memory = (uint8_t *) calloc(MAX_FILE_SIZE, sizeof(uint8_t));
    if (memory == NULL) {
        perror("Memory not allocated.\n");
        exit(139);
    }

    // read the file
    read_file(argv[1], memory);

    // Run the emulator
    initialize_registers();
    emulate(memory);
    terminate(memory, output_path);

    // Free all allocated memories
    free(memory);
    for (int8_t i = 0; i < NUM_REGISTERS; i++) {
        free(general_purpose_register_list[i]);
    }

    return EXIT_SUCCESS;
}
