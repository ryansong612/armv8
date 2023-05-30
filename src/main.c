#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint16_t a = 1234;  // Example 16-bit integer
    // 10011010010

    uint32_t result = ((uint32_t)a) << 16;  // Shift left by 16 positions
    // 100110100100000
    printf("New value:       %i\n", result);
    return 0;
}
