#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "emulate.h"
#include "../BitUtils/custombit.h"

// returns the val of gpr in 64-bit
int64_t read_64(general_purpose_register *gpr) {
    if (gpr -> id == 31) {
        // reads from the Zero Register always return zero
        return 0;
    }
    return gpr -> val;
}

// returns the val of gpr in 32-bit after truncation
int32_t read_32(general_purpose_register *gpr) {
    if (gpr -> id == 31) {
        // reads from the Zero Register always return zero
        return 0;
    }
    return (int32_t) gpr -> val;
}

// writes a 64-bit int to the designated gpr
// returns false if attempted to write 64-bit to 32-bit accessed register
bool write_64(general_purpose_register *gpr, int64_t num) {
    if (!gpr -> mode) {
        // bit-width access is 32-bit, unable to write
        return false;
    }
    if (gpr -> id == 31) {
        // writes to Zero Register are ignored
        return false;
    }
    gpr -> val = num;
    return true;
}

// writes a 32-bit int to the designated gpr
bool write_32(general_purpose_register *gpr, int32_t num) {
    if (gpr -> id == 31) {
        // writes to Zero Register are ignored
        return false;
    }
    gpr -> val = num & 0xFFFFFFFFLL;
    return true;
}
