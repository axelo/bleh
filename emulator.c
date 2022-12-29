#include <assert.h>
#include <stdint.h>
#include <stdio.h> // f*
#include <stdlib.h> // exit
#include <time.h> // nanosleep

#define CLOCK_DELAY_MS (60) // TODO: Change to HZ
#define EXIT_AFTER_N_INSTRUCTIONS (100) // TODO: Probably an in parameter

#define CONTROL_ROM_SIZE (1 << 17)
#define ALU_ROM_SIZE (1 << 17)
#define ROM_SIZE (1 << 15)
#define RAM_SIZE (1 << 15)

#define RAM_ABSOLUTE_START_ADDRESS (0x8000)
#define PROGRAM_RAM_RELATIVE_START_ADDRESS (0x1000) // TODO: Probably an in parameter or parse `customasm_ram.asm`

// Control low signals
#define SIGNAL_C0_OR_CE_M(signals) (((signals) >> 0) & 1)
#define SIGNAL_C1_OR_LD_O(signals) (((signals) >> 1) & 1)
#define SIGNAL_C2_OR_LD_S(signals) (((signals) >> 2) & 1)
#define SIGNAL_C3_OR_LD_RS(signals) (((signals) >> 3) & 1)
#define SIGNAL_C4_ALU_OP4_OR_LD_IO(signals) (((signals) >> 4) & 1)
#define SIGNAL_C5_LS_ALU_Q_OR_HALT_C(signals) (((signals) >> 5) & 1)
#define SIGNAL_LD_C(signals) (((signals) >> 6) & 1)
#define SIGNAL_TOGGLE_M_C(signals) (((signals) >> 7) & 1)

// Control high signals
#define SIGNAL_LD_MEM(signals) (((signals) >> 8) & 1)
#define SIGNAL_LD_LS(signals) (((signals) >> 9) & 1)
#define SIGNAL_LD_ML(signals) (((signals) >> 10) & 1)
#define SIGNAL_LD_MH(signals) (((signals) >> 11) & 1)
#define SIGNAL_OE_ML(signals) (((signals) >> 12) & 1)
#define SIGNAL_OE_MH(signals) (((signals) >> 13) & 1)
#define SIGNAL_OE_ALU(signals) (((signals) >> 14) & 1)
#define SIGNAL_OE_MEM(signals) (((signals) >> 15) & 1)

// Combined NAND signals
#define SIGNAL_LD_O(signals) (~(SIGNAL_C1_OR_LD_O(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_LD_S(signals) (~(SIGNAL_C2_OR_LD_S(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_LD_RS(signals) (~(SIGNAL_C3_OR_LD_RS(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_LD_IO(signals) (~(SIGNAL_C4_ALU_OP4_OR_LD_IO(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_HALT(signals) (~(SIGNAL_C5_LS_ALU_Q_OR_HALT_C(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_LD_RS(signals) (~(SIGNAL_C3_OR_LD_RS(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_LD_IO(signals) (~(SIGNAL_C4_ALU_OP4_OR_LD_IO(signals) & SIGNAL_LD_C(signals)) & 1)
#define SIGNAL_HALT(signals) (~(SIGNAL_C5_LS_ALU_Q_OR_HALT_C(signals) & SIGNAL_LD_C(signals)) & 1)

// Combined NAND and C signals
#define SIGNAL_C_LD_MEM(signals, c_exec) (~(SIGNAL_LD_MEM(signals) & c_exec) & 1)

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

#define C_OE_IO(c_q) (((c_q) >> 6) & 1) // Active high

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

    uint16_t control_signals;
    uint16_t alu_signals;

    uint16_t address_bus;
    uint8_t en_rom; // TODO: macro
    uint8_t en_ram; // TODO: macro

    uint8_t data_bus;
} State;

static uint8_t control_rom[CONTROL_ROM_SIZE];
static uint8_t alu_low_rom[ALU_ROM_SIZE];
static uint8_t alu_high_rom[ALU_ROM_SIZE];

