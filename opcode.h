#ifndef OPCODE_H
#define OPCODE_H

typedef enum {
    OPCODE_NOP = 0x00,
    OPCODE_LD_A_IMM8 = 0x01,
    OPCODE_LD_B_IMM8 = 0x02,
    OPCODE_LD_C_IMM8 = 0x03,
    OPCODE_LD_D_IMM8 = 0x04,
    OPCODE_LD_I_IMM16 = 0x05,
    OPCODE_LD_J_IMM16 = 0x06,
    OPCODE_LD_A_I_PTR = 0x07,
    OPCODE_LD_A_J_PTR = 0x08,
    OPCODE_LD_A_I_PTR_INC1 = 0x09,
    OPCODE_LD_A_J_PTR_INC1 = 0x0a,
    OPCODE_LD_I_PTR_A = 0x0b,
    OPCODE_LD_J_PTR_A = 0x0c,
    OPCODE_LD_I_PTR_INC1_A = 0x0d,
    OPCODE_LD_J_PTR_INC1_A = 0x0e,
    OPCODE_LD_I_PTR_AB = 0x0f,
    OPCODE_LD_I_PTR_CD = 0x10,
    OPCODE_LD_J_PTR_CD = 0x12,
    OPCODE_LD_AB_I_PTR = 0x11,
    OPCODE_LD_CD_I_PTR = 0x13,
    OPCODE_LD_CD_J_PTR = 0x14,
    // 0x15
    // 0x16
    // 0x17
    // 0x18
    // 0x19
    // 0x1a
    // 0x1b
    // 0x1c
    // 0x1d
    // 0x1e
    // 0x1f
    // 0x20

    OPCODE_LD_A_B = 0x21,
    OPCODE_LD_A_C = 0x22,
    OPCODE_LD_A_D = 0x23,
    OPCODE_LD_B_A = 0x24,
    // 0x25

    OPCODE_LD_B_C = 0x26,
    OPCODE_LD_B_D = 0x27,
    OPCODE_LD_C_A = 0x28,
    OPCODE_LD_C_B = 0x29,
    // 0x2a

    OPCODE_LD_C_D = 0x2b,
    OPCODE_LD_D_A = 0x2c,
    OPCODE_LD_D_B = 0x2d,
    OPCODE_LD_D_C = 0x2e,
    // 0x2f
    // 0x30
    // 0x31
    // 0x32
    // 0x33
    // 0x34
    // 0x35
    // 0x36
    // 0x37
    // 0x38
    // 0x39
    // 0x3a
    // 0x3b
    // 0x3c
    // 0x3d
    // 0x3e
    // 0x3f

    OPCODE_INC_A = 0x40,
    OPCODE_SHL_A = 0x41,
    OPCODE_SHR_A = 0x42,
    OPCODE_NOT_A = 0x43,
    OPCODE_DEC_A = 0x44,
    OPCODE_ROR_A = 0x45,
    // 0x46
    // 0x47
    // 0x48
    // 0x49
    // 0x4a
    // 0x4b
    // 0x4c
    // 0x4d
    // 0x4e
    // 0x4f

    OPCODE_ADD_A_B = 0x50,
    OPCODE_OR_A_B = 0x51,
    OPCODE_AND_A_B = 0x52,
    OPCODE_XOR_A_B = 0x53,
    OPCODE_ADC_A_B = 0x54,
    // 0x55
    // 0x56
    // 0x57
    // 0x58
    // 0x59
    // 0x5a
    // 0x5b
    // 0x5c
    // 0x5d
    // 0x5e
    // 0x5f

    OPCODE_DEC_B = 0x60,
    OPCODE_DEC_C = 0x61,
    OPCODE_DEC_D = 0x62,
    OPCODE_INC_B = 0x63,
    OPCODE_INC_C = 0x64,
    OPCODE_INC_D = 0x65,
    OPCODE_ADD_D_B = 0x66,
    OPCODE_ADC_C_A = 0x67,
    OPCODE_ADC_D_IMM8 = 0x68,
    OPCODE_ADD_A_IMM8 = 0x6f,
    OPCODE_OR_A_IMM8 = 0x70,
    OPCODE_AND_A_IMM8 = 0x71,
    OPCODE_XOR_A_IMM8 = 0x72,
    OPCODE_ADC_A_IMM8 = 0x73,
    // 0x74

    OPCODE_ADD_B_IMM8 = 0x75,
    OPCODE_CMP_A_IMM8 = 0x76,
    OPCODE_CMP_B_IMM8 = 0x77,
    OPCODE_OUT_PORT0_IMM8 = 0x78,
    OPCODE_OUT_PORT1_IMM8 = 0x79,
    OPCODE_OUT_PORT2_IMM8 = 0x7a,
    OPCODE_OUT_PORT3_IMM8 = 0x7b,
    OPCODE_OUT_PORT4_IMM8 = 0x7c,
    OPCODE_OUT_PORT5_IMM8 = 0x7d,
    OPCODE_OUT_PORT6_IMM8 = 0x7e,
    OPCODE_OUT_PORT7_IMM8 = 0x7f,
    OPCODE_IN_A_PORT0 = 0x80,
    OPCODE_IN_A_PORT1 = 0x81,
    OPCODE_IN_A_PORT2 = 0x82,
    OPCODE_IN_A_PORT3 = 0x83,
    OPCODE_IN_A_PORT4 = 0x84,
    OPCODE_IN_A_PORT5 = 0x85,
    OPCODE_IN_A_PORT6 = 0x86,
    OPCODE_IN_A_PORT7 = 0x87,
    OPCODE_OUT_PORT0_A = 0x88,
    OPCODE_OUT_PORT1_A = 0x89,
    OPCODE_OUT_PORT2_A = 0x8a,
    OPCODE_OUT_PORT3_A = 0x8b,
    OPCODE_OUT_PORT4_A = 0x8c,
    OPCODE_OUT_PORT5_A = 0x8d,
    OPCODE_OUT_PORT6_A = 0x8e,
    OPCODE_OUT_PORT7_A = 0x8f,
    OPCODE_JMP_I = 0x90,
    OPCODE_JMP_J = 0x91,
    OPCODE_JMP_IMM16 = 0x92,
    OPCODE_JZ_IMM16 = 0x93,
    OPCODE_JNZ_IMM16 = 0x94,
    OPCODE_JC_IMM16 = 0x95,
    OPCODE_JNC_IMM16 = 0x96,
    OPCODE_JO_IMM16 = 0x97,
    OPCODE_JNO_IMM16 = 0x98,
    OPCODE_JS_IMM16 = 0x99,
    OPCODE_JNS_IMM16 = 0x9a,
    OPCODE_LD_SP_IMM8 = 0xa0,
    OPCODE_PUSH_A = 0xa2,
    OPCODE_PUSH_B = 0xa3,
    OPCODE_PUSH_C = 0xa4,
    OPCODE_PUSH_D = 0xa5,
    OPCODE_PUSH_I = 0xa6,
    OPCODE_PUSH_J = 0xa7,
    OPCODE_POP_A = 0xa8,
    OPCODE_POP_B = 0xa9,
    OPCODE_POP_C = 0xaa,
    OPCODE_POP_D = 0xab,
    OPCODE_POP_I = 0xac,
    OPCODE_POP_J = 0xad,
    OPCODE_CALL_IMM16 = 0xb0,
    OPCODE_RET = 0xb1,
    OPCODE_LD_A_SP_PLUS_IMM8_PTR = 0xb2,
    // 0xb3
    // 0xb4
    // 0xb5
    // 0xb6
    // 0xb7
    // 0xb8
    // 0xb9
    // 0xba
    // 0xbb
    // 0xbc
    // 0xbd
    // 0xbe
    // 0xbf
    // 0xc0
    // 0xc1
    // 0xc2
    // 0xc3
    // 0xc4
    // 0xc5
    // 0xc6
    // 0xc7
    // 0xc8
    // 0xc9
    // 0xca
    // 0xcb
    // 0xcc
    // 0xcd
    // 0xce
    // 0xcf
    // 0xd0
    // 0xd1
    // 0xd2
    // 0xd3
    // 0xd4
    // 0xd5
    // 0xd6
    // 0xd7
    // 0xd8
    // 0xd9
    // 0xda
    // 0xdb
    // 0xdc
    // 0xdd
    // 0xde
    // 0xdf
    // 0xe0
    // 0xe1
    // 0xe2
    // 0xe3
    // 0xe4
    // 0xe5
    // 0xe6
    // 0xe7
    // 0xe8
    // 0xe9
    // 0xea
    // 0xeb
    // 0xec
    // 0xed
    // 0xee
    // 0xef
    // 0xf0
    // 0xf1
    // 0xf2
    // 0xf3
    // 0xf4
    // 0xf5
    // 0xf6
    // 0xf7
    // 0xf8
    // 0xf9
    // 0xfa
    // 0xfb
    // 0xfc
    // 0xfd
    // 0xfe

    OPCODE_HALT = 0xff,
} Opcode;

#endif
