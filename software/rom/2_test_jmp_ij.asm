#include "../../customasm.asm"

start:

ld a, 0xba
ld i, my_label
ld j, my_other_label

jmp i

my_other_label:
ld a, 0x45

jmp start

#addr 0x1234
my_label:
    ld a, 0x12
    jmp j
