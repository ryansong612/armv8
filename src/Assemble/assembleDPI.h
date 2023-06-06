#include <stdint.h>

#ifndef ARMV8_32_DPI_H
#define ARMV8_32_DPI_H

typedef struct {
	uint32_t sf;
	uint32_t opc;
	uint32_t opi;
	uint32_t operand;
	uint32_t rd;
} dpi_immediate;

typedef struct {
	uint32_t sf;
	uint32_t opc;
	uint32_t opr;
	uint32_t rm;
	uint32_t operand;
	uint32_t rn;
	uint32_t rd;
} dpi_register;

uint32_t assemble_DPI(char *assembly_instruction);

#endif //ARMV8_32_DPI_H