static uint8_t rom[RAM_SIZE];
static uint8_t ram[RAM_SIZE];
static uint8_t io_ports[8];

static int n_instructions = 0;

static void print_state(State s) {
    // TODO: Write to a buffer then do one write to stdout.

    printf("\033[2J\033[3J"); // Clear the viewport and the screen, the order seems to be important
    printf("\033[H"); // Position cursor at top-left corner

    printf("CLK   S   O   F   LS   RS   C   ML   MH (ic: %d)\n", n_instructions);
    printf("  %d%4d%4x%4x%5x%5x%4x%5x%5x\n\n", s.c_exec, s.r_s, s.r_o, s.r_f, s.r_ls, s.r_rs, s.r_c, s.r_ml, s.r_mh);

    printf("ZF   CF   OF   SF   SEL ~M/C   ~HALT\n");
    printf("%2d%5d%5d%5d%11d%8d\n\n",
           (s.r_f >> 0 & 1),
           (s.r_f >> 1 & 1),
           (s.r_f >> 2 & 1),
           (s.r_f >> 3 & 1),
           s.r_sel_m_or_c,
           SIGNAL_HALT(s.control_signals));

    printf("ALU Q ZF   ALU Q CF   ALU Q OF   ALU Q IO OE   ALU Q\n");
    printf("%8d%11d%11d%11d%11x\n\n",
           ALU_SIGNAL_Q_ZF(s.alu_signals),
           ALU_SIGNAL_Q_CF(s.alu_signals),
           ALU_SIGNAL_Q_OF(s.alu_signals),
           ALU_SIGNAL_Q_IO_OE(s.alu_signals),
           ALU_SIGNAL_Q(s.alu_signals));

    printf("C0/CE M   C1/LD O   C2/LD S   C3/LD RS\n");
    printf("%7d%10d%10d%11d\n\n",
           SIGNAL_C0_OR_CE_M(s.control_signals),
           SIGNAL_C1_OR_LD_O(s.control_signals),
           SIGNAL_C2_OR_LD_S(s.control_signals),
           SIGNAL_C3_OR_LD_RS(s.control_signals));

    printf("C4 ALU OP4/LD IO   C5 LS ALU Q/HALT C   C3..0   C5..0\n");
    printf("%16d%21d%8x%8x\n\n",
           SIGNAL_C4_ALU_OP4_OR_LD_IO(s.control_signals),
           SIGNAL_C5_LS_ALU_Q_OR_HALT_C(s.control_signals),
           s.control_signals & 0xf,
           s.control_signals & 0x3f);

    printf("~LD C   TOGGLE ~M/C   LD MEM   ~LD LS   ~LD ML   ~LD MH\n");
    printf("%5d%14d%9d%9d%9d%9d\n\n",
           SIGNAL_LD_C(s.control_signals),
           SIGNAL_TOGGLE_M_C(s.control_signals),
           SIGNAL_LD_MEM(s.control_signals),
           SIGNAL_LD_LS(s.control_signals),
           SIGNAL_LD_ML(s.control_signals),
           SIGNAL_LD_MH(s.control_signals));

    printf("C ~LD MEM   ~LD O   ~LD S   ~LD RS   ~LD IO\n");
    printf("%9d%8d%8d%9d%9d\n\n",
           SIGNAL_C_LD_MEM(s.control_signals, s.c_exec),
           SIGNAL_LD_O(s.control_signals),
           SIGNAL_LD_S(s.control_signals),
           SIGNAL_LD_RS(s.control_signals),
           SIGNAL_LD_IO(s.control_signals));

    printf("~OE ML   ~OE MH   ~OE ALU   ~OE MEM   OE IO\n");
    printf("%6d%9d%10d%10d%8d\n\n",
           SIGNAL_OE_ML(s.control_signals),
           SIGNAL_OE_MH(s.control_signals),
           SIGNAL_OE_ALU(s.control_signals),
           SIGNAL_OE_MEM(s.control_signals),
           C_OE_IO(s.r_c));

    printf("IO PORT 0   IO PORT 1   IO PORT 2   IO PORT 3\n");
    printf("%9x%12x%12x%12x\n\n",
           io_ports[0],
           io_ports[1],
           io_ports[2],
           io_ports[3]);

    printf("IO PORT 4   IO PORT 5   IO PORT 6   IO PORT 7\n");
    printf("%9x%12x%12x%12x\n\n",
           io_ports[4],
           io_ports[5],
           io_ports[6],
           io_ports[7]);

    fflush(stdout);
}

