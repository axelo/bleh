# BLEH-1

Simple 8-bit processor with an 8-bit data bas, a 16-bit address bus and 8 in/out ports. Code and data is shared (Von Neumann architecture).

## Logic blocks

### Memory backed registers

The last 16 bytes of memory are used for storing data and address registers (instruction registers) in addition to temporary storage during microcode steps.

8-bit data registers: `a`, `b`, `c` and `d`

16-bit address registers: `i` and `j`

8-bit special registers: `sp`

### ALU

Two 8-bit registers `LS` and `RS` makes up the operands to the ALU. Neither are accessible from software. Two ROMs makes up a lookup table for different ALU operation. A 4-bit flag register is available for storing `zf`, `cf`, `of` and `sf`.

### Control

Two ROMs together with a 4-bit step counter, an 8-bit opcode register and the flags register makes up a lookup table for different microcode steps for different instructions. The step counter is cleared on reset.

### Memory

Two 8-bit register `ML` and `MH` makes up the 16-bit address bus. Neither are accessible from software. The registers are cleared on reset making program execution start at `0x0000`.

#### Stack

The high part of the stack is locked at `0xff`. The low part is loaded by setting the `sp` instruction register. The stack grows upwards towards `0xffff`. Usually the `sp` is loaded with `0xff` making the first `push` at `0xff00`.

#### Constant and ML/MH toggle

One control signal toggles if the address bus is based on `ML`/`MH` or the low 4-bits of the constant register `or`:ed by `0xfff0`. This is used to address the instruction registers during microcode steps.

#### Memory map

    0x0000 - 0x7fff: ROM
    0x8000 - 0xfff0: RAM
    0xfff0 - 0xffff: RAM containing instruction registers

### Clock

The input clock source is divided into two equally long phases named setup and execute. The setup phase settles the control signals and the execute phase latches the input source (the data bus for example).

### Data bus

The data bus is pulled high so if nothing is asserted then `0xff` will be read.

## Software

### Loading software

Using the [arduino boot device](./arduino/BootDeviceSketch/BootDeviceSketch.ino), connect a serial monitor and paste Intel Hex format of the program, for example:

    customasm -q customasm_ram.asm ./software/ram/3_test.asm --print --format intelhex

