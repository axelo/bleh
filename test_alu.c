#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // f*

#include "alu_op.h"

#define ALU_ROM_SIZE (1 << 17)

// ALU low signals
#define ALU_SIGNAL_L_QZ(alu_signals) (((alu_signals) >> 0) & 1)
#define ALU_SIGNAL_L_QC(alu_signals) (((alu_signals) >> 1) & 1)
#define ALU_SIGNAL_Q_ZF(alu_signals) (((alu_signals) >> 2) & 1)
#define ALU_SIGNAL_Q_IO_OE(alu_signals) (((alu_signals) >> 3) & 1)

// ALU high signals
#define ALU_SIGNAL_H_QZ(alu_signals) (((alu_signals) >> 8) & 1)
#define ALU_SIGNAL_H_QC(alu_signals) (((alu_signals) >> 9) & 1)
#define ALU_SIGNAL_Q_CF(alu_signals) (((alu_signals) >> 10) & 1)
#define ALU_SIGNAL_Q_OF(alu_signals) (((alu_signals) >> 11) & 1)

// ALU combined signals
#define ALU_SIGNAL_Q(alu_signals) ((((alu_signals) >> 8) & 0xf0) | ((alu_signals) >> 4) & 0x0f)
#define ALU_SIGNAL_Q_SF(alu_signals) ((ALU_SIGNAL_Q(alu_signals) >> 7) & 1)

#define ALU_OP_0(alu_op) (((alu_op) >> 0) & 1)
#define ALU_OP_1(alu_op) (((alu_op) >> 1) & 1)
#define ALU_OP_2(alu_op) (((alu_op) >> 2) & 1)
#define ALU_OP_3(alu_op) (((alu_op) >> 3) & 1)
#define ALU_OP_4(alu_op) (((alu_op) >> 4) & 1)
#define ALU_OP_5(alu_op) (((alu_op) >> 5) & 1)

#define EXPECT_ZF_SET (1 << 1)
#define EXPECT_ZF_CLEARED (1 << 2)
#define EXPECT_CF_SET (1 << 3)
#define EXPECT_CF_CLEARED (1 << 4)
#define EXPECT_OF_SET (1 << 5)
#define EXPECT_OF_CLEARED (1 << 6)

typedef struct {
    uint8_t ls;
    uint8_t rs;
    uint8_t in_cf;
    uint8_t i;
    uint16_t alu_signals;
} ALU_Result;

typedef struct {
    uint8_t flags;
    uint8_t q;
} Expect;

static uint8_t alu_low_rom[ALU_ROM_SIZE];
static uint8_t alu_high_rom[ALU_ROM_SIZE];

static inline const char *alu_op_to_string(ALU_OP alu_op) {
    switch (alu_op) {
    case ALU_OP_INC_LS: return "ALU_OP_INC_LS";
    case ALU_OP_SHL_LS: return "ALU_OP_SHL_LS";
    case ALU_OP_SHR_LS: return "ALU_OP_SHR_LS";
    case ALU_OP_NOT_LS: return "ALU_OP_NOT_LS";
    case ALU_OP_DEC_LS: return "ALU_OP_DEC_LS";
    case ALU_OP_ROR_LS: return "ALU_OP_ROR_LS";
    case ALU_OP_LS_ADD_RS: return "ALU_OP_LS_ADD_RS";
    case ALU_OP_LS_OR_RS: return "ALU_OP_LS_OR_RS";
    case ALU_OP_LS_AND_RS: return "ALU_OP_LS_AND_RS";
    case ALU_OP_LS_XOR_RS: return "ALU_OP_LS_XOR_RS";
    case ALU_OP_LS_ADC_RS: return "ALU_OP_LS_ADC_RS";
    case ALU_OP_LS_SUB_RS: return "ALU_OP_LS_SUB_RS";
    case ALU_OP_SET_IO_OE_FLAG: return "ALU_OP_SET_IO_OE_FLAG";
    }
}

static void inline print_unexpected_flag(ALU_OP alu_op, ALU_Result r, const char *expected_flag_string) {
    fprintf(stderr, "%s (%d) NOT OK!\n"
                    "  Expected %s\n"
                    "    LS: 0x%x (%d) RS: 0x%x (%d) IN CF: %d i: %d ALU Q: 0x%x (%d)\n"
                    "    ALU ZF: %d ALU CF: %d ALU OF: %d ALU SF: %d\n",
            alu_op_to_string(alu_op), alu_op,
            expected_flag_string,
            r.ls, r.ls, r.rs, r.rs, r.in_cf, r.i,
            ALU_SIGNAL_Q(r.alu_signals), ALU_SIGNAL_Q(r.alu_signals),
            ALU_SIGNAL_Q_ZF(r.alu_signals),
            ALU_SIGNAL_Q_CF(r.alu_signals),
            ALU_SIGNAL_Q_OF(r.alu_signals),
            ALU_SIGNAL_Q_SF(r.alu_signals));
}

