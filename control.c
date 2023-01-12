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

static uint16_t signals_from_input(uint8_t step, bool zero_flag_set, bool carry_flag_set, bool overflow_flag_set, bool sign_flag_set, uint8_t opcode) {
    assert(step < 16);

    switch ((Opcode)opcode) {
    case OPCODE_NOP:
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

    case OPCODE_HALT:
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return HALT_NOT_LD_C;
        case 2: return LD_S_NOT_LD_C;
        }
        break;

    case OPCODE_LD_A_IMM8: // ld a, {imm: i8} => _ @ imm
    case OPCODE_LD_B_IMM8: // ld b, {imm: i8} => _ @ imm
    case OPCODE_LD_C_IMM8: // ld c, {imm: i8} => _ @ imm
    case OPCODE_LD_D_IMM8: // ld d, {imm: i8} => _ @ imm
    {
        uint8_t dest_const_reg = opcode - 0x01;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case OPCODE_LD_I_IMM16: // ld i, {imm: i16} => _ @ le(imm)
    case OPCODE_LD_J_IMM16: // ld j, {imm: i16} => _ @ le(imm)
    {
        uint8_t dest_const_index_l = opcode == 0x05 ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_index_l | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C;
        case 3: return OE_MEM | LD_LS | C_LS_ALU_Q | (dest_const_index_l + 1) | LD_C | TG_M_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x07: // ld a, [i] => 0x07
    case 0x08: // ld a, [j] => 0x08
    {
        uint8_t const_index_l = opcode == 0x07 ? C_IL : C_JL;

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

    case 0x09: // ld a, [i++] => 0x09
    case 0x0a: // ld a, [j++] => 0x0a
    {
        uint8_t const_index_l = opcode == 0x09 ? C_IL : C_JL;

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

    case 0x0b: // ld [i], a => 0x0b
    case 0x0c: // ld [j], a => 0x0c
    {
        uint8_t const_index_l = opcode == 0x0b ? C_IL : C_JL;

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

    case 0x0d: // ld [i++], a => 0x0d
    case 0x0e: // ld [j++], a => 0x0e
    {
        uint8_t const_index_l = opcode == 0x0d ? C_IL : C_JL;

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

    case 0x0f: // ld [i], ab => 0x0f
    case 0x10: // ld [i], cd => 0x10
    case 0x12: // ld [j], cd => 0x12
    {
        uint8_t const_index_l = opcode == 0x12 ? C_JL : C_IL;
        uint8_t src_const_reg_l = opcode == 0x0f ? C_B : C_D;
        uint8_t src_const_reg_h = opcode == 0x0f ? C_A : C_C;

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

    case 0x11: // ld ab, [i] => 0x11
    case 0x13: // ld cd, [i] => 0x13
    case 0x14: // ld cd, [j] => 0x14
    {
        uint8_t const_index_l = opcode == 0x14 ? C_JL : C_IL;
        uint8_t dest_const_reg_l = opcode == 0x11 ? C_B : C_D;
        uint8_t dest_const_reg_h = opcode == 0x11 ? C_A : C_C;

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

    case 0x21: // ld a, b => 0x21
    case 0x22: // ld a, c => 0x22
    case 0x23: // ld a, d => 0x23
    case 0x24: // ld b, a => 0x24
    case 0x26: // ld b, c => 0x26
    case 0x27: // ld b, d => 0x27
    case 0x28: // ld c, a => 0x28
    case 0x29: // ld c, b => 0x29
    case 0x2b: // ld c, d => 0x2b
    case 0x2c: // ld d, a => 0x2c
    case 0x2d: // ld d, b => 0x2d
    case 0x2e: // ld d, c => 0x2e
    {
        uint8_t relative_opcode = opcode - 0x20;

        uint8_t dest_const_reg = relative_opcode >> 2;
        uint8_t src_const_reg = relative_opcode & 3;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return src_const_reg | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 3: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x40: // inc a => 0x40
    case 0x41: // shl a => 0x41
    case 0x42: // shr a => 0x42
    case 0x43: // not a => 0x43
    case 0x44: // dec a => 0x44
    case 0x45: // ror a => 0x45
    case 0x60: // dec b => 0x60
    case 0x61: // dec c => 0x61
    case 0x62: // dec d => 0x62
    case 0x63: // inc b => 0x63
    case 0x64: // inc c => 0x64
    case 0x65: // inc d => 0x65
    {
        uint8_t alu_operation =
            (opcode == 0x60 || opcode == 0x61 || opcode == 0x62)
                ? ALU_OP_DEC_LS
            : (opcode == 0x63 || opcode == 0x64 || opcode == 0x65)
                ? ALU_OP_INC_LS
                : opcode - 0x40;

        uint8_t dest_const_reg =
            (opcode == 0x62 || opcode == 0x65)   ? C_D
            : (opcode == 0x61 || opcode == 0x64) ? C_C
            : (opcode == 0x60 || opcode == 0x63) ? C_B
                                                 : C_A;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return dest_const_reg | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_LS | alu_operation | LD_C;
        case 3: return OE_ALU | LD_LS | C_LS_ALU_Q | dest_const_reg | LD_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x50: // add a, b => 0x50
    case 0x51: // or a, b => 0x51
    case 0x52: // and a, b => 0x52
    case 0x53: // xor a, b => 0x53
    case 0x54: // adc a, b => 0x54
    case 0x66: // add d, b => 0x66
    case 0x67: // adc c, a => 0x67
    {
        uint8_t alu_operation =
            opcode == 0x66   ? ALU_OP_LS_ADD_RS
            : opcode == 0x67 ? ALU_OP_LS_ADC_RS
                             : opcode - 0x50 + ALU_OP_LS_ADD_RS;

        uint8_t dest_const_reg =
            opcode == 0x66   ? C_D
            : opcode == 0x67 ? C_C
                             : C_A;

        uint8_t src_const_reg =
            opcode == 0x66   ? C_B
            : opcode == 0x67 ? C_A
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

    case 0x68: // adc d, {imm: i8} => 0x68
    case 0x6f: // add a, {imm: i8} => 0x6f @ imm
    case 0x70: // or a, {imm: i8} => 0x70 @ imm
    case 0x71: // and a, {imm: i8} => 0x71 @ imm
    case 0x72: // xor a, {imm: i8} => 0x72 @ imm
    case 0x73: // adc a, {imm: i8} => 0x73 @ imm
    case 0x75: // add b, {imm: i8} => 0x75 @ imm
    {
        uint8_t alu_operation =
            (opcode == 0x75)   ? ALU_OP_LS_ADD_RS
            : (opcode == 0x68) ? ALU_OP_LS_ADC_RS
                               : opcode - 0x6f + ALU_OP_LS_ADD_RS;

        uint8_t dest_const_reg =
            (opcode == 0x75)   ? C_B
            : (opcode == 0x68) ? C_D
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

    case 0x76: // cmp a, {imm: i8} => 0x76 @ imm
    case 0x77: // cmp b, {imm: i8} => 0x77 @ imm
    {
        uint8_t const_reg =
            opcode == 0x76 ? C_A
                           : C_B;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_RS_NOT_LD_C | CE_M_NOT_LD_C;
        case 2: return const_reg | LD_C | TG_M_C;
        case 3: return OE_MEM | LD_LS | ALU_OP_LS_SUB_RS | LD_C;
        case 4: return OE_ALU | LD_LS | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x78: // out {port: u3}, {imm: i8} => (0x78 + port)`8 @ imm
    case 0x79:
    case 0x7a:
    case 0x7b:
    case 0x7c:
    case 0x7d:
    case 0x7e:
    case 0x7f: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_IO_NOT_LD_C;
        case 2: return CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x80: // in a, {port: u3} => (0x80 + port)`8
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return ALU_OP_SET_IO_OE_FLAG | LD_C | TG_M_C;
        case 2: return LD_C;
        case 3: return LD_LS | C_LS_ALU_Q | C_A | LD_C;
        case 4: return OE_ALU | LD_MEM | TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x88: // out {port: u3}, a => (0x88 + port)`8
    case 0x89:
    case 0x8a:
    case 0x8b:
    case 0x8c:
    case 0x8d:
    case 0x8e:
    case 0x8f: {
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return C_A | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_IO_NOT_LD_C;
        case 3: return TG_M_C | LD_S_NOT_LD_C;
        }
    } break;

    case 0x90: // jmp i => 0x90
    case 0x91: // jmp j => 0x91
    {
        uint8_t const_index_l = opcode == 0x90 ? C_IL : C_JL;

        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return const_index_l | LD_C | TG_M_C;
        case 2: return OE_MEM | LD_ML | (const_index_l + 1) | LD_C;
        case 3: return OE_MEM | LD_MH | LD_S_NOT_LD_C | TG_M_C;
        }
    } break;

    case 0x92: // jmp {imm: i16} => 0x92 @ le(imm)
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | CE_M_NOT_LD_C;
        case 2: return OE_MEM | LD_MH | C_LS_ALU_Q | LD_C;
        case 3: return OE_ALU | LD_ML | LD_S_NOT_LD_C;
        }
        break;

    case 0x93: // jz  {imm: i16} => 0x93 @ le(imm)
    case 0x94: // jnz {imm: i16} => 0x94 @ le(imm)
    case 0x95: // jc  {imm: i16} => 0x95 @ le(imm)
    case 0x96: // jnc {imm: i16} => 0x96 @ le(imm)
    case 0x97: // jo {imm: i16} => 0x97 @ le(imm)
    case 0x98: // jno {imm: i16} => 0x98 @ le(imm)
    case 0x99: // js {imm: i16} => 0x99 @ le(imm)
    case 0x9a: // jns {imm: i16} => 0x9a @ le(imm)
    {
        bool do_jump = opcode == 0x93   ? zero_flag_set
                       : opcode == 0x94 ? !zero_flag_set

                       : opcode == 0x95 ? carry_flag_set
                       : opcode == 0x96 ? !carry_flag_set

                       : opcode == 0x97 ? overflow_flag_set
                       : opcode == 0x98 ? !overflow_flag_set

                       : opcode == 0x99 ? sign_flag_set
                       : opcode == 0x9a ? !sign_flag_set
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

    case 0xa0: // ld sp, {imm: i8} => 0xa0 @ imm
        switch (step) {
        case 0: return FETCH_OPCODE;
        case 1: return OE_MEM | LD_LS | C_LS_ALU_Q | C_SPL | LD_C | TG_M_C;
        case 2: return OE_ALU | LD_MEM | TG_M_C | CE_M_NOT_LD_C | LD_S_NOT_LD_C;
        }
        break;

    case 0xa2: // push a => 0xa2 (store at ++sp)
    case 0xa3: // push b => 0xa3 (store at ++sp)
    case 0xa4: // push c => 0xa4 (store at ++sp)
    case 0xa5: // push d => 0xa5 (store at ++sp)
    {
        uint8_t src_const_reg = opcode - 0xa2;

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

    case 0xa6: // push i => 0xa6 (store l at ++sp, h at ++sp)
    case 0xa7: // push j => 0xa7 (store l at ++sp, h at ++sp)
    {
        uint8_t src_const_index_l = opcode == 0xa6 ? C_IL : C_JL;

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

    case 0xa8: // pop a => 0xa8 (load from sp--)
    case 0xa9: // pop b => 0xa9 (load from sp--)
    case 0xaa: // pop c => 0xaa (load from sp--)
    case 0xab: // pop d => 0xab (load from sp--)
    {
        uint8_t dest_const_reg = opcode - 0xa8;

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

    case 0xac: // pop i => 0xac (read h at sp--, l at sp--)
    case 0xad: // pop j => 0xad (read h at sp--, l at sp--)
    {
        uint8_t dest_const_index_l = opcode == 0xac ? C_IL : C_JL;

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

    case 0xb0: // call {imm: i16} => 0xb0 @ le(imm) ; (store pc l at ++sp, pc h at ++sp)
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

    case 0xb1: // ret => 0xb1 ; (read pc h at sp--, pc l at sp--)
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

    case 0xb2: // ld a, [sp-{imm:i8}] => 0xb2 @ (-imm)`8
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
