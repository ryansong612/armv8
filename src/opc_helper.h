#ifndef ARMV8_32_DPI_REGISTER_OPC_HELPER_H
#define ARMV8_32_DPI_REGISTER_OPC_HELPER_H

void arithmetic_helper_64(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, uint32_t op2);
void arithmetic_helper_32(GeneralPurposeRegister *rd, uint32_t instruction, int64_t rn_val, uint32_t op2);

#endif //ARMV8_32_DPI_REGISTER_OPC_HELPER_H
