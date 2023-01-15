#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // f*
#include <stdlib.h> // exit
#include <time.h> // nanosleep

#include "opcode.h"

#define EXIT_AFTER_N_INSTRUCTIONS (50000) // TODO: Probably an in parameter

#define CONTROL_ROM_SIZE (1 << 17)
#define ALU_ROM_SIZE (1 << 17)
#define ROM_SIZE (1 << 15)
#define RAM_SIZE (1 << 15)

#define RAM_ABSOLUTE_START_ADDRESS (0x8000)
#define PROGRAM_RAM_RELATIVE_START_ADDRESS (0x0000) // TODO: Probably an in parameter

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
#define ALU_SIGNAL_Q_SF(alu_signals) ((ALU_SIGNAL_Q(alu_signals) >> 7) & 1)

// C signals
#define C_OE_IO(c_q) (((c_q) >> 6) & 1) // Active high

// Based on address bus signals
#define SIGNAL_EN_ROM(address_bus) (((address_bus) >> 15) & 1)
#define SIGNAL_EN_RAM(address_bus) (~SIGNAL_EN_ROM(address_bus) & 1)

#define F_ZF(r_f) (((r_f) >> 0) & 1)
#define F_CF(r_f) (((r_f) >> 1) & 1)
#define F_OF(r_f) (((r_f) >> 2) & 1)
#define F_SF(r_f) (((r_f) >> 3) & 1)

#define S_Q0(r_s) (((r_s) >> 0) & 1)
#define S_Q1(r_s) (((r_s) >> 1) & 1)
#define S_Q2(r_s) (((r_s) >> 2) & 1)
#define S_Q3(r_s) (((r_s) >> 3) & 1)

#define IO_LD_DEBUG_PORT (1) // TODO: Probably an in parameter
#define IO_LD_LCD_PORT (2) // TODO: Probably an in parameter
#define IO_OE_LCD_PORT (2) // TODO: Probably an in parameter

#define LCD_SIGNAL_RS(data_bus) (((data_bus) >> 7) & 1)
#define LCD_SIGNAL_RW(data_bus) (((data_bus) >> 6) & 1)
#define LCD_SIGNAL_E(data_bus) (((data_bus) >> 5) & 1)
#define LCD_SIGNAL_DATA(data_bus) ((data_bus)&0xf)

typedef struct {
    bool c_exec; // 1 bit

    uint8_t r_s; // 4 bit
    uint8_t r_o;
    uint8_t r_f; // 4 bit
    uint8_t r_ls;
    uint8_t r_rs;
    uint8_t r_c;
    uint8_t r_ml;
    uint8_t r_mh;
    bool r_sel_m_or_c; // 1 bit, m when low

    uint16_t control_signals;
    uint16_t alu_signals;

    uint16_t address_bus;
    uint8_t data_bus;
} CPU;

static uint8_t control_rom[CONTROL_ROM_SIZE];
static uint8_t alu_low_rom[ALU_ROM_SIZE];
static uint8_t alu_high_rom[ALU_ROM_SIZE];

static uint8_t rom[RAM_SIZE];
static uint8_t ram[RAM_SIZE];
static uint8_t io_ports[8];

typedef struct {
    uint8_t ir;
    uint8_t dr;

    bool rs;
    bool rw;
    bool e;

    uint8_t ac;
    uint8_t lines;
    uint8_t columns;
    bool display_on;
    bool cursor_on; // TODO: Render cursor
    bool cursor_blink_on; // TODO: Render blink
    bool entry_mode; // 0 = Decrements, 1 = Increments

    uint8_t ddram[80]; // Display data ram

    bool next_is_lower_4bit;
    uint8_t resetting;
    int8_t busy;
} IO_LCD;

static IO_LCD io_lcd = {0};

static int n_instructions = 0;
static bool step_by_keyboard = false;

