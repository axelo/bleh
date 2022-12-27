#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define CONTROL_ROM_SIZE (1 << 17)
#define ALU_ROM_SIZE (1 << 17)
#define ROM_SIZE (1 << 15)
#define RAM_SIZE (1 << 15)

typedef struct {
    // Control low signals
    uint8_t c0_or_ce_m;
    uint8_t c1_or_ld_o;
    uint8_t c2_or_ld_s;
    uint8_t c3_or_ld_rs;
    uint8_t c4_alu_op4_or_ld_io;
    uint8_t c5_ls_alu_q_or_halt_c;
    uint8_t ld_c; // active low
    uint8_t toggle_m_c; // m when low

    // Control high signals
    uint8_t ld_mem; // active high
    uint8_t ld_ls; // active low
    uint8_t ld_ml; // active low
    uint8_t ld_mh; // active low
    uint8_t oe_ml; // active low
    uint8_t oe_mh; // active low
    uint8_t oe_alu; // active low
    uint8_t oe_mem; // active low

    // Signals based on clock exec
    uint8_t c_ld_mem; // active low

    // Signals based on ld_c high.
    uint8_t ld_o; // active low
    uint8_t ld_s; // active low
    uint8_t ld_rs; // active low
    uint8_t ld_io; // active low
    uint8_t halt; // active low
} Signals;

typedef struct {
    uint8_t alu_l_qz; // 1 bit
    uint8_t alu_l_qc; // 1 bit

    uint8_t alu_h_qz; // 1 bit
    uint8_t alu_h_qc; // 1 bit

    uint8_t alu_q_zf; // 1 bit
    uint8_t alu_q_io_oe; // 1 bit
    uint8_t alu_q_cf; // 1 bit
    uint8_t alu_q_of; // 1 bit

    uint8_t alu_q;
} ALU;

typedef struct {
    uint8_t c_exec; // 1 bit

    uint8_t r_s; // 4 bit
    uint8_t r_o;
    uint8_t r_f; // 4 bit
    uint8_t r_ls;
    uint8_t r_rs;
    uint8_t r_c;
    uint8_t r_ml;
    uint8_t r_mh;
    uint8_t r_sel_m_or_c; // 1 bit, m when low

    uint16_t address_bus;
    uint8_t en_rom;
    uint8_t en_ram;

    uint8_t data_bus;

    Signals signals;

    ALU alu;
} State;

static uint8_t control_rom[CONTROL_ROM_SIZE];
static uint8_t alu_low_rom[ALU_ROM_SIZE];
static uint8_t alu_high_rom[ALU_ROM_SIZE];

static uint8_t rom[RAM_SIZE];
static uint8_t ram[RAM_SIZE];

static void print_state(State *s) {
    Signals *sig = &s->signals;

    printf(
        "\n============= STATE =============\n"
        "            C EXEC: %d\n"
        "\n"
        "           C0/CE M: %d\n"
        "           C1/LD O: %d\n"
        "           C2/LD S: %d\n"
        "          C3/LD RS: %d\n"
        "  C4 ALU OP4/LD IO: %d\n"
        "C5 LS ALU Q/HALT C: %d\n"
        "             ~LD C: %d\n"
        "       TOGGLE ~M/C: %d\n"
        "\n"
        "            LD MEM: %d\n"
        "            ~LD LS: %d\n"
        "            ~LD ML: %d\n"
        "            ~LD MH: %d\n"
        "            ~OE ML: %d\n"
        "            ~OE MH: %d\n"
        "           ~OE ALU: %d\n"
        "           ~OE MEM: %d\n"
        "\n"
        "         C ~LD MEM: %d\n"
        "             ~LD O: %d\n"
        "             ~LD S: %d\n"
        "            ~LD RS: %d\n"
        "            ~LD IO: %d\n"
        "             ~HALT: %d\n"
        "\n"
        "               BUS: 0x%02x\n"
        "       ADDRESS BUS: 0x%04x\n"
        "           ~EN ROM: %d\n"
        "           ~EN RAM: %d\n"
        "\n"
        "                 S: 0x%x\n"
        "                 O: 0x%02x\n"
        "                 F: 0x%x\n"
        "          SEL ~M/C: %d\n"
        "                 C: 0x%02x\n"
        "                LS: 0x%02x\n"
        "                RS: 0x%02x\n"
        "                ML: 0x%02x\n"
        "                MH: 0x%02x\n"
        "\n"
        "          ALU Q ZF: %d\n"
        "       ALU Q IO OE: %d\n"
        "          ALU Q CF: %d\n"
        "          ALU Q OF: %d\n"
        "             ALU Q: 0x%02x\n",
        s->c_exec,

        sig->c0_or_ce_m,
        sig->c1_or_ld_o,
        sig->c2_or_ld_s,
        sig->c3_or_ld_rs,
        sig->c4_alu_op4_or_ld_io,
        sig->c5_ls_alu_q_or_halt_c,
        sig->ld_c,
        sig->toggle_m_c,

        sig->ld_mem,
        sig->ld_ls,
        sig->ld_ml,
        sig->ld_mh,
        sig->oe_ml,
        sig->oe_mh,
        sig->oe_alu,
        sig->oe_mem,

        sig->c_ld_mem,
        sig->ld_o,
        sig->ld_s,
        sig->ld_rs,
        sig->ld_io,
        sig->halt,

        s->data_bus,
        s->address_bus,
        s->en_rom,
        s->en_ram,

        s->r_s,
        s->r_o,
        s->r_f,
        s->r_sel_m_or_c,
        s->r_c,
        s->r_ls,
        s->r_rs,
        s->r_ml,
        s->r_mh,

        s->alu.alu_q_zf,
        s->alu.alu_q_io_oe,
        s->alu.alu_q_cf,
        s->alu.alu_q_of,
        s->alu.alu_q);
}

