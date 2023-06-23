#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <regex.h>
#include <ctype.h>

#include "dynmap.h"
#include "../BitUtils/custombit.h"

#define SF 30
#define U 24
#define L 22
#define OFFSET 10
#define XN 5
#define MB 1000000

#define LOAD_LITERAL_BASE 0b00011000
#define REGISTER_OFFSET_BASE 0b100000011010
#define SINGLE_DTI_BASE 0b1011100000
#define LOAD_LITERAL_BASE_SHIFT 24
#define SINGLE_DTI_BASE_SHIFT 22

/*
    Defining internal representation (IR) structs for the two types of instructions.
*/ 
struct singleDTI_IR {
    int load; // 1 if instruction is load and 0 if instruction is store
    int sf; // 1 if target register is 64-bit and 0 if target register is 32-bit
    int unsigned_offset_flag; // 1 if the instruction is unsigned immediate
    uint32_t offset; // offset depending on the instruction
    uint32_t xn; 
    uint32_t rt;
};
typedef struct singleDTI_IR *singleDTI_IR;

struct load_literal_IR {
    int sf; // 1 if target register is 64-bit and 0 if target register is 32-bit
    uint32_t offset; // offset depending on the instruction
    uint32_t rt; 
};
typedef struct load_literal_IR *load_literal_IR;

// Global variables
extern dynmap symbol_table;
extern uint32_t program_counter;

/* 
    A helper function extracting all bits of a signed integer and turns it into
    an unsigned integer.
*/
uint32_t convert_signed_to_unsigned(int32_t num) {
    uint32_t output = 0;
    for (int i = 31; i >= 0; i--) {
        int bit = (num >> i) & 1;
        output += (bit << i);
    }
    return output;
}

// --------------------- LOAD LITERAL ------------------------------------------
/* 
    Take in the address and target_register and set the parameters of IR accordingly.
*/
void load_literal(load_literal_IR struct_ptr, char* target_register, char* address) {
    // Set sf
    if (target_register[0] == 'x') {
        struct_ptr->sf = 1;
    } else {
        struct_ptr->sf = 0;
    }

    // Set target register
    struct_ptr->rt = strtol(target_register + 1, NULL, 10);

    uint32_t load_address;
    if (address[0] == '#') { // is immediate value
        load_address = strtol(address + 1, NULL, 10);
    } else { // It's a label
        if (strchr(address, ' ') != NULL) {
            address[strchr(address, ' ') - address] = '\0';
        }
        load_address = dynmap_get(symbol_table, address);
    }

    // Set offset
    assert(abs(load_address - program_counter) < MB && load_address % 4 == 0);
    struct_ptr->offset = convert_signed_to_unsigned(shrink32((load_address - program_counter) / 4, 19));
}

/* 
    Assembling the 32-bit instruction from the IR struct for load literals.
*/
uint32_t load_literal_construct(load_literal_IR struct_ptr) {
    uint32_t output = LOAD_LITERAL_BASE << LOAD_LITERAL_BASE_SHIFT;

    output |= struct_ptr->sf << SF;
    output |= struct_ptr->offset << XN;
    output |= struct_ptr->rt;

    return output;
}

// ------------------------ SINGLE DTI ----------------------------------------
/* 
    Take in the address and set the parameters of IR accordingly when the mode is
    pre-index.
*/
void pre_index(singleDTI_IR struct_ptr, char *address) {
    // remove the ! on address
    address[strlen(address) - 1] = '\0';

    // Set unsigned offset flag
    struct_ptr->unsigned_offset_flag = 0;
    
    // Set xn
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = strtol(tok + 1, NULL, BASE_10);

    // Set offset(simm9)
    tok = strtok(NULL, ",] ");
    int32_t offset;
    if (tok[1] == '0') { 
        offset = strtol(tok + 1, NULL, BASE_16); // hex
    } else {
        offset = strtol(tok + 1, NULL, BASE_10); // decimal
    }
    uint32_t simm9 = convert_signed_to_unsigned(shrink32(offset, 9));
    struct_ptr->offset = (simm9 << 2) + (1 << 1) + 1;
}

/* 
    Take in the address and set the parameters of IR accordingly when the mode is
    post-index.
*/
void post_index(singleDTI_IR struct_ptr, char *address) {
    // Set unsigned offset flag
    struct_ptr->unsigned_offset_flag = 0;

    // Set xn
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = strtol(tok + 1, NULL, BASE_10);

    // Set offset(simm9)
    tok = strtok(NULL, ", ");
    int32_t offset;
    if (tok[1] == '0') { // #0x..
        offset = strtol(tok + 1, NULL, BASE_16);
    } else {
        offset = strtol(tok + 1, NULL, BASE_10);
    }
    uint32_t simm9 = convert_signed_to_unsigned(shrink32(offset, 9));
    struct_ptr->offset = (simm9 << 2) + 1;
}

