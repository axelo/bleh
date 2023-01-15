#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "opcode.h"

typedef enum {
    NONE,
    PORT,
    PORT_IMM8,
    PORT_A,
    IMM8,
    IMM16
} Operand;

typedef struct {
    const char *n;
    Operand op;
} Rule;

static inline Rule rule(const char *name, Operand operand) {
    return (Rule){.n = name, .op = operand};
}

static Rule rule_from_opcode(Opcode opcode) {
    switch (opcode) {
    case OPCODE_NOP: return rule("nop", NONE);
    case OPCODE_LD_A_IMM8: return rule("ld a,", IMM8);
    case OPCODE_LD_B_IMM8: return rule("ld b,", IMM8);
    case OPCODE_LD_C_IMM8: return rule("ld c,", IMM8);
    case OPCODE_LD_D_IMM8: return rule("ld d,", IMM8);
    case OPCODE_LD_I_IMM16: return rule("ld i,", IMM16);
    case OPCODE_LD_J_IMM16: return rule("ld j,", IMM16);
    case OPCODE_LD_A_I_PTR: return rule("ld a, [i]", NONE);
    case OPCODE_LD_A_J_PTR: return rule("ld a, [j]", NONE);
    case OPCODE_LD_A_I_PTR_INC1: return rule("ld a, [i++]", NONE);
    case OPCODE_LD_A_J_PTR_INC1: return rule("ld a, [j++]", NONE);
    case OPCODE_LD_I_PTR_A: return rule("ld [i], a", NONE);
    case OPCODE_LD_J_PTR_A: return rule("ld [j], a", NONE);
    case OPCODE_LD_I_PTR_INC1_A: return rule("ld [i++], a", NONE);
    case OPCODE_LD_J_PTR_INC1_A: return rule("ld [j++], a", NONE);
    case OPCODE_LD_I_PTR_AB: return rule("ld [i], ab", NONE);
    case OPCODE_LD_I_PTR_CD: return rule("ld [i], cd", NONE);
    case OPCODE_LD_J_PTR_CD: return rule("ld [j], cd", NONE);
    case OPCODE_LD_AB_I_PTR: return rule("ld ab, [i]", NONE);
    case OPCODE_LD_CD_I_PTR: return rule("ld cd, [i]", NONE);
    case OPCODE_LD_CD_J_PTR: return rule("ld cd, [j]", NONE);
    case OPCODE_LD_A_B: return rule("ld a, b", NONE);
    case OPCODE_LD_A_C: return rule("ld a, c", NONE);
    case OPCODE_LD_A_D: return rule("ld a, d", NONE);
    case OPCODE_LD_B_A: return rule("ld b, a", NONE);
    case OPCODE_LD_B_C: return rule("ld b, c", NONE);
    case OPCODE_LD_B_D: return rule("ld b, d", NONE);
    case OPCODE_LD_C_A: return rule("ld c, a", NONE);
    case OPCODE_LD_C_B: return rule("ld c, b", NONE);
    case OPCODE_LD_C_D: return rule("ld c, d", NONE);
    case OPCODE_LD_D_A: return rule("ld d, a", NONE);
    case OPCODE_LD_D_B: return rule("ld d, b", NONE);
    case OPCODE_LD_D_C: return rule("ld d, c", NONE);
    case OPCODE_INC_A: return rule("inc a", NONE);
    case OPCODE_SHL_A: return rule("shl a", NONE);
    case OPCODE_SHR_A: return rule("shr a", NONE);
    case OPCODE_NOT_A: return rule("not a", NONE);
    case OPCODE_DEC_A: return rule("dec a", NONE);
    case OPCODE_ROR_A: return rule("ror a", NONE);
    case OPCODE_ADD_A_B: return rule("add a, b", NONE);
    case OPCODE_OR_A_B: return rule("or a, b", NONE);
    case OPCODE_AND_A_B: return rule("and a, b", NONE);
    case OPCODE_XOR_A_B: return rule("xor a, b", NONE);
    case OPCODE_ADC_A_B: return rule("adc a, b", NONE);
    case OPCODE_DEC_B: return rule("dec b", NONE);
    case OPCODE_DEC_C: return rule("dec c", NONE);
    case OPCODE_DEC_D: return rule("dec d", NONE);
    case OPCODE_INC_B: return rule("inc b", NONE);
    case OPCODE_INC_C: return rule("inc c", NONE);
    case OPCODE_INC_D: return rule("inc d", NONE);
    case OPCODE_ADD_D_B: return rule("add d, b", NONE);
    case OPCODE_ADC_C_A: return rule("adc c, a", NONE);
    case OPCODE_ADC_D_IMM8: return rule("adc d,", IMM8);
    case OPCODE_ADD_A_IMM8: return rule("add a,", IMM8);
    case OPCODE_OR_A_IMM8: return rule("or a,", IMM8);
    case OPCODE_AND_A_IMM8: return rule("and a,", IMM8);
    case OPCODE_XOR_A_IMM8: return rule("xor a,", IMM8);
    case OPCODE_ADC_A_IMM8: return rule("adc a,", IMM8);
    case OPCODE_ADD_B_IMM8: return rule("add b,", IMM8);
    case OPCODE_CMP_A_IMM8: return rule("cmp a,", IMM8);
    case OPCODE_CMP_B_IMM8: return rule("cmp b,", IMM8);
    case OPCODE_OUT_PORT0_IMM8: assert((opcode & 7) == 0); return rule("out", PORT_IMM8);
    case OPCODE_OUT_PORT1_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT0_IMM8);
        assert((opcode & 7) == 1);
        return rule("", NONE);
    case OPCODE_OUT_PORT2_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT1_IMM8);
        assert((opcode & 7) == 2);
        return rule("", NONE);
    case OPCODE_OUT_PORT3_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT2_IMM8);
        assert((opcode & 7) == 3);
        return rule("", NONE);
    case OPCODE_OUT_PORT4_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT3_IMM8);
        assert((opcode & 7) == 4);
        return rule("", NONE);
    case OPCODE_OUT_PORT5_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT4_IMM8);
        assert((opcode & 7) == 5);
        return rule("", NONE);
    case OPCODE_OUT_PORT6_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT5_IMM8);
        assert((opcode & 7) == 6);
        return rule("", NONE);
    case OPCODE_OUT_PORT7_IMM8:
        assert(opcode - 1 == OPCODE_OUT_PORT6_IMM8);
        assert((opcode & 7) == 7);
        return rule("", NONE);
    case OPCODE_IN_A_PORT0: assert((opcode & 7) == 0); return rule("in a,", PORT);
    case OPCODE_IN_A_PORT1:
        assert(opcode - 1 == OPCODE_IN_A_PORT0);
        assert((opcode & 7) == 1);
        return rule("", NONE);
    case OPCODE_IN_A_PORT2:
        assert(opcode - 1 == OPCODE_IN_A_PORT1);
        assert((opcode & 7) == 2);
        return rule("", NONE);
    case OPCODE_IN_A_PORT3:
        assert(opcode - 1 == OPCODE_IN_A_PORT2);
        assert((opcode & 7) == 3);
        return rule("", NONE);
    case OPCODE_IN_A_PORT4:
        assert(opcode - 1 == OPCODE_IN_A_PORT3);
        assert((opcode & 7) == 4);
        return rule("", NONE);
    case OPCODE_IN_A_PORT5:
        assert(opcode - 1 == OPCODE_IN_A_PORT4);
        assert((opcode & 7) == 5);
        return rule("", NONE);
    case OPCODE_IN_A_PORT6:
        assert(opcode - 1 == OPCODE_IN_A_PORT5);
        assert((opcode & 7) == 6);
        return rule("", NONE);
    case OPCODE_IN_A_PORT7:
        assert(opcode - 1 == OPCODE_IN_A_PORT6);
        assert((opcode & 7) == 7);
        return rule("", NONE);
    case OPCODE_OUT_PORT0_A: assert((opcode & 7) == 0); return rule("out", PORT_A);
    case OPCODE_OUT_PORT1_A:
        assert(opcode - 1 == OPCODE_OUT_PORT0_A);
        assert((opcode & 7) == 1);
        return rule("", NONE);
    case OPCODE_OUT_PORT2_A:
        assert(opcode - 1 == OPCODE_OUT_PORT1_A);
        assert((opcode & 7) == 2);
        return rule("", NONE);
    case OPCODE_OUT_PORT3_A:
        assert(opcode - 1 == OPCODE_OUT_PORT2_A);
        assert((opcode & 7) == 3);
        return rule("", NONE);
    case OPCODE_OUT_PORT4_A:
        assert(opcode - 1 == OPCODE_OUT_PORT3_A);
        assert((opcode & 7) == 4);
        return rule("", NONE);
    case OPCODE_OUT_PORT5_A:
        assert(opcode - 1 == OPCODE_OUT_PORT4_A);
        assert((opcode & 7) == 5);
        return rule("", NONE);
    case OPCODE_OUT_PORT6_A:
        assert(opcode - 1 == OPCODE_OUT_PORT5_A);
        assert((opcode & 7) == 6);
        return rule("", NONE);
    case OPCODE_OUT_PORT7_A:
        assert(opcode - 1 == OPCODE_OUT_PORT6_A);
        assert((opcode & 7) == 7);
        return rule("", NONE);
    case OPCODE_JMP_I: return rule("jmp i", NONE);
    case OPCODE_JMP_J: return rule("jmp j", NONE);
    case OPCODE_JMP_IMM16: return rule("jmp", IMM16);
    case OPCODE_JZ_IMM16: return rule("jz", IMM16);
    case OPCODE_JNZ_IMM16: return rule("jnz", IMM16);
    case OPCODE_JC_IMM16: return rule("jc", IMM16);
    case OPCODE_JNC_IMM16: return rule("jnc", IMM16);
    case OPCODE_JO_IMM16: return rule("jo", IMM16);
    case OPCODE_JNO_IMM16: return rule("jno", IMM16);
    case OPCODE_JS_IMM16: return rule("js", IMM16);
    case OPCODE_JNS_IMM16: return rule("jns", IMM16);
    case OPCODE_LD_SP_IMM8: return rule("ld sp,", IMM8);
    case OPCODE_PUSH_A: return rule("push a", NONE);
    case OPCODE_PUSH_B: return rule("push b", NONE);
    case OPCODE_PUSH_C: return rule("push c", NONE);
    case OPCODE_PUSH_D: return rule("push d", NONE);
    case OPCODE_PUSH_I: return rule("push i", NONE);
    case OPCODE_PUSH_J: return rule("push j", NONE);
    case OPCODE_POP_A: return rule("pop a", NONE);
    case OPCODE_POP_B: return rule("pop b", NONE);
    case OPCODE_POP_C: return rule("pop c", NONE);
    case OPCODE_POP_D: return rule("pop d", NONE);
    case OPCODE_POP_I: return rule("pop i", NONE);
    case OPCODE_POP_J: return rule("pop j", NONE);
    case OPCODE_CALL_IMM16: return rule("call", IMM16);
    case OPCODE_RET: return rule("ret", NONE);
    case OPCODE_LD_A_SP_PLUS_IMM8_PTR: return rule("ld [sp+{imm:u8}],", IMM8);
    case OPCODE_HALT: return rule("halt", NONE);
    }

    return rule("; ?", NONE);
}

