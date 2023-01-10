#ifndef ALU_OP_H
#define ALU_OP_H

typedef enum {
    // Unary operations
    ALU_OP_INC_LS,
    ALU_OP_SHL_LS,
    ALU_OP_SHR_LS,
    ALU_OP_NOT_LS,
    ALU_OP_DEC_LS,
    ALU_OP_ROR_LS,
    // ALU_OP_ROL_LS,

    // Binary operations
    ALU_OP_LS_ADD_RS,
    ALU_OP_LS_OR_RS,
    ALU_OP_LS_AND_RS,
    ALU_OP_LS_XOR_RS,
    ALU_OP_LS_ADC_RS,
    ALU_OP_LS_SUB_RS,

    // Special operations
    ALU_OP_SET_IO_OE_FLAG = 31 // Important to be above 0xf to not clash with any constant addresses
} ALU_OP;

#endif
