#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "emulate.h"
#include "custombit.h"
#include "readnwrite.h"
#include "dpi-immediate.h"
#include "parseLS.h"

// Global variables
extern general_purpose_register * general_purpose_register_list[NUM_REGISTERS];
extern general_purpose_register zero_register;
extern uint64_t program_counter;
extern p_state p_state_register;

// Takes numBytesToAccess number of bytes from memory starting from the address
// and loads this 4-byte (32 bits) or 8-byte number (64 bits) on to the register.
void load_to_register(int num_byte_to_access, uint8_t *memory, int64_t address, general_purpose_register *target_register) {
    printf("loading\n");
    uint64_t total = 0;
    for (int i = 0; i < num_byte_to_access; i++) {
        total += ((uint64_t) memory[address + i]) << (i * 8);
    }

    if (num_byte_to_access == BYTES_IN_X_MODE_REGISTER) {
        write_64(target_register, total);
    } else {
        write_32(target_register, total);
    }
}

// Takes numBytesToAccess number of bytes from target register and 
// stores this 4-byte (32 bits) or 8-byte number (64 bits) to the memory
// starting from the address.
void store_to_memory(int num_byte_to_access, uint8_t *memory, int64_t address, general_purpose_register *target_register) {
    printf("storing\n");
    // what's the difference between read_64 and read_32?
    if (num_byte_to_access == BYTES_IN_X_MODE_REGISTER) {
        for (int i = 0; i < num_byte_to_access; i++) {
            memory[address + i] = (uint8_t) (read_64(target_register) >> (i * 8));
        }
    } else {
        for (int i = 0; i < num_byte_to_access; i++) {
            memory[address + i] = (uint8_t) (read_32(target_register) >> (i * 8));
        }
    }
}

// Calculate address by adding the PC and the offset (simm19 ∗ 4)
bool load_literal(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int num_byte_to_access) {
    printf("load literal");
    // gets address PC + simms19 * 4
    uint32_t simms19 = get_bits(instruction, 5, 23) * 4;
    int64_t offset = extend_sign_bit(simms19, 19); // sign extend it
    uint64_t address = program_counter + offset;

    int64_t total = 0;
    // Access memory to get value
    for (int i = 0; i < num_byte_to_access; i++) {
        total += ((uint64_t) memory[address + i]) << (i * 8);
    }

    // writes this to the target register
    if (num_byte_to_access == BYTES_IN_X_MODE_REGISTER) {
        write_64(target_register, total);
    } else {
        write_32(target_register, total);
    }

    return true;
}

bool unsigned_immediate_offset(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load) {
    printf("Unsigned Immediate offset is running\n");
    uint64_t imm12 = get_bits(instruction, 10, 21) * 8;
    printf("%lu\n", imm12);

    // Check validity of arguments
    if (target_register->mode) {
        if (imm12 > 32760) {
            return false;
        }
    } else {
        if (imm12 > 16380) {
            return false;
        }
    }
    printf("%i", xn);      
    int64_t address = read_64(find_register(xn)) + imm12;

    printf("%ld\n", address);

    // Access this address in memory and write this to target register
    if (load) {
        load_to_register(num_byte_to_access, memory, address, target_register);
    } else {
        store_to_memory(num_byte_to_access, memory, address, target_register);
    }
    return true;
}

bool pre_or_post_indexed(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load, bool pre) {
    printf("pre-index\n");

    // Calculates value of address
    int32_t simm9 = extend_sign_bit(get_bits(instruction, 12, 20), 9);
    printf("%d\n", simm9);
    int64_t value = read_64(find_register(xn));
    int64_t address = value + simm9;

    int64_t memory_address = 0;
    if (pre) {
        memory_address = address;
    } else {
        memory_address = value;
    }

    //writes back to register
    if (num_byte_to_access == BYTES_IN_X_MODE_REGISTER) {
        write_64(find_register(xn), address);
    } else {
        write_32(find_register(xn), address);
    }

    // Access this address in memory and write this to target register
    if (load) {
        load_to_register(num_byte_to_access, memory, memory_address, target_register);
    } else {
        store_to_memory(num_byte_to_access, memory, memory_address, target_register);
    }

    return true;
}

// Calculate the address by
bool registerOffset(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load) {
    printf("register offset\n");
    // Add the two values of registers
    int8_t xm = get_bits(instruction, 16, 20);

    if (!find_register(xm)->mode) {
        printf("target register has wrong mode!");
        return false;
    }

    int64_t address = find_register(xm)->val + find_register(xn)->val;

    // Access this address in memory and write this to target register
    if (load) {
        load_to_register(num_byte_to_access, memory, address, target_register);
    } else {
        store_to_memory(num_byte_to_access, memory, address, target_register);
    }

    return true;
}

bool execute_DTI(uint8_t *memory, uint32_t instruction) {
    // get target register
    uint32_t target = get_bits(instruction,0, 4);
    general_purpose_register *target_register = general_purpose_register_list[target];

    // Determine whether it is load or store
    bool load = false;
    if (get_bits(instruction, 22, 22) == 1) {
        load = true;
    }

    // Determine the number of bytes to access
    int num_byte_to_access;
    if (target_register->mode) {
        num_byte_to_access = 8;
    } else {
        num_byte_to_access = 4;
    }

    bool success;

    // Determine type of data transfer
    // It is a load literal
    if (get_bits(instruction, 31, 31) == 0) {
        success = load_literal(memory, instruction, target_register, num_byte_to_access);
    }
    // It is a single data transfer
    else {
        int8_t xn = get_bits(instruction, 5, 9);
        if (get_bits(instruction, 24, 24) != 0) { // U = 1
            success = unsigned_immediate_offset(memory, instruction, target_register, xn, num_byte_to_access, load);
        } else if (get_bits(instruction, 21, 21) != 0) { // L = 1
            success = registerOffset(memory, instruction, target_register, xn, num_byte_to_access, load);
        } else {
            if (get_bits(instruction, 11, 11) == 1) {
                success = pre_or_post_indexed(memory, instruction, target_register, xn, num_byte_to_access, load, true);
            } else {
                success = pre_or_post_indexed(memory, instruction, target_register, xn, num_byte_to_access, load, false);
            }
        }
    }
    // Modify PC
    program_counter += 4;

    return success;
}