static uint16_t control_signals(State s) {
    uint8_t zf = (s.r_f >> 0) & 1; // TODO: macro
    uint8_t cf = (s.r_f >> 1) & 1; // TODO: macro
    uint8_t of = (s.r_f >> 2) & 1; // TODO: macro
    uint8_t sf = (s.r_f >> 3) & 1; // TODO: macro

    uint8_t step_q0 = (s.r_s >> 0) & 1;
    uint8_t step_q1 = (s.r_s >> 1) & 1;
    uint8_t step_q2 = (s.r_s >> 2) & 1;
    uint8_t step_q3 = (s.r_s >> 3) & 1;

    uint32_t control_l_address = (step_q2 << 16) | (step_q1 << 15) | (step_q3 << 14) | (sf << 13) | (step_q0 << 12) | (zf << 11) | (0 << 10) | (cf << 9) | (of << 8) | s.r_o;
    uint32_t control_h_address = (step_q2 << 16) | (step_q1 << 15) | (step_q3 << 14) | (sf << 13) | (step_q0 << 12) | (zf << 11) | (1 << 10) | (cf << 9) | (of << 8) | s.r_o;

    uint8_t control_l_signals = control_rom[control_l_address];
    uint8_t control_h_signals = control_rom[control_h_address];

    return (control_h_signals << 8) | control_l_signals;
}