/* 
    Take in the address and set the parameters of IR accordingly when the mode is
    register offset.
*/
void register_offset(singleDTI_IR struct_ptr, char *address) {
    // Set unsigned offset flag
    struct_ptr->unsigned_offset_flag = 0;

    // Set xn
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = strtol(tok + 1, NULL, BASE_10);

    // Set offset (xm)
    tok = strtok(NULL, ",] ");
    uint32_t xm = strtol(tok + 1, NULL, BASE_10);
    struct_ptr->offset = REGISTER_OFFSET_BASE + (xm << 6);
}

/* 
    Take in the address and set the parameters of IR accordingly when the mode is
    unsigned offset.
*/
void unsigned_offset(singleDTI_IR struct_ptr, char *address) {
    // Set unsigned offset
    struct_ptr->unsigned_offset_flag = 1;

    // Set xn
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = strtol(tok + 1, NULL, BASE_10);

    // Set offset
    if ((tok = strtok(NULL, ",] "))) {
        uint32_t imm12;
        if (tok[1] == '0') { // #0x..
            imm12 = strtol(tok + 1, NULL, BASE_16);
        } else {
            imm12 = strtol(tok + 1, NULL, BASE_10);
        }
        
        // Adjust offset depending on sf
        if (struct_ptr->sf) {
            struct_ptr->offset = convert_signed_to_unsigned(imm12 / 8);
        } else {
            struct_ptr->offset = convert_signed_to_unsigned(imm12 / 4);
        }

    } else {
        struct_ptr->offset = 0;
    }
}

/* 
    Assembling the 32-bit instruction from the IR struct for single DTIs.
*/
static uint32_t singleDTI_construct(singleDTI_IR struct_ptr) {
    uint32_t output = SINGLE_DTI_BASE << SINGLE_DTI_BASE_SHIFT;

    output |= struct_ptr->sf << SF;
    output |= struct_ptr->unsigned_offset_flag << U;
    output |= struct_ptr->load << L;
    output |= struct_ptr->offset << OFFSET;
    output |= struct_ptr->xn << XN;
    output |= struct_ptr->rt;

    return output;
}

// ------------------------------- MAIN FUNCTION ----------------------------------------
/* 
    Receives the assembly instruction from assemble.c and parses the instruction.
    It then calls the corresponding helper function defined above to convert it into 
    a 32-bit binary instruction to be outputted.
*/
uint32_t assemble_single_DTI(char *assembly_instruction) {
    // Converting char * into char[]
    unsigned long n = strlen(assembly_instruction);
    char assembly_instruction_arr[n + 1];
    strcpy(assembly_instruction_arr, assembly_instruction);

    // Parse instruction into opcode, register and <address>
    // Initializing variables
    int load = 0;
    char *target_register = malloc(sizeof(char) * 4);
    char *address = malloc(sizeof(char) * 256);

    // Parsing
    char *tok = strtok(assembly_instruction_arr, " ");
    for (int i = 0; i < 3; i++ ) {
        if (i == 0) {
            if (strcmp(tok, "ldr") == 0) {
                load = 1;
            }
        } else if (i == 1) {
            tok = strtok(NULL, ",");
            while (isspace(*tok)) tok++;
            memcpy(target_register, tok, strlen(tok)+1);
        } else {
            tok = strtok(NULL, "");
            while (isspace(*tok)) tok++;
            memcpy(address, tok, strlen(tok)+1);
        }
    }

    uint32_t output;
    // Parse the address to determine address mode
    if (address[0] != '[') { // Load literal
        load_literal_IR struct_ptr = malloc(sizeof(struct load_literal_IR));

        load_literal(struct_ptr, target_register, address);
        
        output = load_literal_construct(struct_ptr);
        free(struct_ptr);
    } else { // Single DTI
        singleDTI_IR struct_ptr = malloc(sizeof(struct singleDTI_IR));

        // Set the load flag
        struct_ptr->load = load;

        // Set the target_register mode flag
        struct_ptr->sf = target_register[0] == 'x';

        // Set the target register
        struct_ptr->rt = strtol(target_register + 1, NULL, BASE_10);

        // Parsing into single DTI
        if (address[strlen(address) - 1] == '!') {
            pre_index(struct_ptr, address);
        } else if (address[strlen(address) - 1] != ']') {
            post_index(struct_ptr, address);
        } else if (strchr(address, '#') == NULL && strchr(address, ',') != NULL) {
            register_offset(struct_ptr, address);
        } else if (strchr(address, '#') == NULL || (strchr(address, '#') != NULL && strchr(address, ',') != NULL)) {
            unsigned_offset(struct_ptr, address);
        }
        else {
            perror("Cannot match any load and store instructions!");
            exit(EXIT_FAILURE);
        }

        output = singleDTI_construct(struct_ptr);
        free(struct_ptr);
    }
    free(target_register);
    free(address);
    return output;
}