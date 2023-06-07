#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <regex.h>

#define SF 30
#define U 24
#define L 22
#define OFFSET 10
#define XN 5
#define MB 1000000

regex_t unsigned_regex;
regex_t pre_index_regex;
regex_t post_index_regex;
regex_t register_regex;

extern dynmap symbol_table;

struct singleDTI_IR {
    int load;
    int sf;
    int unsigned_offset_flag;
    uint32_t offset;
    uint32_t xn;
    uint32_t rt;
};

typedef struct singleDTI_IR *singleDTI_IR;

struct load_literal_IR {
    int sf;
    uint32_t rt;
    uint32_t offset;
};

typedef struct load_literal_IR *load_literal_IR;

void initialize_regex() {
    int return_value;

    return_value = regcomp(&unsigned_regex, "\[x[0-3][0-9]?(, #[[0-9]*)?]", REG_EXTENDED);
    if (return_value) {
        printf("Cannot compile unsigned regex.\n");
        exit(EXIT_FAILURE);
    }

    return_value = regcomp(&pre_index_regex, "\[x[0-3][0-9]?, #[0-9]*]!", REG_EXTENDED);
    if (return_value) {
        printf("Cannot compile pre-index regex.\n");
        exit(EXIT_FAILURE);
    }

    return_value = regcomp(&post_index_regex, "\[x[0-3][0-9]?], #[0-9]*", REG_EXTENDED);
    if (return_value) {
        printf("Cannot compile post-index regex.\n");
        exit(EXIT_FAILURE);
    }

    return_value = regcomp(&register_regex, "\[x[0-3][0-9]?, x[0-3][0-9]?(, lsl #[0-9]*)?]", REG_EXTENDED);
    if (return_value) {
        printf("Cannot compile register regex.\n");
        exit(EXIT_FAILURE);
    }
}