static void print_state(CPU cpu) {
    // TODO: Write to a buffer then do one write to stdout.
    printf("\033[2J\033[3J"); // Clear the viewport and the screen, the order seems to be important
    printf("\033[H"); // Position cursor at top-left corner

    printf("CLK   S   O   F   LS   RS   C   ML   MH (ic: %d)\n", n_instructions);
    printf("  %d%4d%4x%4x%5x%5x%4x%5x%5x\n\n", cpu.c_exec, cpu.r_s, cpu.r_o, cpu.r_f, cpu.r_ls, cpu.r_rs, cpu.r_c, cpu.r_ml, cpu.r_mh);

    printf("ZF   CF   OF   SF   SEL ~M/C   ~HALT\n");
    printf("%2d%5d%5d%5d%11d%8d\n\n",
           (cpu.r_f >> 0 & 1),
           (cpu.r_f >> 1 & 1),
           (cpu.r_f >> 2 & 1),
           (cpu.r_f >> 3 & 1),
           cpu.r_sel_m_or_c,
           SIGNAL_HALT(cpu.control_signals));

    printf("ALU Q ZF   ALU Q CF   ALU Q OF   ALU Q IO OE   ALU Q\n");
    printf("%8d%11d%11d%11d%11x\n\n",
           ALU_SIGNAL_Q_ZF(cpu.alu_signals),
           ALU_SIGNAL_Q_CF(cpu.alu_signals),
           ALU_SIGNAL_Q_OF(cpu.alu_signals),
           ALU_SIGNAL_Q_IO_OE(cpu.alu_signals),
           ALU_SIGNAL_Q(cpu.alu_signals));

    printf("C0/CE M   C1/LD O   C2/LD S   C3/LD RS\n");
    printf("%7d%10d%10d%11d\n\n",
           SIGNAL_C0_OR_CE_M(cpu.control_signals),
           SIGNAL_C1_OR_LD_O(cpu.control_signals),
           SIGNAL_C2_OR_LD_S(cpu.control_signals),
           SIGNAL_C3_OR_LD_RS(cpu.control_signals));

    printf("C4 ALU OP4/LD IO   C5 LS ALU Q/HALT C   C3..0   C5..0\n");
    printf("%16d%21d%8x%8x\n\n",
           SIGNAL_C4_ALU_OP4_OR_LD_IO(cpu.control_signals),
           SIGNAL_C5_LS_ALU_Q_OR_HALT_C(cpu.control_signals),
           cpu.control_signals & 0xf,
           cpu.control_signals & 0x3f);

    printf("~LD C   TOGGLE ~M/C   LD MEM   ~LD LS   ~LD ML   ~LD MH\n");
    printf("%5d%14d%9d%9d%9d%9d\n\n",
           SIGNAL_LD_C(cpu.control_signals),
           SIGNAL_TOGGLE_M_C(cpu.control_signals),
           SIGNAL_LD_MEM(cpu.control_signals),
           SIGNAL_LD_LS(cpu.control_signals),
           SIGNAL_LD_ML(cpu.control_signals),
           SIGNAL_LD_MH(cpu.control_signals));

    printf("C ~LD MEM   ~LD O   ~LD S   ~LD RS   ~LD IO\n");
    printf("%9d%8d%8d%9d%9d\n\n",
           SIGNAL_C_LD_MEM(cpu.control_signals, cpu.c_exec),
           SIGNAL_LD_O(cpu.control_signals),
           SIGNAL_LD_S(cpu.control_signals),
           SIGNAL_LD_RS(cpu.control_signals),
           SIGNAL_LD_IO(cpu.control_signals));

    printf("~OE ML   ~OE MH   ~OE ALU   ~OE MEM   OE IO\n");
    printf("%6d%9d%10d%10d%8d\n\n",
           SIGNAL_OE_ML(cpu.control_signals),
           SIGNAL_OE_MH(cpu.control_signals),
           SIGNAL_OE_ALU(cpu.control_signals),
           SIGNAL_OE_MEM(cpu.control_signals),
           C_OE_IO(cpu.r_c));

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

    printf(" A   B   C   D      I      J\n");
    printf("%2x%4x%4x%4x%7x%7x\n\n",
           ram[0x7ff0], ram[0x7ff1], ram[0x7ff2], ram[0x7ff3],
           (ram[0x7ff6] << 8) | ram[0x7ff5],
           (ram[0x7ff8] << 8) | ram[0x7ff7]);

    printf("RAM DUMP at 0x9200 - 0x9203\n");
    printf("%3d %3d %3d %3d => %d\n",
           ram[0x9200 - RAM_ABSOLUTE_START_ADDRESS],
           ram[0x9201 - RAM_ABSOLUTE_START_ADDRESS],
           ram[0x9202 - RAM_ABSOLUTE_START_ADDRESS],
           ram[0x9203 - RAM_ABSOLUTE_START_ADDRESS],

           ram[0x9203 - RAM_ABSOLUTE_START_ADDRESS] << 24 |
               ram[0x9202 - RAM_ABSOLUTE_START_ADDRESS] << 16 |
               ram[0x9201 - RAM_ABSOLUTE_START_ADDRESS] << 8 |
               ram[0x9200 - RAM_ABSOLUTE_START_ADDRESS]);

    if (io_lcd.display_on) {
        printf("╔");
        for (int x = 0; x < io_lcd.columns; ++x) {
            printf("═");
        }
        puts("╗");
        for (int y = 0; y < io_lcd.lines; ++y) {
            printf("║");
            for (int x = 0; x < io_lcd.columns; ++x) {
                int c = io_lcd.ddram[y * 40 + x];

                if (c == 0xef) { // TODO: Create an explicit character map that is over-writable
                    printf("ö");
                } else {
                    putc(c ? c : ' ', stdout);
                }
            }
            puts("║");
        }
        printf("╚");
        for (int x = 0; x < io_lcd.columns; ++x) {
            printf("═");
        }
        puts("╝");
    } else {
        printf("LCD display turned off\n");
    }

    fflush(stdout);
}

