#ifndef OPCODE_H
#define OPCODE_H

typedef enum {
    OPCODE_NOP = 0x00,
    OPCODE_LD_A_IMM8,
    OPCODE_LD_B_IMM8,
    OPCODE_LD_C_IMM8,
    OPCODE_LD_D_IMM8,
    OPCODE_LD_I_IMM16,
    OPCODE_LD_J_IMM16,
    OPCODE_LD_A_I_PTR,
    OPCODE_LD_A_J_PTR,
    OPCODE_LD_A_I_PTR_INC1,
    OPCODE_LD_A_J_PTR_INC1,
    OPCODE_LD_I_PTR_A,
    OPCODE_LD_J_PTR_A,
    OPCODE_LD_I_PTR_INC1_A,
    OPCODE_LD_J_PTR_INC1_A,
    OPCODE_LD_I_PTR_AB,
    OPCODE_LD_I_PTR_CD,
    OPCODE_LD_J_PTR_CD,
    OPCODE_LD_AB_I_PTR,
    OPCODE_LD_CD_I_PTR,
    OPCODE_LD_CD_J_PTR,
    OPCODE_LD_A_B,
    OPCODE_LD_A_C,
    OPCODE_LD_A_D,
    OPCODE_LD_B_A,
    OPCODE_LD_B_C,
    OPCODE_LD_B_D,
    OPCODE_LD_C_A,
    OPCODE_LD_C_B,
    OPCODE_LD_C_D,
    OPCODE_LD_D_A,
    OPCODE_LD_D_B,
    OPCODE_LD_D_C,
    OPCODE_INC_A,
    OPCODE_SHL_A,
    OPCODE_SHR_A,
    OPCODE_NOT_A,
    OPCODE_DEC_A,
    OPCODE_ROR_A,
    OPCODE_ADD_A_B,
    OPCODE_OR_A_B,
    OPCODE_AND_A_B,
    OPCODE_XOR_A_B,
    OPCODE_ADC_A_B,
    OPCODE_DEC_B,
    OPCODE_DEC_C,
    OPCODE_DEC_D,
    OPCODE_INC_B,
    OPCODE_INC_C,
    OPCODE_INC_D,
    OPCODE_ADD_D_B,
    OPCODE_ADC_C_A,
    OPCODE_ADC_D_IMM8,
    OPCODE_ADD_A_IMM8,
    OPCODE_OR_A_IMM8,
    OPCODE_AND_A_IMM8,
    OPCODE_XOR_A_IMM8,
    OPCODE_ADC_A_IMM8,
    OPCODE_ADD_B_IMM8,
    OPCODE_CMP_A_IMM8,
    OPCODE_CMP_B_IMM8,
    OPCODE_JMP_I,
    OPCODE_JMP_J,
    OPCODE_JMP_IMM16,
    OPCODE_JZ_IMM16,
    OPCODE_JNZ_IMM16,
    OPCODE_JC_IMM16,
    OPCODE_JNC_IMM16,
    OPCODE_JO_IMM16,
    OPCODE_JNO_IMM16,
    OPCODE_JS_IMM16,
    OPCODE_JNS_IMM16,
    OPCODE_LD_SP_IMM8,
    OPCODE_PUSH_A,
    OPCODE_PUSH_B,
    OPCODE_PUSH_C,
    OPCODE_PUSH_D,
    OPCODE_PUSH_I,
    OPCODE_PUSH_J,
    OPCODE_POP_A,
    OPCODE_POP_B,
    OPCODE_POP_C,
    OPCODE_POP_D,
    OPCODE_POP_I,
    OPCODE_POP_J,
    OPCODE_CALL_IMM16,
    OPCODE_RET,
    OPCODE_LD_A_SP_PLUS_IMM8_PTR,
    OPCODE_HALT,
    OPCODE_IN_A_PORT0 = 0xe8,
    OPCODE_IN_A_PORT1,
    OPCODE_IN_A_PORT2,
    OPCODE_IN_A_PORT3,
    OPCODE_IN_A_PORT4,
    OPCODE_IN_A_PORT5,
    OPCODE_IN_A_PORT6,
    OPCODE_IN_A_PORT7,
    OPCODE_OUT_PORT0_A,
    OPCODE_OUT_PORT1_A,
    OPCODE_OUT_PORT2_A,
    OPCODE_OUT_PORT3_A,
    OPCODE_OUT_PORT4_A,
    OPCODE_OUT_PORT5_A,
    OPCODE_OUT_PORT6_A,
    OPCODE_OUT_PORT7_A,
    OPCODE_OUT_PORT0_IMM8,
    OPCODE_OUT_PORT1_IMM8,
    OPCODE_OUT_PORT2_IMM8,
    OPCODE_OUT_PORT3_IMM8,
    OPCODE_OUT_PORT4_IMM8,
    OPCODE_OUT_PORT5_IMM8,
    OPCODE_OUT_PORT6_IMM8,
    OPCODE_OUT_PORT7_IMM8,
} Opcode;

// Port selection is defined by the lower 3 bits of the opcode.
_Static_assert((OPCODE_IN_A_PORT0 & 7) == 0, "Expected 0");
_Static_assert((OPCODE_IN_A_PORT1 & 7) == 1, "Expected 1");
_Static_assert((OPCODE_IN_A_PORT2 & 7) == 2, "Expected 2");
_Static_assert((OPCODE_IN_A_PORT3 & 7) == 3, "Expected 3");
_Static_assert((OPCODE_IN_A_PORT4 & 7) == 4, "Expected 4");
_Static_assert((OPCODE_IN_A_PORT5 & 7) == 5, "Expected 5");
_Static_assert((OPCODE_IN_A_PORT6 & 7) == 6, "Expected 6");
_Static_assert((OPCODE_IN_A_PORT7 & 7) == 7, "Expected 7");

_Static_assert((OPCODE_OUT_PORT0_A & 7) == 0, "Expected 0");
_Static_assert((OPCODE_OUT_PORT1_A & 7) == 1, "Expected 1");
_Static_assert((OPCODE_OUT_PORT2_A & 7) == 2, "Expected 2");
_Static_assert((OPCODE_OUT_PORT3_A & 7) == 3, "Expected 3");
_Static_assert((OPCODE_OUT_PORT4_A & 7) == 4, "Expected 4");
_Static_assert((OPCODE_OUT_PORT5_A & 7) == 5, "Expected 5");
_Static_assert((OPCODE_OUT_PORT6_A & 7) == 6, "Expected 6");
_Static_assert((OPCODE_OUT_PORT7_A & 7) == 7, "Expected 7");

_Static_assert((OPCODE_OUT_PORT0_IMM8 & 7) == 0, "Expected 0");
_Static_assert((OPCODE_OUT_PORT1_IMM8 & 7) == 1, "Expected 1");
_Static_assert((OPCODE_OUT_PORT2_IMM8 & 7) == 2, "Expected 2");
_Static_assert((OPCODE_OUT_PORT3_IMM8 & 7) == 3, "Expected 3");
_Static_assert((OPCODE_OUT_PORT4_IMM8 & 7) == 4, "Expected 4");
_Static_assert((OPCODE_OUT_PORT5_IMM8 & 7) == 5, "Expected 5");
_Static_assert((OPCODE_OUT_PORT6_IMM8 & 7) == 6, "Expected 6");
_Static_assert((OPCODE_OUT_PORT7_IMM8 & 7) == 7, "Expected 7");

#endif
