#pragma once
#ifndef ASS_H
#define ASS_H

#define LINE_LEN    256
#define SYMBOL_LEN  32
#define NUL         '\0'

#include "symtbl.h"

/* Addressing modes - As or Ad */
enum ADDR_MODE {
	REGISTER, INDEXED, RELATIVE, ABSOLUTE, INDIRECT,
	INDIRECT_INCR, IMMEDIATE, BAD_ADDR_MODE
};
/* Instruction type (number of operands) */
enum INST_TYPE { NONE, SINGLE, DOUBLE, JUMP };
/* Operand sizes */
enum BYTE_COMB { WORD, BYTE, WORDBYTE, OFFSET };

/* INSTRUCTION FORMATS: */
/* Single operand: Opcode (9) + BW (1) + As (2) + Reg (4) */
struct single_op
{
	unsigned reg : 4;
	unsigned as : 2;
	unsigned bw : 1;
	unsigned opcode : 9;
};

/* Jump instructions - Opcode (6) + Offset (10) */
struct jump_op
{
	unsigned offset : 10;
	unsigned opcode : 6;
};


/* Instruction entries
- create at compile time (see getinst.c)
- global to allow access to fields to build instruction
*/
struct inst_el
{
	char *inst;              /* Instruction mnemonic (e.g., MOV, MOV.W, or MOV.B */
	enum INST_TYPE type;     /* Type - None, Single, Double, or Jump */
	unsigned short opcode;   /* Opcode in 16-bits */
	enum BYTE_COMB bw;       /* Byte combination - Word, Byte, etc */
};

/* Extracting opcode from inst_el */
#define SINGLE_OP(x)     ((x)>>7)
#define DOUBLE_OP(x)     ((x)>>12)
#define JUMP_OP(x)       ((x)>>10)

/* Externals */
extern struct inst_el *get_inst(char *);

extern enum ADDR_MODE parse_opr(char *operand, unsigned short *value0, unsigned short *value1);
#endif