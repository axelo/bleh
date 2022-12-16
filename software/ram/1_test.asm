#include "../../customasm.asm"

DEBUG_PORT = 1

start:

ld a, 1
out DEBUG_PORT, a
nop
nop
out DEBUG_PORT, a
nop
nop
ld c, 1
nop
nop
dec c
nop
nop
ld a, 2
out DEBUG_PORT, a
nop
nop
dec c
nop
nop
ld a, 3
out DEBUG_PORT, a
nop
nop

jmp start
