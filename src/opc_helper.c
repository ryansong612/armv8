#include <stdio.h>
#include <stdint.h>
#include "custombit.h"
#include "emulate.h"
#include "readnwrite.h"

void arithmetic_helper_64(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, uint32_t op2) {
    // case: opc (29-30)
        switch (get_bits(instruction, 29, 30)) {
            case 0b00:
                write_64(rd, rn_val + op2);
                break;
            case 0b01:
                write_64(rd, rn_val + op2);

                // reads the result after operation
                int64_t res = read_64(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < op2) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                int64_t sign_rn_val = get_bit_register64(rn_val, 63);
                int64_t sign_op2 = get_bit_register64(op2, 63);
                int64_t sign_res = get_bit_register64(res, 63);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                       && (sign_res != sign_rn_val);
                break;
            case 0b10:
                write_64(rd, rn_val - op2);
                break;
            case 0b11:
                write_64(rd, rn_val - op2);
                // reads the result after operation
                res = read_64(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register64(res, 63);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < op2) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                sign_rn_val = get_bit_register64(rn_val, 63);
                sign_op2 = 1;
                sign_res = get_bit_register64(res, 63);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                       && (sign_res != sign_rn_val);
                break;
        }
}

void arithmetic_helper_32(GeneralPurposeRegister *rd, uint32_t instruction, int32_t rn_val, uint32_t op2) {
    // case: opc (29-30)
        switch (get_bits(instruction, 29, 30)) {
            case 0b00:
                write_32(rd, rn_val + op2);
                break;
            case 0b01:
                write_32(rd, rn_val + op2);

                // reads the result after operation
                int32_t res = read_32(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < op2) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                int64_t sign_rn_val = get_bit_register32(rn_val, 31);
                int64_t sign_op2 = get_bit_register32(op2, 31);
                int64_t sign_res = get_bit_register32(res, 31);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                       && (sign_res != sign_rn_val);
                break;
            case 0b10:
                write_32(rd, rn_val - op2);
                break;
            case 0b11:
                write_32(rd, rn_val - op2);
                // reads the result after operation
                res = read_64(rd);
                // updates pState flag
                pStateRegister.negativeConditionFlag = get_bit_register32(res, 31);

                // updates pState zero condition flag
                if (res == 0) {
                    pStateRegister.zeroConditionFlag = 1;
                } else {
                    pStateRegister.zeroConditionFlag = 0;
                }

                // updates pState carry condition flag
                if (res < rn_val || res < op2) {
                    pStateRegister.carryConditionFlag = 1;
                } else {
                    pStateRegister.carryConditionFlag = 0;
                }

                // updates pState sign overflow condition flag
                sign_rn_val = get_bit_register32(rn_val, 31);
                sign_op2 = 1;
                sign_res = get_bit_register32(res, 31);
                pStateRegister.overflowConditionFlag = (sign_rn_val != sign_op2)
                                                       && (sign_res != sign_rn_val);
                break;
        }
}