static uint16_t alu_signals(State s) {
    uint8_t cf = (s.r_f >> 1) & 1; // TODO: macro

    uint8_t c_q0 = (s.r_c >> 0) & 1;
    uint8_t c_q1 = (s.r_c >> 1) & 1;
    uint8_t c_q2 = (s.r_c >> 2) & 1;
    uint8_t c_q3 = (s.r_c >> 3) & 1;
    uint8_t c_q4 = (s.r_c >> 4) & 1;
    uint8_t c_q5 = (s.r_c >> 5) & 1;

    uint8_t alu_l_qz = ALU_SIGNAL_L_QZ(s.alu_signals);
    uint8_t alu_l_qc = ALU_SIGNAL_L_QC(s.alu_signals);

    uint8_t alu_h_qz = ALU_SIGNAL_H_QZ(s.alu_signals);
    uint8_t alu_h_qc = ALU_SIGNAL_H_QC(s.alu_signals);

    for (int i = 0; i < 5; ++i) {
        uint32_t alu_l_address = (c_q5 << 16) | (alu_h_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_h_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (cf << 8) | ((s.r_rs & 0xf) << 4) | (s.r_ls & 0xf);
        uint32_t alu_h_address = (c_q5 << 16) | (alu_l_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_l_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (cf << 8) | ((s.r_rs >> 4) << 4) | (s.r_ls >> 4);

        uint16_t alu_signals = (alu_high_rom[alu_h_address] << 8) | alu_low_rom[alu_l_address];

        uint8_t alu_l_q0_alu_l_qz = ALU_SIGNAL_L_QZ(alu_signals);
        uint8_t alu_l_q1_alu_l_qc = ALU_SIGNAL_L_QC(alu_signals);

        uint8_t alu_h_q0_alu_h_qz = ALU_SIGNAL_H_QZ(alu_signals);
        uint8_t alu_h_q1_alu_h_qc = ALU_SIGNAL_H_QC(alu_signals);

        if (alu_l_qz == alu_l_q0_alu_l_qz && alu_l_qc == alu_l_q1_alu_l_qc &&
            alu_h_qz == alu_h_q0_alu_h_qz && alu_h_qc == alu_h_q1_alu_h_qc) {

            if (i > 2) {
                // TODO: Temp.
                printf("\n I: %d\n", i);
                exit(0);
            }

            return alu_signals;
        }

        alu_l_qz = alu_l_q0_alu_l_qz;
        alu_l_qc = alu_l_q1_alu_l_qc;

        alu_h_qz = alu_h_q0_alu_h_qz;
        alu_h_qc = alu_h_q1_alu_h_qc;
    }

    assert(0 && "ALU signals never settled");
}

static State next_state(State s) {
    if (!SIGNAL_HALT(s.control_signals)) {
        return s;
    }

    s.c_exec = (!s.c_exec) & 1;

    if (!s.c_exec) { // C SETUP (~C EXEC)
        // Count S
        if (++s.r_s >= 0x10) {
            s.r_s = 0;
        }

        // Latch S
        if (!SIGNAL_LD_S(s.control_signals)) {
            s.r_s = 0x0;
        }

        // Latch C
        if (!SIGNAL_LD_C(s.control_signals)) {
            s.r_c = (1 << 7) |
                    (ALU_SIGNAL_Q_IO_OE(s.alu_signals) << 6) |
                    (SIGNAL_C5_LS_ALU_Q_OR_HALT_C(s.control_signals) << 5) |
                    (SIGNAL_C4_ALU_OP4_OR_LD_IO(s.control_signals) << 4) |
                    (SIGNAL_C3_OR_LD_RS(s.control_signals) << 3) |
                    (SIGNAL_C2_OR_LD_S(s.control_signals) << 2) |
                    (SIGNAL_C1_OR_LD_O(s.control_signals) << 1) |
                    (SIGNAL_C0_OR_CE_M(s.control_signals) << 0);

            s.alu_signals = alu_signals(s);

            if (C_OE_IO(s.r_c)) {
                // TODO: Use r_o to know which port.
                // TODO: Update emulated IO devices
                assert(0 && "OE IO not yet implemented");
            }
        }

        // Count ML/MH
        if (SIGNAL_LD_C(s.control_signals) && SIGNAL_C0_OR_CE_M(s.control_signals) && SIGNAL_LD_ML(s.control_signals)) {
            if (++s.r_ml == 0 && SIGNAL_LD_MH(s.control_signals)) {
                ++s.r_mh;
            }
        }

        // Latch ML
        if (!SIGNAL_LD_ML(s.control_signals)) {
            s.r_ml = s.data_bus;
        }

        // Latch MH
        if (!SIGNAL_LD_MH(s.control_signals)) {
            s.r_mh = s.data_bus;
        }

        // Toggle SEL ~M/C
        if (SIGNAL_TOGGLE_M_C(s.control_signals)) {
            s.r_sel_m_or_c = (!s.r_sel_m_or_c) & 1;
        }

        s.control_signals = control_signals(s);

        s.address_bus = s.r_sel_m_or_c ? (0xfff0 | (s.r_c & 0xf)) : (s.r_mh << 8) | s.r_ml;
        s.en_rom = s.address_bus >> 15;
        s.en_ram = ~s.en_rom & 1;

        int n_oe = 0;

        // Assert ML to data bus
        if (!SIGNAL_OE_ML(s.control_signals)) {
            s.data_bus = s.r_ml;
            ++n_oe;
        }

        // Assert MH to data bus
        if (!SIGNAL_OE_MH(s.control_signals)) {
            s.data_bus = s.r_mh;
            ++n_oe;
        }

        // Assert ALU to data bus
        if (!SIGNAL_OE_ALU(s.control_signals)) {
            s.data_bus = ALU_SIGNAL_Q(s.alu_signals);
            ++n_oe;
        }

        // Assert MEM to data bus
        if (!SIGNAL_OE_MEM(s.control_signals)) {
            if (!s.en_rom) {
                s.data_bus = rom[s.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)];
                ++n_oe;
            }

            if (!s.en_ram) {
                s.data_bus = ram[s.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)];
                ++n_oe;
            }
        }

        if (C_OE_IO(s.r_c)) {
            // C is latched during C ~EXEC
            ++n_oe;
        }

        if (n_oe == 0) {
            s.data_bus = 0xff; // Data bus is pulled up
        }

        assert(n_oe <= 1 && "More then one is asserting to the data bus");
    } else { // C EXEC
        // Latch O
        if (!SIGNAL_LD_O(s.control_signals)) {
            s.r_o = s.data_bus;
        }

        // Latch RS
        if (!SIGNAL_LD_RS(s.control_signals)) {
            s.r_rs = s.data_bus;

            s.alu_signals = alu_signals(s);
        }

        // Latch LS
        if (!SIGNAL_LD_LS(s.control_signals)) {
            s.r_ls = s.data_bus;

            s.alu_signals = alu_signals(s);
        }

        // Latch RAM (ROM is read only :))
        if (!SIGNAL_C_LD_MEM(s.control_signals, s.c_exec) && !s.en_ram) {
            ram[s.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)] = s.data_bus;
        }
    }

    // Latch F
    if (!SIGNAL_OE_ALU(s.control_signals) && !SIGNAL_LD_LS(s.control_signals)) {
        s.r_f = (ALU_SIGNAL_Q(s.alu_signals) << 3) | // SF
                (ALU_SIGNAL_Q_OF(s.alu_signals) << 2) |
                (ALU_SIGNAL_Q_CF(s.alu_signals) << 1) |
                (ALU_SIGNAL_Q_ZF(s.alu_signals) << 0);

        s.alu_signals = alu_signals(s);
    }

    // Latch IO
    if (!SIGNAL_LD_IO(s.control_signals)) {
        io_ports[s.r_o & 7] = s.data_bus;

        // TODO: Update emulated IO devices
    }

    return s;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Missing program\n");
        exit(1);
    }

    FILE *file = fopen(argv[1], "r");
    assert(file != NULL && "Failed to read program");

    fseek(file, 0, SEEK_END);
    size_t program_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    assert(program_size <= (RAM_SIZE - PROGRAM_RAM_RELATIVE_START_ADDRESS) && "Program too big");

    size_t read_bytes = fread(ram + PROGRAM_RAM_RELATIVE_START_ADDRESS, sizeof(uint8_t), program_size, file);
    assert(read_bytes == program_size && "Failed to read entire contents of program");
    assert(fclose(file) == 0 && "Failed to close file");

    file = fopen("./bin/control.bin", "r");
    assert(file != NULL && "Failed to read control.bin");
    read_bytes = fread(control_rom, sizeof(uint8_t), CONTROL_ROM_SIZE, file);
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

    rom[0] = 0x92; // jmp imm16
    rom[1] = (RAM_ABSOLUTE_START_ADDRESS + PROGRAM_RAM_RELATIVE_START_ADDRESS) & 0xff;
    rom[2] = (RAM_ABSOLUTE_START_ADDRESS + PROGRAM_RAM_RELATIVE_START_ADDRESS) >> 8;

    // Reset by running an initial setup phase where S is 0 afterwards.
    State state = next_state((State){.c_exec = 1,
                                     .r_s = 0xf});

    struct timespec ts = {
        .tv_sec = 0,
        .tv_nsec = (CLOCK_DELAY_MS)*1e6,
    };

    while (1) {
        // Execute
        state = next_state(state);
        print_state(state);

        nanosleep(&ts, NULL);

        if (!SIGNAL_LD_S(state.control_signals)) {
            ++n_instructions;

            if (n_instructions >= EXIT_AFTER_N_INSTRUCTIONS) {
                break;
            }
        }

        // Setup
        state = next_state(state);
        print_state(state);

        nanosleep(&ts, NULL);
    }

    return 0;
}