static void test_alu_op(ALU_OP alu_op, Expect (*expect_fn)(ALU_Result)) {
    bool success = true;

    for (uint16_t ls = 0; ls < 0x100; ++ls) {
        for (uint16_t rs = 0; rs < 0x100; ++rs) {
            for (uint8_t in_cf = 0; in_cf < 2; ++in_cf) {
                uint16_t alu_signals = 0;

                uint8_t i = 0;
                for (; i < 10; ++i) {
                    uint32_t alu_l_address = (ALU_OP_5(alu_op) << 16) |
                                             (ALU_SIGNAL_H_QC((uint32_t)alu_signals) << 15) |
                                             (ALU_OP_4(alu_op) << 14) | (ALU_OP_3(alu_op) << 13) |
                                             (ALU_SIGNAL_H_QZ((uint32_t)alu_signals) << 12) |
                                             (ALU_OP_2(alu_op) << 11) | (ALU_OP_1(alu_op) << 10) | (ALU_OP_0(alu_op) << 9) |
                                             (((uint32_t)in_cf & 1) << 8) |
                                             (((uint32_t)rs & 0xf) << 4) |
                                             (ls & 0xf);

                    uint32_t alu_h_address = (ALU_OP_5(alu_op) << 16) |
                                             (ALU_SIGNAL_L_QC((uint32_t)alu_signals) << 15) |
                                             (ALU_OP_4(alu_op) << 14) | (ALU_OP_3(alu_op) << 13) |
                                             (ALU_SIGNAL_L_QZ((uint32_t)alu_signals) << 12) |
                                             (ALU_OP_2(alu_op) << 11) | (ALU_OP_1(alu_op) << 10) | (ALU_OP_0(alu_op) << 9) |
                                             (((uint32_t)in_cf & 1) << 8) |
                                             (((uint32_t)rs >> 4) << 4) |
                                             (ls >> 4);

                    uint16_t next_alu_signals =
                        (uint16_t)((alu_high_rom[alu_h_address] << 8) |
                                   alu_low_rom[alu_l_address]);

                    if (next_alu_signals == alu_signals) {
                        ALU_Result alu_result = {
                            .ls = (uint8_t)ls,
                            .rs = (uint8_t)rs,
                            .in_cf = in_cf,
                            .i = i,
                            .alu_signals = alu_signals,
                        };

                        Expect expect = expect_fn(alu_result);

                        if (expect.q != ALU_SIGNAL_Q(alu_signals)) {
                            fprintf(stderr, "%s (%d) NOT OK!\n", alu_op_to_string(alu_op), alu_op);
                            fprintf(stderr, "  Expected Q 0x%x (%d) got 0x%x (%d)\n", expect.q, expect.q, ALU_SIGNAL_Q(alu_signals), ALU_SIGNAL_Q(alu_signals));
                            fprintf(stderr, "    LS: 0x%x (%d) RS: 0x%x (%d) IN CF: %d i: %d\n", ls, ls, rs, rs, in_cf, i);
                            success = false;
                        }

                        if ((expect.flags & EXPECT_ZF_SET) && !ALU_SIGNAL_Q_ZF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "ZF set");
                            success = false;
                        }

                        if ((expect.flags & EXPECT_ZF_CLEARED) && ALU_SIGNAL_Q_ZF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "ZF cleared");
                            success = false;
                        }

                        if ((expect.flags & EXPECT_CF_SET) && !ALU_SIGNAL_Q_CF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "CF set");
                            success = false;
                        }

                        if ((expect.flags & EXPECT_CF_CLEARED) && ALU_SIGNAL_Q_CF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "CF cleared");
                            success = false;
                        }

                        if ((expect.flags & EXPECT_OF_SET) && !ALU_SIGNAL_Q_OF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "OF set");
                            success = false;
                        }

                        if ((expect.flags & EXPECT_OF_CLEARED) && ALU_SIGNAL_Q_OF(alu_signals)) {
                            print_unexpected_flag(alu_op, alu_result, "OF cleared");
                            success = false;
                        }

                        break;
                    } else {
                        alu_signals = next_alu_signals;
                    }
                }

                assert(success);

                assert(i < 4 && "ALU signals never settled");
            }
        }
    }

    if (success) {
        printf("%s (%d) OK!\n", alu_op_to_string(alu_op), alu_op);
    } else {
        assert(false);
    }
}

