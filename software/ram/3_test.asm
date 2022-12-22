#include "../../customasm.asm"

DEBUG_PORT = 1

start:

out DEBUG_PORT, 1 << 0
out DEBUG_PORT, 1 << 1
out DEBUG_PORT, 1 << 2
out DEBUG_PORT, 1 << 3

ld a, 1 << 4
out DEBUG_PORT, a

ld a, 1 << 5
out DEBUG_PORT, a

ld a, 1 << 6
out DEBUG_PORT, a

ld a, 1 << 7
out DEBUG_PORT, a

ld a, 1 << 7
out DEBUG_PORT, a

out DEBUG_PORT, 1 << 7
out DEBUG_PORT, 1 << 6
out DEBUG_PORT, 1 << 5
out DEBUG_PORT, 1 << 4
out DEBUG_PORT, 1 << 3
out DEBUG_PORT, 1 << 2
out DEBUG_PORT, 1 << 1
out DEBUG_PORT, 1 << 0

jmp start
