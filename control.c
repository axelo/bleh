#include <assert.h> // assert
#include <stdbool.h> // bool
#include <stdint.h> // uint*_t
#include <stdio.h> // FILE, f* functions

#include "alu_op.h"
#include "opcode.h"

#define CONTROL_ROM_SIZE (1 << 17)

#define PIN_OPCODE_0 0
#define PIN_OPCODE_1 1
#define PIN_OPCODE_2 2
#define PIN_OPCODE_3 3
#define PIN_OPCODE_4 4
#define PIN_OPCODE_5 5
#define PIN_OPCODE_6 6
#define PIN_OPCODE_7 7

#define PIN_HIGH_SLICE 10

#define PIN_FLAG_0_ZERO 11
#define PIN_FLAG_1_CARRY 9
#define PIN_FLAG_2_OVERFLOW 8
#define PIN_FLAG_3_SIGN 13

#define PIN_STEP_0 12
#define PIN_STEP_1 15
#define PIN_STEP_2 16
#define PIN_STEP_3 14

#define CE_M_NOT_LD_C (1 << 0)
#define LD_O_NOT_LD_C (1 << 1)
#define LD_S_NOT_LD_C (1 << 2)
#define LD_RS_NOT_LD_C (1 << 3)
#define LD_IO_NOT_LD_C (1 << 4)
#define C_LS_ALU_Q (1 << 5)
#define HALT_NOT_LD_C (1 << 5)
#define LD_C (1 << 6)
#define TG_M_C (1 << 7)
#define LD_MEM (1 << 8)
#define LD_LS (1 << 9)
#define LD_ML (1 << 10)
#define LD_MH (1 << 11)
#define OE_ML (1 << 12)
#define OE_MH (1 << 13)
#define OE_ALU (1 << 14)
#define OE_MEM (1 << 15)

#define ACTIVE_LOW_MASK (LD_C | LD_LS | LD_ML | LD_MH | OE_ML | OE_MH | OE_ALU | OE_MEM)

#define FETCH_OPCODE (OE_MEM | LD_O_NOT_LD_C | CE_M_NOT_LD_C)

#define C_A (0x0)
#define C_B (0x1)
#define C_C (0x2)
#define C_D (0x3)
#define C_SPL (0x4)
#define C_IL (0x5)
#define C_IH (0x6)
#define C_JL (0x7)
#define C_JH (0x8)
// #define C_ (0x9)
// #define C_ (0xa)
#define C_TL (0xb)
#define C_TH (0xc)
#define C_UL (0xd)
// #define C_ (0xe)
// #define C_ (0xf)

_Static_assert(ALU_OP_DEC_LS == C_SPL, "C_SPL must be identical to ALU_OP_DEC_LS");