static ALU next_alu(const State *s) {
    uint8_t cf = (s->r_f >> 1) & 1;

    uint8_t c_q0 = (s->r_c >> 0) & 1;
    uint8_t c_q1 = (s->r_c >> 1) & 1;
    uint8_t c_q2 = (s->r_c >> 2) & 1;
    uint8_t c_q3 = (s->r_c >> 3) & 1;
    uint8_t c_q4 = (s->r_c >> 4) & 1;
    uint8_t c_q5 = (s->r_c >> 5) & 1;

    uint8_t alu_l_qz = s->alu.alu_l_qz;
    uint8_t alu_l_qc = s->alu.alu_l_qc;

    uint8_t alu_h_qz = s->alu.alu_h_qz;
    uint8_t alu_h_qc = s->alu.alu_h_qc;

    for (int i = 0; i < 10; ++i) {
        uint32_t alu_l_address = (c_q5 << 16) | (alu_h_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_h_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (cf << 8) | ((s->r_rs & 0xf) << 4) | (s->r_ls & 0xf);
        uint32_t alu_h_address = (c_q5 << 16) | (alu_l_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_l_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (cf << 8) | ((s->r_rs >> 4) << 4) | (s->r_ls >> 4);

        uint8_t alu_l_signals = alu_low_rom[alu_l_address];
        uint8_t alu_h_signals = alu_high_rom[alu_h_address];

        uint8_t alu_l_q0_alu_l_qz = (alu_l_signals >> 0) & 1;
        uint8_t alu_l_q1_alu_l_qc = (alu_l_signals >> 1) & 1;

        uint8_t alu_h_q0_alu_h_qz = (alu_h_signals >> 0) & 1;
        uint8_t alu_h_q1_alu_h_qc = (alu_h_signals >> 1) & 1;

        if (alu_l_qz == alu_l_q0_alu_l_qz && alu_l_qc == alu_l_q1_alu_l_qc &&
            alu_h_qz == alu_h_q0_alu_h_qz && alu_h_qc == alu_h_q1_alu_h_qc) {

            return (ALU){
                .alu_l_qz = alu_l_qz,
                .alu_l_qc = alu_l_qc,

                .alu_h_qz = alu_h_qz,
                .alu_h_qc = alu_h_qc,

                .alu_q_zf = (alu_l_signals >> 2) & 1,
                .alu_q_io_oe = (alu_l_signals >> 3) & 1,
                .alu_q_cf = (alu_h_signals >> 2) & 1,
                .alu_q_of = (alu_h_signals >> 3) & 1,

                .alu_q = (alu_h_signals & 0xf0) | (alu_l_signals >> 4),
            };
        }

        alu_l_qz = alu_l_q0_alu_l_qz;
        alu_l_qc = alu_l_q1_alu_l_qc;

        alu_h_qz = alu_h_q0_alu_h_qz;
        alu_h_qc = alu_h_q1_alu_h_qc;
    }

    assert(0 && "ALU signals never settled");
}

static State next_state(State s) {
    s.c_exec = (!s.c_exec) & 1;

    if (!s.c_exec) { // C SETUP (~C EXEC)
        // Count S
        if (++s.r_s >= 0x10) {
            s.r_s = 0;
        }

        // Latch S
        if (!s.signals.ld_s) {
            s.r_s = 0x0;
        }

        // Latch C
        if (!s.signals.ld_c) {
            s.r_c = (1 << 7) |
                    (s.alu.alu_q_io_oe << 6) |
                    (s.signals.c5_ls_alu_q_or_halt_c << 5) |
                    (s.signals.c4_alu_op4_or_ld_io << 4) |
                    (s.signals.c3_or_ld_rs << 3) |
                    (s.signals.c2_or_ld_s << 2) |
                    (s.signals.c1_or_ld_o << 1) |
                    (s.signals.c0_or_ce_m << 0);

            s.alu = next_alu(&s);
        }

        // Count ML/MH
        if (s.signals.ld_c && s.signals.c0_or_ce_m && s.signals.ld_ml) {
            if (++s.r_ml == 0 && s.signals.ld_mh) {
                ++s.r_mh;
            }
        }

        // Latch ML
        if (!s.signals.ld_ml) {
            s.r_ml = s.data_bus;
        }

        // Latch MH
        if (!s.signals.ld_mh) {
            s.r_mh = s.data_bus;
        }

        // Toggle SEL ~M/C
        if (s.signals.toggle_m_c) {
            s.r_sel_m_or_c = (!s.r_sel_m_or_c) & 1;
        }

        uint8_t zf = (s.r_f >> 0) & 1;
        uint8_t cf = (s.r_f >> 1) & 1;
        uint8_t of = (s.r_f >> 2) & 1;
        uint8_t sf = (s.r_f >> 3) & 1;

        uint8_t step_q0 = (s.r_s >> 0) & 1;
        uint8_t step_q1 = (s.r_s >> 1) & 1;
        uint8_t step_q2 = (s.r_s >> 2) & 1;
        uint8_t step_q3 = (s.r_s >> 3) & 1;

        uint32_t control_l_address = (step_q2 << 16) | (step_q1 << 15) | (step_q3 << 14) | (sf << 13) | (step_q0 << 12) | (zf << 11) | (0 << 10) | (cf << 9) | (of << 8) | s.r_o;
        uint32_t control_h_address = (step_q2 << 16) | (step_q1 << 15) | (step_q3 << 14) | (sf << 13) | (step_q0 << 12) | (zf << 11) | (1 << 10) | (cf << 9) | (of << 8) | s.r_o;

        uint8_t control_l_signals = control_rom[control_l_address];
        uint8_t control_h_signals = control_rom[control_h_address];

        s.signals.c0_or_ce_m = (control_l_signals >> 0) & 1;
        s.signals.c1_or_ld_o = (control_l_signals >> 1) & 1;
        s.signals.c2_or_ld_s = (control_l_signals >> 2) & 1;
        s.signals.c3_or_ld_rs = (control_l_signals >> 3) & 1;
        s.signals.c4_alu_op4_or_ld_io = (control_l_signals >> 4) & 1;
        s.signals.c5_ls_alu_q_or_halt_c = (control_l_signals >> 5) & 1;
        s.signals.ld_c = (control_l_signals >> 6) & 1;
        s.signals.toggle_m_c = (control_l_signals >> 7) & 1;

        s.signals.ld_mem = (control_h_signals >> 0) & 1;
        s.signals.ld_ls = (control_h_signals >> 1) & 1;
        s.signals.ld_ml = (control_h_signals >> 2) & 1;
        s.signals.ld_mh = (control_h_signals >> 3) & 1;
        s.signals.oe_ml = (control_h_signals >> 4) & 1;
        s.signals.oe_mh = (control_h_signals >> 5) & 1;
        s.signals.oe_alu = (control_h_signals >> 6) & 1;
        s.signals.oe_mem = (control_h_signals >> 7) & 1;

        s.signals.c_ld_mem = ~(s.signals.ld_mem & s.c_exec) & 1;
        s.signals.ld_o = ~(s.signals.c1_or_ld_o & s.signals.ld_c) & 1;
        s.signals.ld_s = ~(s.signals.c2_or_ld_s & s.signals.ld_c) & 1;
        s.signals.ld_rs = ~(s.signals.c3_or_ld_rs & s.signals.ld_c) & 1;
        s.signals.ld_io = ~(s.signals.c4_alu_op4_or_ld_io & s.signals.ld_c) & 1;
        s.signals.halt = ~(s.signals.c5_ls_alu_q_or_halt_c & s.signals.ld_c) & 1;

        s.address_bus = s.r_sel_m_or_c ? (0xfff0 | (s.r_c & 0xf)) : (s.r_mh << 8) | s.r_ml;
        s.en_rom = s.address_bus >> 15;
        s.en_ram = ~s.en_rom & 1;

        s.data_bus = // TODO: oe_io
            (!s.signals.oe_ml)    ? s.r_ml
            : (!s.signals.oe_mh)  ? s.r_mh
            : (!s.signals.oe_alu) ? s.alu.alu_q
            : (!s.signals.oe_mem) ? ((!s.en_rom)   ? rom[s.address_bus]
                                     : (!s.en_ram) ? ram[s.address_bus]
                                                   : 0xff)
                                  : 0xff;
    } else { // C EXEC
        s.signals.c_ld_mem = ~(s.signals.ld_mem & s.c_exec) & 1;

        // Latch O
        if (!s.signals.ld_o) {
            s.r_o = s.data_bus;
        }

        // Latch RS
        if (!s.signals.ld_rs) {
            s.r_rs = s.data_bus;

            s.alu = next_alu(&s);
        }

        // Latch LS
        if (!s.signals.ld_ls) {
            s.r_ls = s.data_bus;

            s.alu = next_alu(&s);
        }

        // Latch RAM (ROM is read only :))
        if (!s.signals.c_ld_mem && !s.en_ram) {
            ram[s.address_bus & 0x7fff] = s.data_bus;
        }
    }

    // Latch IO
    if (!s.signals.ld_io) {
        // TODO: Use r_o to know which port.
    }

    // Latch F
    if (!s.signals.oe_alu && !s.signals.ld_ls) {
        s.r_f = ((s.alu.alu_q >> 4) << 3) |
                (s.alu.alu_q_of << 2) |
                (s.alu.alu_q_cf << 1) |
                (s.alu.alu_q_zf << 0);

        s.alu = next_alu(&s);
    }

    return s;
}

int main(void) {
    FILE *file = fopen("./bin/control.bin", "r");
    assert(file != NULL && "Failed to read control.bin");
    int read_bytes = fread(control_rom, sizeof(uint8_t), CONTROL_ROM_SIZE, file);
    assert(read_bytes == CONTROL_ROM_SIZE && "Failed to read the entire contents of control.bin");
    assert(fclose(file) == 0 && "Failed to close file");

    file = fopen("./bin/alu_low.bin", "r");
    assert(file != NULL && "Failed to read alu_low.bin");
    read_bytes = fread(alu_low_rom, sizeof(uint8_t), ALU_ROM_SIZE, file);
    assert(read_bytes == ALU_ROM_SIZE && "Failed to read the entire contents of alu_low.bin");
    assert(fclose(file) == 0 && "Failed to close file");

    file = fopen("./bin/alu_high.bin", "r");
    assert(file != NULL && "Failed to read alu_high.bin");
    read_bytes = fread(alu_high_rom, sizeof(uint8_t), ALU_ROM_SIZE, file);
    assert(read_bytes == ALU_ROM_SIZE && "Failed to read the entire contents of alu_high.bin");
    assert(fclose(file) == 0 && "Failed to close file");

    rom[0] = 0x01;
    rom[1] = 0xab;

    // Reset
    State state = {0};
    state.c_exec = 1;

    //
    state = next_state(state);
    // print_state(&state);

    state = next_state(state);
    print_state(&state);

    //
    state = next_state(state);
    // print_state(&state);

    state = next_state(state);
    print_state(&state);

    //
    state = next_state(state);
    state = next_state(state);

    print_state(&state);

    printf("%02x\n", ram[0x7ff0]);

    return 0;
}
