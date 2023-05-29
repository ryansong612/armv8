#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define IMM_LENGTH 12

typedef struct {
    char mode; // Either 'x' for 64, or 'w' for 32
    unsigned int id; // id of register (0-30), if out of range, it is special
    unsigned int bits[64]; // bits stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;


// imm12 is a 12-bit unsigned integer
// shifts imm12 by 12-bits
unsigned int* left_shift(unsigned int imm12[IMM_LENGTH]) {
    unsigned int *new_imm12 = malloc(sizeof(unsigned int) * IMM_LENGTH * 2);
    for (int i = IMM_LENGTH - 2; i >= 0; i--) {
        new_imm12[i] = imm12[i + 1];
    }

    for (int i = IMM_LENGTH - 1; i < IMM_LENGTH * 2; i++) {
        new_imm12[i] = 0;
    }

    return new_imm12;
}

void parseDPImmediate(GeneralPurposeRegister *gpr) {
    return;
}

int main(int argc, char **argv) {
    unsigned int bits[12] = {0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0};
    unsigned int *new_bits = left_shift(bits);
    for (int i = 0; i < IMM_LENGTH * 2; i++) {
        printf("%u ", new_bits[i]);
    }
    return 1;
}