static uint16_t opcode_nop(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return 0;
    case 2: return 0;
    case 3: return 0;
    case 4: return 0;
    case 5: return 0;
    case 6: return 0;
    case 7: return LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_halt(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return HALT_NOT_LD_C;
    case 2: return LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_reg_imm8(uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
    case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_index_imm16(uint8_t dest_const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_index_l | LD_C | TG_M_C;
    case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C;
    case 3: return OE_MEM | LD_LS | C_LS_ALU_Q | (dest_const_index_l + 1) | LD_C | TG_M_C;
    case 4: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_reg_index_ptr(uint8_t dest_const_reg, uint8_t const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 5: return OE_MEM | LD_MH | TG_M_C;
    case 6: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
    case 7: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 8: return OE_MEM | LD_ML | C_TH | LD_C;
    case 9: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_reg_index_ptr_inc1(uint8_t dest_const_reg, uint8_t const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 5: return OE_MEM | LD_MH | TG_M_C;
    case 6: return OE_MEM | LD_LS | CE_M_NOT_LD_C | TG_M_C;
    case 7: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 8: return OE_ML | LD_MEM | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 9: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 10: return OE_MEM | LD_ML | C_TH | LD_C;
    case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_index_ptr_reg(uint8_t const_index_l, uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 5: return OE_MEM | LD_MH | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 6: return OE_MEM | LD_LS | TG_M_C;
    case 7: return OE_ALU | LD_MEM | C_TL | LD_C | TG_M_C;
    case 8: return OE_MEM | LD_ML | C_TH | LD_C;
    case 9: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_index_ptr_inc1_reg(uint8_t const_index_l, uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return dest_const_reg | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_LS | C_TL | LD_C;
    case 3: return OE_ML | LD_MEM | C_TH | LD_C;
    case 4: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 5: return OE_MEM | LD_ML | C_LS_ALU_Q | (const_index_l + 1) | LD_C;
    case 6: return OE_MEM | LD_MH | TG_M_C;
    case 7: return OE_ALU | LD_MEM | CE_M_NOT_LD_C | TG_M_C;
    case 8: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 9: return OE_ML | LD_MEM | C_TL | LD_C;
    case 10: return OE_MEM | LD_ML | C_TH | LD_C;
    case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_index_ptr_reg_reg(uint8_t const_index_l, uint8_t src_const_reg_h, uint8_t src_const_reg_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 5: return OE_MEM | LD_MH | C_LS_ALU_Q | src_const_reg_l | LD_C;
    case 6: return OE_MEM | LD_LS | TG_M_C;
    case 7: return OE_ALU | LD_MEM | C_LS_ALU_Q | src_const_reg_h | LD_C | TG_M_C;
    case 8: return OE_MEM | LD_LS | CE_M_NOT_LD_C | TG_M_C;
    case 9: return OE_ALU | LD_MEM | C_TL | LD_C | TG_M_C;
    case 10: return OE_MEM | LD_ML | C_TH | LD_C;
    case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_reg_reg_index_ptr(uint8_t dest_const_reg_h, uint8_t dest_const_reg_l, uint8_t const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
    case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 5: return OE_MEM | LD_MH | C_LS_ALU_Q | dest_const_reg_l | LD_C | TG_M_C;
    case 6: return OE_MEM | LD_LS | CE_M_NOT_LD_C | TG_M_C;
    case 7: return OE_ALU | LD_MEM | C_LS_ALU_Q | dest_const_reg_h | LD_C | TG_M_C;
    case 8: return OE_MEM | LD_LS | TG_M_C;
    case 9: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 10: return OE_MEM | LD_ML | C_TH | LD_C;
    case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static u_int16_t opcode_ld_reg_reg(uint8_t dest_const_reg, uint8_t src_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return src_const_reg | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 3: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_alu_op_reg(ALU_OP alu_op, uint8_t dest_and_src_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return dest_and_src_const_reg | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_LS | (uint8_t)alu_op | LD_C;
    case 3: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_and_src_const_reg | LD_C;
    case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_alu_op_reg_reg(ALU_OP alu_op, uint8_t dest_const_reg, uint8_t src_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return dest_const_reg | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_LS | src_const_reg | LD_C;
    case 3: return OE_MEM | LD_RS_NOT_LD_C;
    case 4: return (uint8_t)alu_op | LD_C;
    case 5: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 6: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_alu_op_reg_imm8(ALU_OP alu_op, uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C;
    case 2: return dest_const_reg | LD_C | TG_M_C;
    case 3: return OE_MEM | LD_LS | (uint8_t)alu_op | LD_C;
    case 4: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 5: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_alu_cmp_reg_imm8(uint8_t const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C;
    case 2: return const_reg | LD_C | TG_M_C;
    case 3: return OE_MEM | LD_LS | ALU_OP_LS_SUB_RS | LD_C;
    case 4: return OE_ALU | LD_LS | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_jmp_imm16(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_LS | CE_M_NOT_LD_C;
    case 2: return OE_MEM | LD_MH | C_LS_ALU_Q | LD_C;
    case 3: return OE_ALU | LD_ML | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_jmp_index(uint8_t const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return const_index_l | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
    case 3: return OE_MEM | LD_MH | LD_S_NOT_LD_C | TG_M_C;
    default: return HALT_NOT_LD_C;
    }
}

static u_int16_t opcode_jmp_condition_imm16(bool condition, uint8_t step) {
    if (condition) {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | CE_M_NOT_LD_C;
        case 2: return OE_MEM | LD_MH | C_LS_ALU_Q | LD_C;
        case 3: return OE_ALU | LD_ML | LD_S_NOT_LD_C;
        default: return HALT_NOT_LD_C;
        }
    } else {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return CE_M_NOT_LD_C;
        case 2: return CE_M_NOT_LD_C;
        case 3: return LD_S_NOT_LD_C;
        default: return HALT_NOT_LD_C;
        }
    }
}

static uint16_t opcode_push_reg(uint8_t src_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | C_SPL | LD_C;
    case 4: return OE_MEM | LD_ML;
    case 5: return LD_MH | CE_M_NOT_LD_C;
    case 6: return OE_ML | LD_MEM | C_LS_ALU_Q | src_const_reg | LD_C;
    case 7: return OE_MEM | LD_LS | TG_M_C;
    case 8: return OE_ALU | LD_MEM | C_TL | LD_C | TG_M_C;
    case 9: return OE_MEM | LD_ML | C_TH | LD_C;
    case 10: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_pop_reg(uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | C_SPL | LD_C; // ALU_OP_DEC_LS at the same time
    case 4: return OE_MEM | LD_ML | LD_LS;
    case 5: return OE_ALU | LD_MEM; // --sp
    case 6: return LD_MH | TG_M_C; // MH = 0xff
    case 7: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
    case 8: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 9: return OE_MEM | LD_ML | C_TH | LD_C;
    case 10: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_push_index(uint8_t src_const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | C_SPL | LD_C;
    case 4: return OE_MEM | LD_ML | src_const_index_l | LD_C;
    case 5: return LD_MH | CE_M_NOT_LD_C;
    case 6: return OE_MEM | LD_LS | C_LS_ALU_Q | (src_const_index_l + 1) | LD_C | TG_M_C;
    case 7: return OE_ALU | LD_MEM | CE_M_NOT_LD_C | TG_M_C;
    case 8: return OE_MEM | LD_LS | TG_M_C;
    case 9: return OE_ALU | LD_MEM | C_SPL | LD_C | TG_M_C;
    case 10: return OE_ML | LD_MEM | C_TL | LD_C;
    case 11: return OE_MEM | LD_ML | C_TH | LD_C;
    case 12: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_pop_index(uint8_t dest_const_index_l, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_TL | LD_C | TG_M_C;
    case 2: return OE_ML | LD_MEM | C_TH | LD_C;
    case 3: return OE_MH | LD_MEM | C_SPL | LD_C; // ALU_OP_DEC_LS at the same time
    case 4: return OE_MEM | LD_ML | LD_LS;
    case 5: return OE_ALU | LD_MEM; // --sp
    case 6: return LD_MH | TG_M_C;
    case 7: return OE_MEM | LD_LS | C_LS_ALU_Q | (dest_const_index_l + 1) | LD_C | TG_M_C;
    case 8: return OE_ALU | LD_MEM | C_SPL | LD_C;
    case 9: return OE_MEM | LD_ML | LD_LS;
    case 10: return OE_ALU | LD_MEM | TG_M_C; // --sp
    case 11: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_index_l | LD_C | TG_M_C;
    case 12: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 13: return OE_MEM | LD_ML | C_TH | LD_C;
    case 14: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_call_imm16(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | C_TL | LD_C | TG_M_C;
    case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C;
    case 3: return OE_MEM | LD_LS | C_LS_ALU_Q | C_TH | LD_C | TG_M_C;
    case 4: return OE_ALU | LD_MEM | CE_M_NOT_LD_C;
    case 5: return OE_ML | LD_LS | C_UL | LD_C;
    case 6: return OE_MH | LD_MEM | C_SPL | LD_C;
    case 7: return OE_MEM | LD_ML | C_LS_ALU_Q | C_UL | LD_C;
    case 8: return LD_MH | CE_M_NOT_LD_C | TG_M_C;
    case 9: return OE_ALU | LD_MEM | CE_M_NOT_LD_C | TG_M_C;
    case 10: return OE_MEM | LD_LS | TG_M_C;
    case 11: return OE_ALU | LD_MEM | C_SPL | LD_C | TG_M_C;
    case 12: return OE_ML | LD_MEM | C_TL | LD_C;
    case 13: return OE_MEM | LD_ML | C_TH | LD_C;
    case 14: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ret(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return C_SPL | LD_C | TG_M_C; // ALU_OP_DEC_LS at the same time
    case 2: return OE_MEM | LD_ML | LD_LS;
    case 3: return OE_ALU | LD_MEM; // --sp
    case 4: return LD_MH | TG_M_C;
    case 5: return OE_MEM | LD_LS | C_LS_ALU_Q | C_TH | LD_C | TG_M_C;
    case 6: return OE_ALU | LD_MEM | C_SPL | LD_C;
    case 7: return OE_MEM | LD_ML | LD_LS;
    case 8: return OE_ALU | LD_MEM | TG_M_C; // --sp
    case 9: return OE_MEM | LD_ML | C_TH | LD_C | TG_M_C;
    case 10: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_ld_reg_sp_plus_imm8_ptr(uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C; // RS = imm
    case 2: return C_TL | LD_C | TG_M_C;
    case 3: return OE_ML | LD_MEM | C_TH | LD_C;
    case 4: return OE_MH | LD_MEM | C_SPL | LD_C;
    case 5: return OE_MEM | LD_LS | ALU_OP_LS_ADD_RS | LD_C; // LS = sp
    case 6: return OE_ALU | LD_ML; // ML = (sp + imm) & 0xff
    case 7: return LD_MH | TG_M_C;
    case 8: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
    case 9: return OE_ALU | LD_MEM | C_TL | LD_C;
    case 10: return OE_MEM | LD_ML | C_TH | LD_C;
    case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_in_reg_port(uint8_t dest_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return ALU_OP_SET_IO_OE_FLAG | LD_C | TG_M_C;
    case 2: return LD_C;
    case 3: return LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
    case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_out_port_reg(uint8_t src_const_reg, uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return src_const_reg | LD_C | TG_M_C;
    case 2: return OE_MEM | LD_IO_NOT_LD_C;
    case 3: return TG_M_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t opcode_out_port_imm8(uint8_t step) {
    switch (step) {
    case 0: return FETCH_OPCODE;
    case 1: return OE_MEM | LD_IO_NOT_LD_C;
    case 2: return CE_M_NOT_LD_C | LD_S_NOT_LD_C;
    default: return HALT_NOT_LD_C;
    }
}

static uint16_t signals_from_input(uint8_t step, bool zero_flag_set, bool carry_flag_set, bool overflow_flag_set, bool sign_flag_set, Opcode opcode) {
    assert(step < 16);

    switch (opcode) {
    case OPCODE_NOP: return opcode_nop(step);
    case OPCODE_HALT: return opcode_halt(step);
    case OPCODE_LD_A_IMM8: return opcode_ld_reg_imm8(C_A, step); // ld a, {imm: i8}
    case OPCODE_LD_B_IMM8: return opcode_ld_reg_imm8(C_B, step); // ld b, {imm: i8}
    case OPCODE_LD_C_IMM8: return opcode_ld_reg_imm8(C_C, step); // ld c, {imm: i8}
    case OPCODE_LD_D_IMM8: return opcode_ld_reg_imm8(C_D, step); // ld d, {imm: i8}
    case OPCODE_LD_I_IMM16: return opcode_ld_index_imm16(C_IL, step); // ld i, {imm: i16}
    case OPCODE_LD_J_IMM16: return opcode_ld_index_imm16(C_JL, step); // ld j, {imm: i16}
    case OPCODE_LD_A_I_PTR: return opcode_ld_reg_index_ptr(C_A, C_IL, step); // ld a, [i]
    case OPCODE_LD_A_J_PTR: return opcode_ld_reg_index_ptr(C_A, C_JL, step); // ld a, [j]
    case OPCODE_LD_A_I_PTR_INC1: return opcode_ld_reg_index_ptr_inc1(C_A, C_IL, step); // ld a, [i++]
    case OPCODE_LD_A_J_PTR_INC1: return opcode_ld_reg_index_ptr_inc1(C_A, C_JL, step); // ld a, [j++]
    case OPCODE_LD_I_PTR_A: return opcode_ld_index_ptr_reg(C_IL, C_A, step); // ld [i], a
    case OPCODE_LD_J_PTR_A: return opcode_ld_index_ptr_reg(C_JL, C_A, step); // ld [j], a
    case OPCODE_LD_I_PTR_INC1_A: return opcode_ld_index_ptr_inc1_reg(C_IL, C_A, step); // ld [i++], a
    case OPCODE_LD_J_PTR_INC1_A: return opcode_ld_index_ptr_inc1_reg(C_JL, C_A, step); // ld [j++], a
    case OPCODE_LD_I_PTR_AB: return opcode_ld_index_ptr_reg_reg(C_IL, C_A, C_B, step); // ld [i], ab
    case OPCODE_LD_I_PTR_CD: return opcode_ld_index_ptr_reg_reg(C_IL, C_C, C_D, step); // ld [i], cd
    case OPCODE_LD_J_PTR_CD: return opcode_ld_index_ptr_reg_reg(C_JL, C_C, C_D, step); // ld [j], cd
    case OPCODE_LD_AB_I_PTR: return opcode_ld_reg_reg_index_ptr(C_A, C_B, C_IL, step); // ld ab, [i]
    case OPCODE_LD_CD_I_PTR: return opcode_ld_reg_reg_index_ptr(C_C, C_D, C_IL, step); // ld cd, [i]
    case OPCODE_LD_CD_J_PTR: return opcode_ld_reg_reg_index_ptr(C_C, C_D, C_JL, step); // ld cd, [j]
    case OPCODE_LD_A_B: return opcode_ld_reg_reg(C_A, C_B, step); // ld a, b
    case OPCODE_LD_A_C: return opcode_ld_reg_reg(C_A, C_C, step); // ld a, c
    case OPCODE_LD_A_D: return opcode_ld_reg_reg(C_A, C_D, step); // ld a, d
    case OPCODE_LD_B_A: return opcode_ld_reg_reg(C_B, C_A, step); // ld b, a
    case OPCODE_LD_B_C: return opcode_ld_reg_reg(C_B, C_C, step); // ld b, c
    case OPCODE_LD_B_D: return opcode_ld_reg_reg(C_B, C_D, step); // ld b, d
    case OPCODE_LD_C_A: return opcode_ld_reg_reg(C_C, C_A, step); // ld c, a
    case OPCODE_LD_C_B: return opcode_ld_reg_reg(C_C, C_B, step); // ld c, b
    case OPCODE_LD_C_D: return opcode_ld_reg_reg(C_C, C_D, step); // ld c, d
    case OPCODE_LD_D_A: return opcode_ld_reg_reg(C_D, C_A, step); // ld d, a
    case OPCODE_LD_D_B: return opcode_ld_reg_reg(C_D, C_B, step); // ld d, b
    case OPCODE_LD_D_C: return opcode_ld_reg_reg(C_D, C_C, step); // ld d, c
    case OPCODE_INC_A: return opcode_alu_op_reg(ALU_OP_INC_LS, C_A, step); // inc a
    case OPCODE_INC_B: return opcode_alu_op_reg(ALU_OP_INC_LS, C_B, step); // inc b
    case OPCODE_INC_C: return opcode_alu_op_reg(ALU_OP_INC_LS, C_C, step); // inc c
    case OPCODE_INC_D: return opcode_alu_op_reg(ALU_OP_INC_LS, C_D, step); // inc d
    case OPCODE_DEC_A: return opcode_alu_op_reg(ALU_OP_DEC_LS, C_A, step); // dec a
    case OPCODE_DEC_B: return opcode_alu_op_reg(ALU_OP_DEC_LS, C_B, step); // dec b
    case OPCODE_DEC_C: return opcode_alu_op_reg(ALU_OP_DEC_LS, C_C, step); // dec c
    case OPCODE_DEC_D: return opcode_alu_op_reg(ALU_OP_DEC_LS, C_D, step); // dec d
    case OPCODE_SHL_A: return opcode_alu_op_reg(ALU_OP_SHL_LS, C_A, step); // shl a
    case OPCODE_SHR_A: return opcode_alu_op_reg(ALU_OP_SHR_LS, C_A, step); // shr a
    case OPCODE_NOT_A: return opcode_alu_op_reg(ALU_OP_NOT_LS, C_A, step); // not a
    case OPCODE_ROR_A: return opcode_alu_op_reg(ALU_OP_ROR_LS, C_A, step); // ror a
    case OPCODE_ADD_A_B: return opcode_alu_op_reg_reg(ALU_OP_LS_ADD_RS, C_A, C_B, step); // add a, b
    case OPCODE_OR_A_B: return opcode_alu_op_reg_reg(ALU_OP_LS_OR_RS, C_A, C_B, step); // or a, b
    case OPCODE_AND_A_B: return opcode_alu_op_reg_reg(ALU_OP_LS_AND_RS, C_A, C_B, step); // and a, b
    case OPCODE_XOR_A_B: return opcode_alu_op_reg_reg(ALU_OP_LS_XOR_RS, C_A, C_B, step); // xor a, b
    case OPCODE_ADC_A_B: return opcode_alu_op_reg_reg(ALU_OP_LS_ADC_RS, C_A, C_B, step); // adc a, b
    case OPCODE_ADC_C_A: return opcode_alu_op_reg_reg(ALU_OP_LS_ADC_RS, C_C, C_A, step); // adc c, a
    case OPCODE_ADD_D_B: return opcode_alu_op_reg_reg(ALU_OP_LS_ADD_RS, C_D, C_B, step); // add d, b
    case OPCODE_ADD_A_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_ADD_RS, C_A, step); // add a, {imm: i8}
    case OPCODE_ADD_B_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_ADD_RS, C_B, step); // add b, {imm: i8}
    case OPCODE_AND_A_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_AND_RS, C_A, step); // and a, {imm: i8}
    case OPCODE_OR_A_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_OR_RS, C_A, step); // or a, {imm: i8}
    case OPCODE_XOR_A_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_XOR_RS, C_A, step); // xor a, {imm: i8}
    case OPCODE_ADC_A_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_ADC_RS, C_A, step); // adc a, {imm: i8}
    case OPCODE_ADC_D_IMM8: return opcode_alu_op_reg_imm8(ALU_OP_LS_ADC_RS, C_D, step); // adc d, {imm: i8}
    case OPCODE_CMP_A_IMM8: return opcode_alu_cmp_reg_imm8(C_A, step); // cmp a, {imm: i8}
    case OPCODE_CMP_B_IMM8: return opcode_alu_cmp_reg_imm8(C_B, step); // cmp b, {imm: i8}
    case OPCODE_JMP_I: return opcode_jmp_index(C_IL, step); // jmp i
    case OPCODE_JMP_J: return opcode_jmp_index(C_JL, step); // jmp j
    case OPCODE_JMP_IMM16: return opcode_jmp_imm16(step); // jmp {imm: i16}
    case OPCODE_JZ_IMM16: return opcode_jmp_condition_imm16(zero_flag_set, step); // jz  {imm: i16}
    case OPCODE_JNZ_IMM16: return opcode_jmp_condition_imm16(!zero_flag_set, step); // jnz {imm: i16}
    case OPCODE_JC_IMM16: return opcode_jmp_condition_imm16(carry_flag_set, step); // jc  {imm: i16}
    case OPCODE_JNC_IMM16: return opcode_jmp_condition_imm16(!carry_flag_set, step); // jnc {imm: i16}
    case OPCODE_JO_IMM16: return opcode_jmp_condition_imm16(overflow_flag_set, step); // jo {imm: i16}
    case OPCODE_JNO_IMM16: return opcode_jmp_condition_imm16(!overflow_flag_set, step); // jno {imm: i16}
    case OPCODE_JS_IMM16: return opcode_jmp_condition_imm16(sign_flag_set, step); // js {imm: i16}
    case OPCODE_JNS_IMM16: return opcode_jmp_condition_imm16(!sign_flag_set, step); // jns {imm: i16}
    case OPCODE_LD_SP_IMM8: return opcode_ld_reg_imm8(C_SPL, step); // ld sp, {imm: i8}
    case OPCODE_PUSH_A: return opcode_push_reg(C_A, step); // push a (store at ++sp)
    case OPCODE_PUSH_B: return opcode_push_reg(C_B, step); // push b (store at ++sp)
    case OPCODE_PUSH_C: return opcode_push_reg(C_C, step); // push c (store at ++sp)
    case OPCODE_PUSH_D: return opcode_push_reg(C_D, step); // push d (store at ++sp)
    case OPCODE_PUSH_I: return opcode_push_index(C_IL, step); // push i (store l at ++sp, h at ++sp)
    case OPCODE_PUSH_J: return opcode_push_index(C_JL, step); // push j (store l at ++sp, h at ++sp)
    case OPCODE_POP_A: return opcode_pop_reg(C_A, step); // pop a (load from sp--)
    case OPCODE_POP_B: return opcode_pop_reg(C_B, step); // pop b (load from sp--)
    case OPCODE_POP_C: return opcode_pop_reg(C_C, step); // pop c (load from sp--)
    case OPCODE_POP_D: return opcode_pop_reg(C_D, step); // pop d (load from sp--)
    case OPCODE_POP_I: return opcode_pop_index(C_IL, step); // pop i (read h at sp--, l at sp--)
    case OPCODE_POP_J: return opcode_pop_index(C_JL, step); // pop j (read h at sp--, l at sp--)
    case OPCODE_CALL_IMM16: return opcode_call_imm16(step); // call {imm: i16} (store pc l at ++sp, pc h at ++sp)
    case OPCODE_RET: return opcode_ret(step); // ret (read pc h at sp--, pc l at sp--)
    case OPCODE_LD_A_SP_PLUS_IMM8_PTR: return opcode_ld_reg_sp_plus_imm8_ptr(C_A, step); // ld a, [sp+{imm:i8}]
    case OPCODE_IN_A_PORT0: // in a, {port: u3}
    case OPCODE_IN_A_PORT1:
    case OPCODE_IN_A_PORT2:
    case OPCODE_IN_A_PORT3:
    case OPCODE_IN_A_PORT4:
    case OPCODE_IN_A_PORT5:
    case OPCODE_IN_A_PORT6:
    case OPCODE_IN_A_PORT7: return opcode_in_reg_port(C_A, step);
    case OPCODE_OUT_PORT0_A: // out {port: u3}, a
    case OPCODE_OUT_PORT1_A:
    case OPCODE_OUT_PORT2_A:
    case OPCODE_OUT_PORT3_A:
    case OPCODE_OUT_PORT4_A:
    case OPCODE_OUT_PORT5_A:
    case OPCODE_OUT_PORT6_A:
    case OPCODE_OUT_PORT7_A: return opcode_out_port_reg(C_A, step);
    case OPCODE_OUT_PORT0_IMM8: // out {port: u3}, {imm: i8}
    case OPCODE_OUT_PORT1_IMM8:
    case OPCODE_OUT_PORT2_IMM8:
    case OPCODE_OUT_PORT3_IMM8:
    case OPCODE_OUT_PORT4_IMM8:
    case OPCODE_OUT_PORT5_IMM8:
    case OPCODE_OUT_PORT6_IMM8:
    case OPCODE_OUT_PORT7_IMM8: return opcode_out_port_imm8(step);
    }

    // Ensure step 0 always is fetch opcode, we don't know the output of O during start up.
    switch (step) {
    case 0: return FETCH_OPCODE;
    default: return HALT_NOT_LD_C;
    }
}

static void generate_table(uint8_t (*table)[CONTROL_ROM_SIZE]) {
    for (size_t i = 0; i < CONTROL_ROM_SIZE; ++i) {
        uint8_t step = (uint8_t)((((i >> PIN_STEP_3) & 1) << 3) |
                                 (((i >> PIN_STEP_2) & 1) << 2) |
                                 (((i >> PIN_STEP_1) & 1) << 1) |
                                 (((i >> PIN_STEP_0) & 1) << 0));

        bool zero_flag_set = (i >> PIN_FLAG_0_ZERO) & 1;
        bool carry_flag_set = (i >> PIN_FLAG_1_CARRY) & 1;
        bool overflow_flag_set = (i >> PIN_FLAG_2_OVERFLOW) & 1;
        bool sign_flag_set = (i >> PIN_FLAG_3_SIGN) & 1;

        uint8_t opcode = (uint8_t)((((i >> PIN_OPCODE_7) & 1) << 7) |
                                   (((i >> PIN_OPCODE_6) & 1) << 6) |
                                   (((i >> PIN_OPCODE_5) & 1) << 5) |
                                   (((i >> PIN_OPCODE_4) & 1) << 4) |
                                   (((i >> PIN_OPCODE_3) & 1) << 3) |
                                   (((i >> PIN_OPCODE_2) & 1) << 2) |
                                   (((i >> PIN_OPCODE_1) & 1) << 1) |
                                   (((i >> PIN_OPCODE_0) & 1) << 0));

        bool is_high_slice = (i >> PIN_HIGH_SLICE) & 1;

        uint16_t active_high_signals =
            signals_from_input(step, zero_flag_set, carry_flag_set, overflow_flag_set, sign_flag_set, opcode);

        uint16_t unmasked_signals = active_high_signals ^ ACTIVE_LOW_MASK;

        (*table)[i] = is_high_slice ? (unmasked_signals >> 8)
                                    : (unmasked_signals & 0xff);
    }
}

static int write_to_file(const char *filename,
                         const uint8_t (*table)[CONTROL_ROM_SIZE]) {
    FILE *file = fopen(filename, "wb");

    if (file == NULL) {
        perror(__func__);
        return 1;
    }

    if (fwrite(*table, sizeof(*table), 1, file) == 0) {
        perror(__func__);
        return 2;
    }

    fclose(file);

    return ferror(file);
}

int main(void) {
    uint8_t table[CONTROL_ROM_SIZE];

    generate_table(&table);

    return write_to_file("./bin/control.bin", &table);
}