void print_binary(int64_t number) {
    // Iterate over each bit of the number
    for (int i = 63; i >= 0; i--) {
        // Right shift the number by 'i' bits and perform bitwise AND with 1
        int64_t bit = (number >> i) & 1;
        printf("%lx", bit);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void print_struct(singleDTI_IR struct_ptr) {
    // Iterate over each bit of the number
    printf("Load: %i\n", struct_ptr->load);
    printf("Target register is 64 bit: %i\n", struct_ptr->sf);
    printf("Unsigned immediate: %i\n", struct_ptr->unsigned_offset_flag);
    printf("Offset is: ");
    print_binary(struct_ptr->offset);
    printf("xn: %i\n", struct_ptr->xn);
    printf("target register: %i\n", struct_ptr->rt);
}


uint32_t convert_signed_to_unsigned(int32_t num) {
    uint32_t output = 0;
    for (int i = 31; i >= 0; i--) {
        int bit = (num >> i) & 1;
        output += (bit << i);
    }
    print_binary(output);
    return output;
}

uint32_t load_literal_assemble(load_literal_IR struct_ptr) {
    uint32_t output = 0b00011000 << (32 - 8);
    if (struct_ptr->sf) {
        output += 1 << SF;
    }
    output += struct_ptr->offset << XN;
    output += struct_ptr->rt;
    return output;
}

void load_literal(load_literal_IR struct_ptr, char* target_register, char* address, uint32_t program_counter) {
    if (target_register[0] == 'x') {
        struct_ptr->sf = 1;
    } else {
        struct_ptr->sf = 0;
    }

    struct_ptr->rt = atoi(target_register + 1);

    uint32_t load_address;
    if (address[0] == '#') {
        load_address = atoi(address + 1);
    } else {
        // It's a label
        load_address = dynmap_get(symbol_table, address);
    }

    assert(abs(load_address - program_counter) < MB && load_address % 4 == 0);
    struct_ptr->offset = convert_signed_to_unsigned((load_address - program_counter) / 4);
}

// ------------------------ SINGLE DTI ----------------------------------------

void pre_index(singleDTI_IR struct_ptr, char *address) {
    printf("Address: %s\n", address);
    address[strlen(address) - 1] = '\0';

    struct_ptr->unsigned_offset_flag = 0;

    uint32_t simm9;
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = atoi(tok + 1);
    tok = strtok(NULL, ",] ");
    simm9 = convert_signed_to_unsigned(atoi(tok + 1));

    struct_ptr->offset = (simm9 << 2) + (1 << 1) + 1;
}

void post_index(singleDTI_IR struct_ptr, char *address) {
    printf("Address: %s\n", address);

    struct_ptr->unsigned_offset_flag = 0;

    uint32_t simm9;
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = atoi(tok + 1);
    printf("%s", tok);

    tok = strtok(NULL, ", ");
    printf("%s", tok);
    simm9 = convert_signed_to_unsigned(atoi(tok + 1));

    struct_ptr->offset = (simm9 << 2) + 1;
}

void register_offset(singleDTI_IR struct_ptr, char *address) {
    // unsigned
    struct_ptr->unsigned_offset_flag = 0;

    // xn
    char *tok = strtok(address, "[,]");
    printf("%s\n", tok);
    struct_ptr->xn = atoi(tok + 1);

    // offset
    tok = strtok(NULL, ",] ");
    printf("%s\n", tok);
    uint32_t xm = atoi(tok + 1);

    struct_ptr->offset = 0b10000011010 + (xm << 6);
}

void unsigned_offset(singleDTI_IR struct_ptr, char *address) {
    printf("Address: %s\n", address);

    // unsigned
    struct_ptr->unsigned_offset_flag = 1;

    // xn
    char *tok = strtok(address, "[,]");
    struct_ptr->xn = atoi(tok + 1);

    // offset
    if ((tok = strtok(NULL, ",] "))) {
        uint32_t imm12 = atoi(tok + 1);
        printf("%i\n", imm12);
        if (struct_ptr->sf) {
            struct_ptr->offset = convert_signed_to_unsigned(imm12 / 8);
        } else {
            struct_ptr->offset = convert_signed_to_unsigned(imm12 / 4);
        }

    } else {
        struct_ptr->offset = 0;
    }
}

uint32_t singleDTI_construct(singleDTI_IR struct_ptr) {
    uint32_t output = 0b1011100000 << (32 - 10);

    // sf = 1 if target register is 64-bit
    if (struct_ptr->sf) {
        output += 1ULL << SF;
    }

    // U is not set since it is not unsigned immediate
    if (struct_ptr->unsigned_offset_flag) {
        output += 1ULL << U;
    }

    // L = 1 if it's a load
    if (struct_ptr->load) {
        output += 1ULL << L;
    }

    output += struct_ptr->offset << OFFSET;
    output += struct_ptr->xn << XN;
    output += struct_ptr->rt;

    print_binary(output);

    return output;
}


void assemble_single_DTI(char *assembly_instruction, uint32_t program_counter) {
    // Converting char * into char[]
    unsigned long n = strlen(assembly_instruction);
    char assembly_instruction_arr[n + 1];
    for (int i = 0; i <= n; i++) {
        assembly_instruction_arr[i] = assembly_instruction[i];
    }

    // Parse instruction into opcode, register and <address>
    int load = 0;
    char *target_register = malloc(sizeof(char) * 4);
    char *address = malloc(sizeof(char) * 256);
    char *tok = strtok(assembly_instruction_arr, " ");
    for (int i = 0; i < 3; i++ ) {
        if (i == 0) {
            if (strcmp(tok, "ldr") == 0) {
                load = 1;
            } else {
                load = 0;
            }
            tok = strtok(NULL, " ");
        } else if (i == 1) {
            tok[strlen(tok) - 1] = '\0';
            memcpy(target_register, tok, strlen(tok)+1);
            tok = strtok(NULL, "");
        } else {
            memcpy(address, tok, strlen(tok)+1);
        }
    }

    // Parse the address to determine address mode
    if (address[0] != '[') {
        load_literal_IR instruction_struct;
        load_literal_IR struct_ptr = &instruction_struct;
        load_literal(struct_ptr, target_register, address, program_counter);
        print_binary(load_literal_assemble(struct_ptr));
    } else {
        singleDTI_IR instruction_struct;
        singleDTI_IR struct_ptr = &instruction_struct;

        // Set the load flag
        if (load) {
            printf("load is one\n");
            struct_ptr->load = 1;
        } else {
            struct_ptr->load = 0;
        }

        // Set the target_register mode flag
        if (target_register[0] == 'x') {
            struct_ptr->sf = 1;
        } else {
            struct_ptr->sf = 0;
        }

        // Set the target register
        struct_ptr->rt = atoi(target_register + 1);

        // Pre-index
        if (!regexec(&pre_index_regex, address, 0, NULL, 0)) {
            printf("Pre\n");
            pre_index(struct_ptr, address);
            print_struct(struct_ptr);
        } else if (!regexec(&post_index_regex, address, 0, NULL, 0)) {
            printf("post\n");
            post_index(struct_ptr, address);
            print_struct(struct_ptr);
        } else if (!regexec(&register_regex, address, 0, NULL, 0)) {
            printf("register\n");
            register_offset(struct_ptr, address);
            print_struct(struct_ptr);
        } else if (!regexec(&unsigned_regex, address, 0, NULL, 0)) {
            // Bug?
            printf("Unsigned\n");
            unsigned_offset(struct_ptr, address);
            print_struct(struct_ptr);
        } else {
            exit(EXIT_FAILURE);
        }
        return singleDTI_construct(struct_ptr);
    }
}