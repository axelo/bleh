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

static uint16_t signals_from_input(uint8_t step, bool zero_flag_set, bool carry_flag_set, bool overflow_flag_set, bool sign_flag_set, Opcode opcode) {
    assert(step < 16);

    switch (opcode) {
    case OPCODE_NOP: // nop
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return 0;
        case 2: return 0;
        case 3: return 0;
        case 4: return 0;
        case 5: return 0;
        case 6: return 0;
        case 7: return LD_S_NOT_LD_C;
        }
        break;

    case OPCODE_HALT: // halt
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return HALT_NOT_LD_C;
        case 2: return LD_S_NOT_LD_C;
        }
        break;

    case OPCODE_LD_A_IMM8: // ld a, {imm: i8}
    case OPCODE_LD_B_IMM8: // ld b, {imm: i8}
    case OPCODE_LD_C_IMM8: // ld c, {imm: i8}
    case OPCODE_LD_D_IMM8: // ld d, {imm: i8}
    {
        uint8_t dest_const_reg = (uint8_t)opcode - OPCODE_LD_A_IMM8;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_I_IMM16: // ld i, {imm: i16}
    case OPCODE_LD_J_IMM16: // ld j, {imm: i16}
    {
        uint8_t dest_const_index_l = opcode == OPCODE_LD_I_IMM16 ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_index_l | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C;
        case 3: return OE_MEM | LD_LS | C_LS_ALU_Q | (dest_const_index_l + 1) | LD_C | TG_M_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_A_I_PTR: // ld a, [i]
    case OPCODE_LD_A_J_PTR: // ld a, [j]
    {
        uint8_t const_index_l = opcode == OPCODE_LD_A_I_PTR ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_TL | LD_C | TG_M_C;
        case 2: return OE_ML | LD_MEM | C_TH | LD_C;
        case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
        case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
        case 5: return OE_MEM | LD_MH | TG_M_C;
        case 6: return OE_MEM | LD_LS | C_LS_ALU_Q | C_A | LD_C | TG_M_C;
        case 7: return OE_ALU | LD_MEM | C_TL | LD_C;
        case 8: return OE_MEM | LD_ML | C_TH | LD_C;
        case 9: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_A_I_PTR_INC1: // ld a, [i++]
    case OPCODE_LD_A_J_PTR_INC1: // ld a, [j++]
    {
        uint8_t const_index_l = opcode == OPCODE_LD_A_I_PTR_INC1 ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_TL | LD_C | TG_M_C;
        case 2: return OE_ML | LD_MEM | C_TH | LD_C;
        case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
        case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
        case 5: return OE_MEM | LD_MH | TG_M_C;
        case 6: return OE_MEM | LD_LS | CE_M_NOT_LD_C | TG_M_C;
        case 7: return OE_MH | LD_MEM | const_index_l | LD_C;
        case 8: return OE_ML | LD_MEM | C_LS_ALU_Q | C_A | LD_C;
        case 9: return OE_ALU | LD_MEM | C_TL | LD_C;
        case 10: return OE_MEM | LD_ML | C_TH | LD_C;
        case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_I_PTR_A: // ld [i], a
    case OPCODE_LD_J_PTR_A: // ld [j], a
    {
        uint8_t const_index_l = opcode == OPCODE_LD_I_PTR_A ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_TL | LD_C | TG_M_C;
        case 2: return OE_ML | LD_MEM | C_TH | LD_C;
        case 3: return OE_MH | LD_MEM | const_index_l | LD_C;
        case 4: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
        case 5: return OE_MEM | LD_MH | C_LS_ALU_Q | C_A | LD_C;
        case 6: return OE_MEM | LD_LS | TG_M_C;
        case 7: return OE_ALU | LD_MEM | C_TL | LD_C | TG_M_C;
        case 8: return OE_MEM | LD_ML | C_TH | LD_C;
        case 9: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_I_PTR_INC1_A: // ld [i++], a
    case OPCODE_LD_J_PTR_INC1_A: // ld [j++], a
    {
        uint8_t const_index_l = opcode == OPCODE_LD_I_PTR_INC1_A ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_A | LD_C | TG_M_C;
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
        }
    } break;

    case OPCODE_LD_I_PTR_AB: // ld [i], ab
    case OPCODE_LD_I_PTR_CD: // ld [i], cd
    case OPCODE_LD_J_PTR_CD: // ld [j], cd
    {
        uint8_t const_index_l = opcode == OPCODE_LD_J_PTR_CD ? C_JL : C_IL;
        uint8_t src_const_reg_l = opcode == OPCODE_LD_I_PTR_AB ? C_B : C_D;
        uint8_t src_const_reg_h = opcode == OPCODE_LD_I_PTR_AB ? C_A : C_C;

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
        }
    } break;

    case OPCODE_LD_AB_I_PTR: // ld ab, [i]
    case OPCODE_LD_CD_I_PTR: // ld cd, [i]
    case OPCODE_LD_CD_J_PTR: // ld cd, [j]
    {
        uint8_t const_index_l = opcode == OPCODE_LD_CD_J_PTR ? C_JL : C_IL;
        uint8_t dest_const_reg_l = opcode == OPCODE_LD_AB_I_PTR ? C_B : C_D;
        uint8_t dest_const_reg_h = opcode == OPCODE_LD_AB_I_PTR ? C_A : C_C;

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
        }
    } break;

    case OPCODE_LD_A_B: // ld a, b
    case OPCODE_LD_A_C: // ld a, c
    case OPCODE_LD_A_D: // ld a, d
    case OPCODE_LD_B_A: // ld b, a
    case OPCODE_LD_B_C: // ld b, c
    case OPCODE_LD_B_D: // ld b, d
    case OPCODE_LD_C_A: // ld c, a
    case OPCODE_LD_C_B: // ld c, b
    case OPCODE_LD_C_D: // ld c, d
    case OPCODE_LD_D_A: // ld d, a
    case OPCODE_LD_D_B: // ld d, b
    case OPCODE_LD_D_C: // ld d, c
    {
        uint8_t relative_opcode = (uint8_t)opcode - (OPCODE_LD_A_B - 1);

        uint8_t dest_const_reg = relative_opcode >> 2;
        uint8_t src_const_reg = relative_opcode & 3;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return src_const_reg | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 3: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_INC_A: // inc a
    case OPCODE_SHL_A: // shl a
    case OPCODE_SHR_A: // shr a
    case OPCODE_NOT_A: // not a
    case OPCODE_DEC_A: // dec a
    case OPCODE_ROR_A: // ror a
    case OPCODE_DEC_B: // dec b
    case OPCODE_DEC_C: // dec c
    case OPCODE_DEC_D: // dec d
    case OPCODE_INC_B: // inc b
    case OPCODE_INC_C: // inc c
    case OPCODE_INC_D: // inc d
    {
        uint8_t alu_operation =
            (opcode == OPCODE_DEC_B || opcode == OPCODE_DEC_C || opcode == OPCODE_DEC_D)
                ? ALU_OP_DEC_LS
            : (opcode == OPCODE_INC_B || opcode == OPCODE_INC_C || opcode == OPCODE_INC_D)
                ? ALU_OP_INC_LS
                : (uint8_t)opcode - OPCODE_INC_A;

        uint8_t dest_const_reg =
            (opcode == OPCODE_DEC_D || opcode == OPCODE_INC_D)   ? C_D
            : (opcode == OPCODE_DEC_C || opcode == OPCODE_INC_C) ? C_C
            : (opcode == OPCODE_DEC_B || opcode == OPCODE_INC_B) ? C_B
                                                                 : C_A;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return dest_const_reg | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_LS | alu_operation | LD_C;
        case 3: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_ADD_A_B: // add a, b
    case OPCODE_OR_A_B: // or a, b
    case OPCODE_AND_A_B: // and a, b
    case OPCODE_XOR_A_B: // xor a, b
    case OPCODE_ADC_A_B: // adc a, b
    case OPCODE_ADD_D_B: // add d, b
    case OPCODE_ADC_C_A: // adc c, a
    {
        uint8_t alu_operation =
            opcode == OPCODE_ADD_D_B   ? ALU_OP_LS_ADD_RS
            : opcode == OPCODE_ADC_C_A ? ALU_OP_LS_ADC_RS
                                       : (uint8_t)opcode - OPCODE_ADD_A_B + ALU_OP_LS_ADD_RS;

        uint8_t dest_const_reg =
            opcode == OPCODE_ADD_D_B   ? C_D
            : opcode == OPCODE_ADC_C_A ? C_C
                                       : C_A;

        uint8_t src_const_reg =
            opcode == OPCODE_ADD_D_B   ? C_B
            : opcode == OPCODE_ADC_C_A ? C_A
                                       : C_B;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return dest_const_reg | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_LS | src_const_reg | LD_C;
        case 3: return OE_MEM | LD_RS_NOT_LD_C;
        case 4: return alu_operation | LD_C;
        case 5: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 6: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_ADC_D_IMM8: // adc d, {imm: i8}
    case OPCODE_ADD_A_IMM8: // add a, {imm: i8}
    case OPCODE_OR_A_IMM8: // or a, {imm: i8}
    case OPCODE_AND_A_IMM8: // and a, {imm: i8}
    case OPCODE_XOR_A_IMM8: // xor a, {imm: i8}
    case OPCODE_ADC_A_IMM8: // adc a, {imm: i8}
    case OPCODE_ADD_B_IMM8: // add b, {imm: i8}
    {
        uint8_t alu_operation =
            (opcode == OPCODE_ADD_B_IMM8)   ? ALU_OP_LS_ADD_RS
            : (opcode == OPCODE_ADC_D_IMM8) ? ALU_OP_LS_ADC_RS
                                            : (uint8_t)opcode - OPCODE_ADD_A_IMM8 + ALU_OP_LS_ADD_RS;

        uint8_t dest_const_reg =
            (opcode == OPCODE_ADD_B_IMM8)   ? C_B
            : (opcode == OPCODE_ADC_D_IMM8) ? C_D
                                            : C_A;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C;
        case 2: return dest_const_reg | LD_C | TG_M_C;
        case 3: return OE_MEM | LD_LS | alu_operation | LD_C;
        case 4: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 5: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_CMP_A_IMM8: // cmp a, {imm: i8}
    case OPCODE_CMP_B_IMM8: // cmp b, {imm: i8}
    {
        uint8_t const_reg =
            opcode == OPCODE_CMP_A_IMM8 ? C_A
                                        : C_B;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C;
        case 2: return const_reg | LD_C | TG_M_C;
        case 3: return OE_MEM | LD_LS | ALU_OP_LS_SUB_RS | LD_C;
        case 4: return OE_ALU | LD_LS | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_OUT_PORT0_IMM8: // out {port: u3}, {imm: i8}
    case OPCODE_OUT_PORT1_IMM8:
    case OPCODE_OUT_PORT2_IMM8:
    case OPCODE_OUT_PORT3_IMM8:
    case OPCODE_OUT_PORT4_IMM8:
    case OPCODE_OUT_PORT5_IMM8:
    case OPCODE_OUT_PORT6_IMM8:
    case OPCODE_OUT_PORT7_IMM8: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_IO_NOT_LD_C;
        case 2: return CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_IN_A_PORT0: // in a, {port: u3}
    case OPCODE_IN_A_PORT1:
    case OPCODE_IN_A_PORT2:
    case OPCODE_IN_A_PORT3:
    case OPCODE_IN_A_PORT4:
    case OPCODE_IN_A_PORT5:
    case OPCODE_IN_A_PORT6:
    case OPCODE_IN_A_PORT7: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return ALU_OP_SET_IO_OE_FLAG | LD_C | TG_M_C;
        case 2: return LD_C;
        case 3: return LD_LS | C_LS_ALU_Q | C_A | LD_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_OUT_PORT0_A: // out {port: u3}, a
    case OPCODE_OUT_PORT1_A:
    case OPCODE_OUT_PORT2_A:
    case OPCODE_OUT_PORT3_A:
    case OPCODE_OUT_PORT4_A:
    case OPCODE_OUT_PORT5_A:
    case OPCODE_OUT_PORT6_A:
    case OPCODE_OUT_PORT7_A: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_A | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_IO_NOT_LD_C;
        case 3: return TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_JMP_I: // jmp i
    case OPCODE_JMP_J: // jmp j
    {
        uint8_t const_index_l = opcode == OPCODE_JMP_I ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return const_index_l | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
        case 3: return OE_MEM | LD_MH | LD_S_NOT_LD_C | TG_M_C;
        }
    } break;

    case OPCODE_JMP_IMM16: // jmp {imm: i16}
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | CE_M_NOT_LD_C;
        case 2: return OE_MEM | LD_MH | C_LS_ALU_Q | LD_C;
        case 3: return OE_ALU | LD_ML | LD_S_NOT_LD_C;
        }
        break;

    case OPCODE_JZ_IMM16: // jz  {imm: i16}
    case OPCODE_JNZ_IMM16: // jnz {imm: i16}
    case OPCODE_JC_IMM16: // jc  {imm: i16}
    case OPCODE_JNC_IMM16: // jnc {imm: i16}
    case OPCODE_JO_IMM16: // jo {imm: i16}
    case OPCODE_JNO_IMM16: // jno {imm: i16}
    case OPCODE_JS_IMM16: // js {imm: i16}
    case OPCODE_JNS_IMM16: // jns {imm: i16}
    {
        bool do_jump = opcode == OPCODE_JZ_IMM16    ? zero_flag_set
                       : opcode == OPCODE_JNZ_IMM16 ? !zero_flag_set

                       : opcode == OPCODE_JC_IMM16  ? carry_flag_set
                       : opcode == OPCODE_JNC_IMM16 ? !carry_flag_set

                       : opcode == OPCODE_JO_IMM16  ? overflow_flag_set
                       : opcode == OPCODE_JNO_IMM16 ? !overflow_flag_set

                       : opcode == OPCODE_JS_IMM16  ? sign_flag_set
                       : opcode == OPCODE_JNS_IMM16 ? !sign_flag_set
                                                    : false;

        if (do_jump) {
            switch (step) {
            case 0: return FETCH_OPCODE;
            case 1: return OE_MEM | LD_LS | CE_M_NOT_LD_C;
            case 2: return OE_MEM | LD_MH | C_LS_ALU_Q | LD_C;
            case 3: return OE_ALU | LD_ML | LD_S_NOT_LD_C;
            }
        } else {
            switch (step) {
            case 0: return FETCH_OPCODE;
            case 1: return CE_M_NOT_LD_C;
            case 2: return CE_M_NOT_LD_C;
            case 3: return LD_S_NOT_LD_C;
            }
        }
    } break;

    case OPCODE_LD_SP_IMM8: // ld sp, {imm: i8}
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | C_SPL | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
        break;

    case OPCODE_PUSH_A: // push a (store at ++sp)
    case OPCODE_PUSH_B: // push b (store at ++sp)
    case OPCODE_PUSH_C: // push c (store at ++sp)
    case OPCODE_PUSH_D: // push d (store at ++sp)
    {
        uint8_t src_const_reg = (uint8_t)opcode - OPCODE_PUSH_A;

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
        }
    } break;

    case OPCODE_PUSH_I: // push i (store l at ++sp, h at ++sp)
    case OPCODE_PUSH_J: // push j (store l at ++sp, h at ++sp)
    {
        uint8_t src_const_index_l = opcode == OPCODE_PUSH_I ? C_IL : C_JL;

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
        }
    } break;

    case OPCODE_POP_A: // pop a (load from sp--)
    case OPCODE_POP_B: // pop b (load from sp--)
    case OPCODE_POP_C: // pop c (load from sp--)
    case OPCODE_POP_D: // pop d (load from sp--)
    {
        uint8_t dest_const_reg = (uint8_t)opcode - OPCODE_POP_A;

        assert(ALU_OP_DEC_LS == C_SPL && "C_SPL must be identical to ALU_OP_DEC_LS");

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_TL | LD_C | TG_M_C;
        case 2: return OE_ML | LD_MEM | C_TH | LD_C;
        case 3: return OE_MH | LD_MEM | C_SPL | LD_C;
        case 4: return OE_MEM | LD_ML | LD_LS; // | ALU_OP_DEC_LS | LD_C;
        case 5: return OE_ALU | LD_MEM;
        case 6: return LD_MH | TG_M_C;
        case 7: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
        case 8: return OE_ALU | LD_MEM | C_TL | LD_C;
        case 9: return OE_MEM | LD_ML | C_TH | LD_C;
        case 10: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_POP_I: // pop i (read h at sp--, l at sp--)
    case OPCODE_POP_J: // pop j (read h at sp--, l at sp--)
    {
        uint8_t dest_const_index_l = opcode == OPCODE_POP_I ? C_IL : C_JL;

        assert(ALU_OP_DEC_LS == C_SPL && "C_SPL must be identical to ALU_OP_DEC_LS");

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_TL | LD_C | TG_M_C;
        case 2: return OE_ML | LD_MEM | C_TH | LD_C;
        case 3: return OE_MH | LD_MEM | C_SPL | LD_C;
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
        }
    } break;

    case OPCODE_CALL_IMM16: // call {imm: i16} (store pc l at ++sp, pc h at ++sp)
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
        }
        break;

    case OPCODE_RET: // ret (read pc h at sp--, pc l at sp--)
    {
        assert(ALU_OP_DEC_LS == C_SPL && "C_SPL must be identical to ALU_OP_DEC_LS");

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_SPL | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_ML | LD_LS;
        case 3: return OE_ALU | LD_MEM; // --sp
        case 4: return LD_MH | TG_M_C;
        case 5: return OE_MEM | LD_LS | C_LS_ALU_Q | C_TH | LD_C | TG_M_C;
        case 6: return OE_ALU | LD_MEM | C_SPL | LD_C;
        case 7: return OE_MEM | LD_ML | LD_LS;
        case 8: return OE_ALU | LD_MEM | TG_M_C; // --sp
        case 9: return OE_MEM | LD_ML | C_TH | LD_C | TG_M_C;
        case 10: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_A_SP_PLUS_IMM8_PTR: // ld a, [sp+{imm:i8}]
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C; // RS = -imm
        case 2: return C_TL | LD_C | TG_M_C;
        case 3: return OE_ML | LD_MEM | C_TH | LD_C;
        case 4: return OE_MH | LD_MEM | C_SPL | LD_C;
        case 5: return OE_MEM | LD_LS | ALU_OP_LS_ADD_RS | LD_C; // LS = sp
        case 6: return OE_ALU | LD_ML; // ML = (sp + (-imm)) & 0xff
        case 7: return LD_MH | TG_M_C;
        case 8: return OE_MEM | LD_LS | C_LS_ALU_Q | C_A | LD_C | TG_M_C;
        case 9: return OE_ALU | LD_MEM | C_TL | LD_C;
        case 10: return OE_MEM | LD_ML | C_TH | LD_C;
        case 11: return OE_MEM | LD_MH | TG_M_C | LD_S_NOT_LD_C;
        }
        break;
    }

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
