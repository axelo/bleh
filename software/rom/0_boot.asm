#include "../../customasm.asm"

PROGRAM_PORT = 0
PROGRAM_ADDRESS = $ ; Set by current bank.

ld sp, 0xff

ld a, 0xaa ; Clear counter
out PROGRAM_PORT, a

in a, PROGRAM_PORT ; Read low size into a
ld b, a ; b = low size
in a, PROGRAM_PORT ; Read high size into a
inc a   ; Convert to number of 256 byte pages
ld c, a ; c = number of pages to load

start_read_bytes:

ld i, PROGRAM_ADDRESS ; Program destination in RAM

loop_read_bytes:
in a, PROGRAM_PORT ; Read byte into a
ld [i++], a ; Store read byte into RAM

dec b
jnz loop_read_bytes

dec c
jnz loop_read_bytes

; Temp indicator that we're done.
ld a, 0xaa
ld b, 0xbb
ld c, 0xcc
ld d, 0xdd

jmp PROGRAM_ADDRESS
