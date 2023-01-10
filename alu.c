#include <stdint.h> // uint8_t.
#include <stdio.h> // FILE, f* functions.

#include "alu_op.h"

#define ROM_SIZE (1 << 17)

#define HALF_ZERO_FLAG_SET 0x1
#define HALF_CARRY_FLAG_SET 0x2
#define ZERO_FLAG_SET 0x4 // Only for low slice
#define IO_OE_FLAG_SET 0x8 // Only for low slice
#define CARRY_FLAG_SET 0x4 // Only for high slice
#define OVERFLOW_FLAG_SET 0x8 // Only for high slice

// Overflow flag reference:
// http://teaching.idallen.com/dat2343/10f/notes/040_overflow.txt

static uint8_t output_from_address(uint32_t i, uint8_t is_higher_half) {
    uint8_t half_A = (i >> 0) & 0xf;
    uint8_t half_B = (i >> 4) & 0xf;
    uint8_t global_carry = (i >> 8) & 1;
    ALU_OP operation = ((i >> 9) & 0x7) | (((i >> 13) & 0x3) << 3) | ((i >> 16) << 5); // 6 bits
    uint8_t other_half_zero = (i >> 12) & 1;
    uint8_t other_half_carry = (i >> 15) & 1;

    if (operation >= 0x20) {
        return (uint8_t)(half_A << 4);
    }

    ALU_OP op = operation; // & 0x1f;

    switch (op) {
    case ALU_OP_SET_IO_OE_FLAG: {
        return is_higher_half ? 0 : IO_OE_FLAG_SET;
    }

    case ALU_OP_INC_LS: {
        uint8_t q = is_higher_half ? half_A + other_half_carry : half_A + 1;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag =
            global_carry ? CARRY_FLAG_SET : 0; // Preserve global carry.

        uint8_t a_sign = (half_A >> 3);
        uint8_t sum_sign = (half_q >> 3);

        uint8_t alu_overflow_flag = (!a_sign && sum_sign)
                                        ? OVERFLOW_FLAG_SET
                                        : 0x0;

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_DEC_LS: {
        uint8_t q = is_higher_half ? half_A - other_half_carry : half_A - 1;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag =
            global_carry ? CARRY_FLAG_SET : 0; // Preserve global carry.

        uint8_t a_sign = (half_A >> 3);
        uint8_t sum_sign = (half_q >> 3);

        uint8_t alu_overflow_flag = (a_sign && !sum_sign)
                                        ? OVERFLOW_FLAG_SET
                                        : 0x0;

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_ADD_RS: {
        uint8_t q = is_higher_half ? half_A + half_B + other_half_carry
                                   : half_A + half_B;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = half_carry ? CARRY_FLAG_SET : 0;

        uint8_t a_sign = (half_A >> 3);
        uint8_t b_sign = (half_B >> 3);
        uint8_t sum_sign = (half_q >> 3);

        uint8_t alu_overflow_flag =
            ((!a_sign && !b_sign && sum_sign) || (a_sign && b_sign && !sum_sign))
                ? OVERFLOW_FLAG_SET
                : 0x0;

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_NOT_LS: {
        uint8_t q = ~half_A;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = 0; // Clear unconditionally

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_SHL_LS: {
        uint8_t q = is_higher_half ? (uint8_t)(half_A << 1) | other_half_carry
                                   : (uint8_t)(half_A << 1);
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag =
            is_higher_half && half_carry ? CARRY_FLAG_SET : 0;

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_SHR_LS: {
        uint8_t q = is_higher_half
                        ? (half_A >> 1)
                        : (uint8_t)(other_half_carry << 3) | (half_A >> 1);
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (half_A & 1) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag =
            is_higher_half && other_half_carry ? CARRY_FLAG_SET : 0;

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_ROR_LS: {
        uint8_t q = (uint8_t)(half_A >> 1) | (uint8_t)(other_half_carry << 3);
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (half_A & 1) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag =
            is_higher_half && other_half_carry ? CARRY_FLAG_SET : 0;

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_OR_RS: {
        uint8_t half_q = half_A | half_B;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = 0; // Clear unconditionally

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_AND_RS: {
        uint8_t half_q = half_A & half_B;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = 0; // Clear unconditionally

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_XOR_RS: {
        uint8_t half_q = half_A ^ half_B;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = 0; // Clear unconditionally

        uint8_t alu_overflow_flag = 0; // Clear unconditionally

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_ADC_RS: {
        uint8_t q = is_higher_half ? half_A + half_B + other_half_carry
                                   : half_A + half_B + global_carry;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = !half_q ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            half_zero && other_half_zero ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = half_carry ? CARRY_FLAG_SET : 0;

        uint8_t a_sign = (half_A >> 3);
        uint8_t b_sign = (half_B >> 3);
        uint8_t sum_sign = (half_q >> 3);

        uint8_t alu_overflow_flag =
            ((!a_sign && !b_sign && sum_sign) || (a_sign && b_sign && !sum_sign))
                ? OVERFLOW_FLAG_SET
                : 0x0;

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }

    case ALU_OP_LS_SUB_RS: {
        uint8_t q = is_higher_half ? half_A - (half_B + other_half_carry)
                                   : half_A - half_B;
        uint8_t half_q = q & 0xf;
        uint8_t half_zero = half_q == 0 ? HALF_ZERO_FLAG_SET : 0;
        uint8_t half_carry = (q & 0x10) ? HALF_CARRY_FLAG_SET : 0;

        uint8_t alu_zero_flag =
            (half_zero && other_half_zero) ? ZERO_FLAG_SET : 0;

        uint8_t alu_carry_flag = half_carry ? CARRY_FLAG_SET : 0;

        uint8_t a_sign = (half_A >> 3);
        uint8_t b_sign = (half_B >> 3);
        uint8_t sum_sign = (half_q >> 3);

        uint8_t alu_overflow_flag =
            ((!a_sign && b_sign && sum_sign) || (a_sign && !b_sign && !sum_sign))
                ? OVERFLOW_FLAG_SET
                : 0;

        return half_zero | half_carry |
               (is_higher_half ? alu_overflow_flag | alu_carry_flag
                               : alu_zero_flag) |
               ((uint8_t)(half_q << 4));
    }
    }

    return 0;
}

static void generate_lookup_table(uint8_t (*low)[ROM_SIZE],
                                  uint8_t (*high)[ROM_SIZE]) {
    for (uint32_t i = 0; i < ROM_SIZE; ++i) {
        (*low)[i] = output_from_address(i, 0);
        (*high)[i] = output_from_address(i, 1);
    }
}

static int write_to_file(const char *filename,
                         const uint8_t (*table)[ROM_SIZE]) {
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

int main() {
    uint8_t low[ROM_SIZE] = {0};
    uint8_t high[ROM_SIZE] = {0};

    generate_lookup_table(&low, &high);

    int error = write_to_file("bin/alu_low.bin", &low);

    return error == 0 ? write_to_file("bin/alu_high.bin", &high) : error;
}