static void update_io_ld(CPU cpu) {
    uint8_t port = cpu.r_o & 7;

    if (port == IO_LD_DEBUG_PORT) {
        // Do nothing
    } else if (port == IO_LD_LCD_PORT) {
        bool e_toggled = !io_lcd.e && LCD_SIGNAL_E(cpu.data_bus);

        io_lcd.rs = LCD_SIGNAL_RS(cpu.data_bus);
        io_lcd.rw = LCD_SIGNAL_RW(cpu.data_bus);
        io_lcd.e = LCD_SIGNAL_E(cpu.data_bus);

        if (io_lcd.rs) {
            // Data register

            if (io_lcd.rw) {
                // Read
                assert(0 && "TODO: Read DR");
            } else {
                // Write
                if (e_toggled) {
                    io_lcd.dr = io_lcd.next_is_lower_4bit
                                    ? ((io_lcd.dr & 0xf0) | LCD_SIGNAL_DATA(cpu.data_bus))
                                    : ((uint8_t)(LCD_SIGNAL_DATA(cpu.data_bus) << 4) | (io_lcd.dr & 0x0f));

                    io_lcd.next_is_lower_4bit = (!io_lcd.next_is_lower_4bit) & 1;

                    if (!io_lcd.next_is_lower_4bit) {
                        printf("Got LCD data: 0x%02x AC: %d\n", io_lcd.dr, io_lcd.ac);

                        io_lcd.ddram[io_lcd.ac] = io_lcd.dr;
                        io_lcd.ac = io_lcd.entry_mode ? io_lcd.ac + 1 : io_lcd.ac - 1;

                        if (io_lcd.ac >= 80) {
                            io_lcd.ac = 0;
                        }

                        io_lcd.busy = 1;
                    }
                }
            }

        } else {
            // Instruction register

            if (io_lcd.rw) {
                // Read
                if (e_toggled) {
                    io_lcd.next_is_lower_4bit = (!io_lcd.next_is_lower_4bit) & 1;

                    if (!io_lcd.next_is_lower_4bit) {
                        if (--io_lcd.busy < 0) {
                            io_lcd.busy = 0;
                        };
                    }
                }
            } else {
                // Write
                if (e_toggled) {
                    io_lcd.ir = io_lcd.next_is_lower_4bit
                                    ? ((io_lcd.ir & 0xf0) | LCD_SIGNAL_DATA(cpu.data_bus))
                                    : ((uint8_t)(LCD_SIGNAL_DATA(cpu.data_bus) << 4) | (io_lcd.ir & 0x0f));

                    io_lcd.next_is_lower_4bit = (!io_lcd.next_is_lower_4bit) & 1;

                    if (!io_lcd.next_is_lower_4bit) {
                        printf("Got LCD instruction: 0x%02x\n", io_lcd.ir);

                        if (io_lcd.ir == 0x33) {
                            // Reset sequence start
                            ++io_lcd.resetting;
                        } else if (io_lcd.ir == 0x32) {
                            // Reset sequence end
                            assert(io_lcd.resetting == 1 && !io_lcd.busy);
                            io_lcd.resetting = 0;
                            io_lcd.busy = 4;
                        } else if ((io_lcd.ir & 0xe0) == 0x20) {
                            // Function set
                            uint8_t dl = (io_lcd.ir >> 4) & 1;
                            assert(dl == 0 && "8-bit interface is not supported");

                            uint8_t nf = (io_lcd.ir >> 2) & 3;

                            io_lcd.lines = (nf >> 1) ? 2 : 1;
                            io_lcd.columns = 16; // TODO: Depends on the model
                            io_lcd.busy = 3;

                            printf("LCD lines: %d\n", io_lcd.lines);
                        } else if ((io_lcd.ir & 0xfc) == 0x0c) {
                            // Display on/off control
                            uint8_t d = (io_lcd.ir >> 2) & 1;
                            uint8_t c = (io_lcd.ir >> 1) & 1;
                            uint8_t b = (io_lcd.ir >> 0) & 1;

                            io_lcd.display_on = d;
                            io_lcd.cursor_on = c;
                            io_lcd.cursor_blink_on = b;
                            io_lcd.busy = 1;
                            printf("LCD: Display on: %d   Cursor on: %d   Blink cursor on: %d\n", io_lcd.display_on, io_lcd.cursor_on, io_lcd.cursor_blink_on);
                        } else if ((io_lcd.ir & 0xfe) == 0x02) {
                            // Return home
                            io_lcd.ac = 0;
                            io_lcd.busy = 5;
                            printf("LCD: address counter: %d\n", io_lcd.ac);
                        } else if (io_lcd.ir == 0x01) {
                            // Clear display
                            io_lcd.ac = 0;
                            io_lcd.entry_mode = 1;
                            io_lcd.busy = 5;

                            for (int i = 0; i < 80; ++i) {
                                io_lcd.ddram[i] = ' ';
                            }

                            printf("LCD: address counter: %d\n", io_lcd.ac);
                        } else if ((io_lcd.ir & 0xc0) == 0x40) {
                            // Set CGRAM/DDRAM address
                            io_lcd.ac = io_lcd.ir & 0x3f;
                            io_lcd.busy = 2;
                            printf("LCD: address counter: %d\n", io_lcd.ac);
                        } else {
                            assert(0 && "Unsupported LCD instruction");
                        }
                    }
                }
            }
        }

    } else {
        assert(0 && "Unconfigured IO LD port");
    }
}

