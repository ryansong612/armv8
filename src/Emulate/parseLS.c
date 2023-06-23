#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "emulate.h"
#include "../BitUtils/custombit.h"
#include "readnwrite.h"
#include "dpi-immediate.h"
#include "parseLS.h"

// Global variables
extern general_purpose_register * general_purpose_register_list[NUM_REGISTERS];
extern general_purpose_register zero_register;
extern uint64_t program_counter;
extern p_state p_state_register;

/*
    Takes numBytesToAccess number of bytes from memory starting from the address
    and loads this 4-byte (32 bits) or 8-byte number (64 bits) on to the register.
*/ 
void load_to_register(int num_byte_to_access, uint8_t *memory, int64_t address, general_purpose_register *target_register) {
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

/*
    Takes numBytesToAccess number of bytes from target register and 
    stores this 4-byte (32 bits) or 8-byte number (64 bits) to the memory
    starting from the address.
*/
void store_to_memory(int num_byte_to_access, uint8_t *memory, int64_t address, general_purpose_register *target_register) {
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

/*
    Calculate address by adding the PC and the offset (simm19 ∗ 4). Then saves num_byte_to_access number of bytes
    starting from address into the target register.    
*/ 
void load_literal(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int num_byte_to_access) {
    // gets address PC + simms19 * 4
    uint32_t simms19 = get_bits(instruction, 5, 23) * 4;
    int64_t offset = extend_sign_bit(simms19, 19); // sign extend it
    uint64_t address = program_counter + offset;

    // Access memory to get value
    int64_t total = 0;
    for (int i = 0; i < num_byte_to_access; i++) {
        total += ((uint64_t) memory[address + i]) << (i * 8);
    }

    // writes this to the target register
    if (num_byte_to_access == BYTES_IN_X_MODE_REGISTER) {
        write_64(target_register, total);
    } else {
        write_32(target_register, total);
    }
}

/*
    Calculate address by adding the value stored in xn and the unsigned offset imm12. Then saves num_byte_to_access number of bytes
    starting from address into the target register.
*/ 
void unsigned_immediate_offset(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load) {
    uint64_t imm12 = get_bits(instruction, 10, 21) * 8;

    // Check validity of arguments
    if (target_register->mode) {
        assert(imm12 <= 32760);
    } else {
        assert(imm12 <= 16380);
    }

    int64_t address = read_64(find_register(xn)) + imm12;

    // Access this address in memory and write this to target register
    if (load) {
        load_to_register(num_byte_to_access, memory, address, target_register);
    } else {
        store_to_memory(num_byte_to_access, memory, address, target_register);
    }
}

/*
    If it is pre-indexed, calculate address by adding the value stored in xn and the signed offset simm9.
    If it is post-indexed, address is given by the value stored in xn.
    Then, it saves num_byte_to_access number of bytes starting from address to target_register.
*/
void pre_or_post_indexed(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load, bool pre) {
    // Calculates value of address
    int32_t simm9 = extend_sign_bit(get_bits(instruction, 12, 20), 9);
    int64_t value = read_64(find_register(xn));
    int64_t address = value + simm9;

    // Set memory_address depending on pre/post-indexed
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
}

/*
    Calculate the address by adding value stored in xn and value stored in xm.
    Then, it saves num_byte_to_access number of bytes starting from address to target_register.
*/ 
void registerOffset(uint8_t *memory, uint32_t instruction, general_purpose_register *target_register, int8_t xn, int num_byte_to_access, bool load) {
    // Add the two values of registers
    int8_t xm = get_bits(instruction, 16, 20);

    assert(find_register(xm)->mode);

    int64_t address = find_register(xm)->val + find_register(xn)->val;

    // Access this address in memory and write this to target register
    if (load) {
        load_to_register(num_byte_to_access, memory, address, target_register);
    } else {
        store_to_memory(num_byte_to_access, memory, address, target_register);
    }
}

/*
    Entry point from emulate. Parses the instructions and invokes the functions above
    depending on the instruction.
*/ 
void execute_DTI(uint8_t *memory, uint32_t instruction) {
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

    // Determine type of data transfer
    // It is a load literal
    if (get_bits(instruction, 31, 31) == 0) {
        load_literal(memory, instruction, target_register, num_byte_to_access);
    }
    // It is a single data transfer
    else {
        int8_t xn = get_bits(instruction, 5, 9);
        if (get_bits(instruction, 24, 24) != 0) { // U = 1
            unsigned_immediate_offset(memory, instruction, target_register, xn, num_byte_to_access, load);
        } else if (get_bits(instruction, 21, 21) != 0) { // L = 1
            registerOffset(memory, instruction, target_register, xn, num_byte_to_access, load);
        } else {
            if (get_bits(instruction, 11, 11) == 1) {
                pre_or_post_indexed(memory, instruction, target_register, xn, num_byte_to_access, load, true);
            } else {
                pre_or_post_indexed(memory, instruction, target_register, xn, num_byte_to_access, load, false);
            }
        }
    }
    // Modify PC
    program_counter += 4;
}