static Expect expect_inc(ALU_Result r) {
    Expect expect = {.q = (uint8_t)(r.ls + 1)};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= r.in_cf ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= (!(r.ls >> 7) && (ALU_SIGNAL_Q(r.alu_signals) >> 7)) ? EXPECT_OF_SET : EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_shl(ALU_Result r) {
    uint16_t expect_q = (uint16_t)(r.ls << 1);
    Expect expect = {.q = (uint8_t)expect_q};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= (expect_q & 0x100) ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_shr(ALU_Result r) {
    Expect expect = {.q = r.ls >> 1};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= (r.ls & 1) ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_not(ALU_Result r) {
    Expect expect = {.q = (uint8_t)~r.ls};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_dec(ALU_Result r) {
    Expect expect = {.q = (uint8_t)(r.ls - 1)};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= r.in_cf ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= ((r.ls >> 7) && !(ALU_SIGNAL_Q(r.alu_signals) >> 7)) ? EXPECT_OF_SET : EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_ror(ALU_Result r) {
    Expect expect = {.q = (uint8_t)(((r.ls & 1) << 7) | r.ls >> 1)};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= (r.ls & 1) ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_add(ALU_Result r) {
    uint16_t expect_q = r.ls + r.rs;
    Expect expect = {.q = (uint8_t)expect_q};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= expect_q > 0xff ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= (((r.ls >> 7) && (r.rs >> 7) && !(ALU_SIGNAL_Q(r.alu_signals) >> 7)) ||
                     (!(r.ls >> 7) && !(r.rs >> 7) && (ALU_SIGNAL_Q(r.alu_signals) >> 7)))
                        ? EXPECT_OF_SET
                        : EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_or(ALU_Result r) {
    Expect expect = {.q = r.ls | r.rs};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_and(ALU_Result r) {
    Expect expect = {.q = r.ls & r.rs};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_xor(ALU_Result r) {
    Expect expect = {.q = r.ls ^ r.rs};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= EXPECT_CF_CLEARED;
    expect.flags |= EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_adc(ALU_Result r) {
    uint16_t expect_q = r.ls + r.rs + r.in_cf;
    Expect expect = {.q = (uint8_t)expect_q};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= expect_q > 0xff ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= (((r.ls >> 7) && (r.rs >> 7) && !(ALU_SIGNAL_Q(r.alu_signals) >> 7)) ||
                     (!(r.ls >> 7) && !(r.rs >> 7) && (ALU_SIGNAL_Q(r.alu_signals) >> 7)))
                        ? EXPECT_OF_SET
                        : EXPECT_OF_CLEARED;

    return expect;
}

static Expect expect_sub(ALU_Result r) {
    uint16_t expect_q = (uint16_t)(r.ls - r.rs);
    Expect expect = {.q = (uint8_t)expect_q};

    expect.flags |= ALU_SIGNAL_Q(r.alu_signals) == 0 ? EXPECT_ZF_SET : EXPECT_ZF_CLEARED;
    expect.flags |= (expect_q & 0x100) ? EXPECT_CF_SET : EXPECT_CF_CLEARED;
    expect.flags |= ((!(r.ls >> 7) && (r.rs >> 7) && (ALU_SIGNAL_Q(r.alu_signals) >> 7)) ||
                     ((r.ls >> 7) && !(r.rs >> 7) && !(ALU_SIGNAL_Q(r.alu_signals) >> 7)))
                        ? EXPECT_OF_SET
                        : EXPECT_OF_CLEARED;

    return expect;
}

int main(void) {
    FILE *file = fopen("./bin/alu_low.bin", "r");
    assert(file != NULL && "Failed to read alu_low.bin");
    size_t read_bytes = fread(alu_low_rom, sizeof(uint8_t), ALU_ROM_SIZE, file);
    assert(read_bytes == ALU_ROM_SIZE && "Failed to read the entire contents of alu_low.bin");
    assert(fclose(file) == 0 && "Failed to close file");

    file = fopen("./bin/alu_high.bin", "r");
    assert(file != NULL && "Failed to read alu_high.bin");
    read_bytes = fread(alu_high_rom, sizeof(uint8_t), ALU_ROM_SIZE, file);
    assert(read_bytes == ALU_ROM_SIZE && "Failed to read the entire contents of alu_high.bin");
    assert(fclose(file) == 0 && "Failed to close file");

    test_alu_op(ALU_OP_INC_LS, expect_inc);
    test_alu_op(ALU_OP_SHL_LS, expect_shl);
    test_alu_op(ALU_OP_SHR_LS, expect_shr);
    test_alu_op(ALU_OP_NOT_LS, expect_not);
    test_alu_op(ALU_OP_DEC_LS, expect_dec);
    test_alu_op(ALU_OP_ROR_LS, expect_ror);

    test_alu_op(ALU_OP_LS_ADD_RS, expect_add);
    test_alu_op(ALU_OP_LS_OR_RS, expect_or);
    test_alu_op(ALU_OP_LS_AND_RS, expect_and);
    test_alu_op(ALU_OP_LS_XOR_RS, expect_xor);
    test_alu_op(ALU_OP_LS_ADC_RS, expect_adc);
    test_alu_op(ALU_OP_LS_SUB_RS, expect_sub);

    return 0;
}
