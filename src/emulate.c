#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


// filepath: /Users/ryansong612/Desktop/ICL/Courses/COMP40009/ARMv8-page29-files/bootcode.bin

typedef struct {
    char mode; // Either 'x' for 64, or 'w' for 32
    unsigned int id; // id of register (0-30), if out of range, it is special
    unsigned char bits[64]; // bits stored in the register
    bool zeroRegisterFlag; // 1 if it is a zero register
    bool programCounterFlag; // 1 if it is a program counter
} GeneralPurposeRegister;

typedef struct {
    bool negativeConditionFlag;
    bool zeroConditionFlag;
    bool carryConditionFlag;
    bool overflowConditionFlag;
} PSTATE;

void readFile(char *dst) {
    unsigned char buffer[34];
    FILE *ptr;

    ptr = fopen(dst, "rb");
    fread(buffer, sizeof(buffer), 1, ptr);

    for (int i = 0, n = sizeof(buffer); i < n; i++) {
        printf("%u ", buffer[i]);
    }

    printf("\n");

    fclose(ptr);
}


int main(int argc, char **argv) {
    readFile(argv[1]);
    return 1;
}