static uint8_t update_io_oe(CPU cpu) {
    uint8_t port = cpu.r_o & 7;

    if (port == IO_OE_LCD_PORT) {
        if (io_lcd.e) {
            assert(io_lcd.rw == 1 && "LCD: Read not selected");

            if (io_lcd.rs) {
                assert(0 && "LCD: Reading from DR not yet supported");
            } else {
                // Read busy flag and address counter
                printf("Reading IR: %02x, BUSY: %d\n", io_lcd.ir, io_lcd.busy);

                uint8_t busy_flag = io_lcd.busy ? 1 : 0;
                uint8_t busy_flag_and_ac =
                    io_lcd.next_is_lower_4bit
                        // Upper 4 bit
                        ? (uint8_t)(busy_flag << 7) | (uint8_t)(busy_flag << 3) | ((io_lcd.ac >> 4) & 3)
                        // Lower 4 bit
                        : (io_lcd.ac & 0xf);

                return busy_flag_and_ac;
            }
        } else {
            return 0xff;
        }
    } else {
        print_state(cpu);
        printf("IO port: %d\n", port);
        assert(0 && "IO port has no configured output enable");
    }
}

static uint16_t alu_signals(CPU cpu) {
    uint8_t c_q0 = (cpu.r_c >> 0) & 1;
    uint8_t c_q1 = (cpu.r_c >> 1) & 1;
    uint8_t c_q2 = (cpu.r_c >> 2) & 1;
    uint8_t c_q3 = (cpu.r_c >> 3) & 1;
    uint8_t c_q4 = (cpu.r_c >> 4) & 1;
    uint8_t c_q5 = (cpu.r_c >> 5) & 1;

    uint8_t alu_l_qz = ALU_SIGNAL_L_QZ(cpu.alu_signals);
    uint8_t alu_l_qc = ALU_SIGNAL_L_QC(cpu.alu_signals);

    uint8_t alu_h_qz = ALU_SIGNAL_H_QZ(cpu.alu_signals);
    uint8_t alu_h_qc = ALU_SIGNAL_H_QC(cpu.alu_signals);

    for (int i = 0; i < 5; ++i) {
        uint32_t alu_l_address = (uint32_t)((c_q5 << 16) | (alu_h_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_h_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (F_CF(cpu.r_f) << 8) | ((cpu.r_rs & 0xf) << 4) | (cpu.r_ls & 0xf));
        uint32_t alu_h_address = (uint32_t)((c_q5 << 16) | (alu_l_qc << 15) | (c_q4 << 14) | (c_q3 << 13) | (alu_l_qz << 12) | (c_q2 << 11) | (c_q1 << 10) | (c_q0 << 9) | (F_CF(cpu.r_f) << 8) | ((cpu.r_rs >> 4) << 4) | (cpu.r_ls >> 4));

        uint16_t alu_signals = (uint16_t)(alu_high_rom[alu_h_address] << 8) | alu_low_rom[alu_l_address];

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

static CPU update_cpu(CPU cpu) {
    if (step_by_keyboard) {
        fgetc(stdin);
    }

    if (!SIGNAL_HALT(cpu.control_signals)) {
        step_by_keyboard = true;
        cpu.control_signals ^= (1 << 5);
        return cpu;
    }

    cpu.c_exec = (!cpu.c_exec) & 1;

    if (!cpu.c_exec) { // C SETUP (~C EXEC)
        // Count S
        if (++cpu.r_s >= 0x10) {
            cpu.r_s = 0;
        }

        // Latch S
        if (!SIGNAL_LD_S(cpu.control_signals)) {
            cpu.r_s = 0x0;
        }

        // Latch C
        if (!SIGNAL_LD_C(cpu.control_signals)) {
            cpu.r_c = (uint8_t)((1 << 7) |
                                (ALU_SIGNAL_Q_IO_OE(cpu.alu_signals) << 6) |
                                (SIGNAL_C5_LS_ALU_Q_OR_HALT_C(cpu.control_signals) << 5) |
                                (SIGNAL_C4_ALU_OP4_OR_LD_IO(cpu.control_signals) << 4) |
                                (SIGNAL_C3_OR_LD_RS(cpu.control_signals) << 3) |
                                (SIGNAL_C2_OR_LD_S(cpu.control_signals) << 2) |
                                (SIGNAL_C1_OR_LD_O(cpu.control_signals) << 1) |
                                (SIGNAL_C0_OR_CE_M(cpu.control_signals) << 0));

            cpu.alu_signals = alu_signals(cpu);
        }

        // Count ML/MH
        if (SIGNAL_LD_C(cpu.control_signals) && SIGNAL_C0_OR_CE_M(cpu.control_signals) && SIGNAL_LD_ML(cpu.control_signals)) {
            // TODO: Understand why ++cpu.r_ml gives "runtime error: implicit conversion from type 'int' of value 256 (32-bit, signed) to type 'uint8_t' (aka 'unsigned char') changed the value to 0 (8-bit, unsigned)"
            cpu.r_ml = (u_int8_t)(cpu.r_ml + 1);
            if (cpu.r_ml == 0 && SIGNAL_LD_MH(cpu.control_signals)) {
                ++cpu.r_mh;
            }
        }

        // Latch ML
        if (!SIGNAL_LD_ML(cpu.control_signals)) {
            cpu.r_ml = cpu.data_bus;
        }

        // Latch MH
        if (!SIGNAL_LD_MH(cpu.control_signals)) {
            cpu.r_mh = cpu.data_bus;
        }

        // Toggle SEL ~M/C
        if (SIGNAL_TOGGLE_M_C(cpu.control_signals)) {
            cpu.r_sel_m_or_c = (!cpu.r_sel_m_or_c) & 1;
        }

        uint32_t control_l_address = (uint32_t)((S_Q2(cpu.r_s) << 16) |
                                                (S_Q1(cpu.r_s) << 15) |
                                                (S_Q3(cpu.r_s) << 14) |
                                                (F_SF(cpu.r_f) << 13) |
                                                (S_Q0(cpu.r_s) << 12) |
                                                (F_ZF(cpu.r_f) << 11) |
                                                (0 << 10) |
                                                (F_CF(cpu.r_f) << 9) |
                                                (F_OF(cpu.r_f) << 8) |
                                                cpu.r_o);

        uint32_t control_h_address = (uint32_t)((S_Q2(cpu.r_s) << 16) |
                                                (S_Q1(cpu.r_s) << 15) |
                                                (S_Q3(cpu.r_s) << 14) |
                                                (F_SF(cpu.r_f) << 13) |
                                                (S_Q0(cpu.r_s) << 12) |
                                                (F_ZF(cpu.r_f) << 11) |
                                                (1 << 10) | (F_CF(cpu.r_f) << 9) |
                                                (F_OF(cpu.r_f) << 8) |
                                                cpu.r_o);

        cpu.control_signals = (uint16_t)(control_rom[control_h_address] << 8) |
                              control_rom[control_l_address];

        cpu.address_bus = cpu.r_sel_m_or_c
                              ? (0xfff0 | (cpu.r_c & 0xf))
                              : (uint16_t)(cpu.r_mh << 8) | cpu.r_ml;

        int n_oe = 0;

        // Assert ML to data bus
        if (!SIGNAL_OE_ML(cpu.control_signals)) {
            cpu.data_bus = cpu.r_ml;
            ++n_oe;
        }

        // Assert MH to data bus
        if (!SIGNAL_OE_MH(cpu.control_signals)) {
            cpu.data_bus = cpu.r_mh;
            ++n_oe;
        }

        // Assert ALU to data bus
        if (!SIGNAL_OE_ALU(cpu.control_signals)) {
            cpu.data_bus = ALU_SIGNAL_Q(cpu.alu_signals);
            ++n_oe;
        }

        // Assert MEM to data bus
        if (!SIGNAL_OE_MEM(cpu.control_signals)) {
            if (!SIGNAL_EN_ROM(cpu.address_bus)) {
                cpu.data_bus = rom[cpu.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)];
                ++n_oe;
            }

            if (!SIGNAL_EN_RAM(cpu.address_bus)) {
                cpu.data_bus = ram[cpu.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)];
                ++n_oe;
            }
        }

        if (C_OE_IO(cpu.r_c)) {
            cpu.data_bus = update_io_oe(cpu);
            ++n_oe;
        }

        if (n_oe == 0) {
            cpu.data_bus = 0xff; // Data bus is pulled up
        }

        assert(n_oe <= 1 && "More then one is asserting to the data bus");
    } else { // C EXEC
        bool update_alu_signals = false;

        // Latch O
        if (!SIGNAL_LD_O(cpu.control_signals)) {
            cpu.r_o = cpu.data_bus;
        }

        // Latch RS
        if (!SIGNAL_LD_RS(cpu.control_signals)) {
            cpu.r_rs = cpu.data_bus;

            update_alu_signals = true;
        }

        // Latch LS
        if (!SIGNAL_LD_LS(cpu.control_signals)) {
            cpu.r_ls = cpu.data_bus;

            update_alu_signals = true;
        }

        // Latch RAM (ROM is read only :))
        if (!SIGNAL_C_LD_MEM(cpu.control_signals, cpu.c_exec) && !SIGNAL_EN_RAM(cpu.address_bus)) {
            ram[cpu.address_bus & (RAM_ABSOLUTE_START_ADDRESS - 1)] = cpu.data_bus;
        }

        // Latch F
        if (!SIGNAL_OE_ALU(cpu.control_signals) && !SIGNAL_LD_LS(cpu.control_signals)) {
            cpu.r_f = (uint8_t)((ALU_SIGNAL_Q_SF(cpu.alu_signals) << 3) |
                                (ALU_SIGNAL_Q_OF(cpu.alu_signals) << 2) |
                                (ALU_SIGNAL_Q_CF(cpu.alu_signals) << 1) |
                                (ALU_SIGNAL_Q_ZF(cpu.alu_signals) << 0));

            update_alu_signals = true;
        }

        // Latch IO
        if (!SIGNAL_LD_IO(cpu.control_signals)) {
            io_ports[cpu.r_o & 7] = cpu.data_bus;

            update_io_ld(cpu);
        }

        if (update_alu_signals) {
            cpu.alu_signals = alu_signals(cpu);
        }
    }

    return cpu;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Missing program\n");
        exit(1);
    }

    uint32_t clock_hz = argc > 2
                            ? (uint32_t)strtoul(argv[2], NULL, 10)
                            : 20;

    if (clock_hz < 1 || clock_hz > 16000000) {
        fprintf(stderr, "Unsupported clock rate: %u\n", clock_hz);
        exit(1);
    }

    FILE *file = fopen(argv[1], "r");
    assert(file != NULL && "Failed to read program");

    fseek(file, 0, SEEK_END);
    long program_size = ftell(file);
    assert(program_size >= 0);
    fseek(file, 0, SEEK_SET);
    assert(program_size <= (RAM_SIZE - PROGRAM_RAM_RELATIVE_START_ADDRESS) && "Program too big");

    size_t read_bytes = fread(ram + PROGRAM_RAM_RELATIVE_START_ADDRESS, sizeof(uint8_t), (size_t)program_size, file);
    assert(read_bytes == (size_t)program_size && "Failed to read entire contents of program");
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

    rom[0] = OPCODE_JMP_IMM16;
    rom[1] = (RAM_ABSOLUTE_START_ADDRESS + PROGRAM_RAM_RELATIVE_START_ADDRESS) & 0xff;
    rom[2] = (RAM_ABSOLUTE_START_ADDRESS + PROGRAM_RAM_RELATIVE_START_ADDRESS) >> 8;

    // Reset by running an initial setup phase where S is 0 afterwards.
    CPU state = update_cpu((CPU){.c_exec = 1,
                                 .r_s = 0xf});

    struct timespec ts = {
        .tv_sec = clock_hz <= 1 ? 1 : 0,
        .tv_nsec = clock_hz <= 1 ? 0 : (1000000000 / clock_hz),
    };

    while (1) {
        // Execute
        state = update_cpu(state);
        print_state(state);

        nanosleep(&ts, NULL);

        if (!SIGNAL_LD_S(state.control_signals)) {
            ++n_instructions;

            if (n_instructions >= EXIT_AFTER_N_INSTRUCTIONS) {
                break;
            }
        }

        // Setup
        state = update_cpu(state);
        print_state(state);

        nanosleep(&ts, NULL);
    }

    return 0;
}