int main(void) {
    FILE *file = fopen("bleh_instructions.asm", "w");
    assert(file != NULL);

    time_t current_time;
    time(&current_time);

    fprintf(file, "; Generated %s\n", ctime(&current_time));

    fprintf(file, "#bits 8\n\n#ruledef {\n");

    for (Opcode opcode = 0; opcode < 0x100; ++opcode) {
        Rule r = rule_from_opcode(opcode);
        if (r.n[0] != '\0') {
            fprintf(file, "    %s%s => (0x%02x%s\n",
                    r.n,
                    r.op == NONE        ? ""
                    : r.op == PORT      ? " {port: u3}"
                    : r.op == PORT_IMM8 ? " {port: u3}, {imm: i8}"
                    : r.op == PORT_A    ? " {port: u3}, a"
                    : r.op == IMM8      ? " {imm: i8}"
                    : r.op == IMM16     ? " {imm: i16}"
                                        : "",
                    opcode,
                    r.op == NONE        ? ")"
                    : r.op == PORT      ? " + port)`8"
                    : r.op == PORT_IMM8 ? " + port)`8 @ imm"
                    : r.op == PORT_A    ? " + port)`8"
                    : r.op == IMM8      ? ") @ imm"
                    : r.op == IMM16     ? ") @ le(imm)"
                                        : ")");
        }
    }

    fprintf(file, "}\n\n");

    fflush(file);
    assert(fclose(file) == 0 && "Failed to close file");

    return 0;
}
