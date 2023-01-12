#include "../../customasm.asm"

DEBUG_PORT = 1

start:

ld a, (1 << 7) | (1 << 0)
out DEBUG_PORT, a

ld a, (1 << 6) | (1 << 1)
out DEBUG_PORT, a

ld a, (1 << 5) | (1 << 2)
out DEBUG_PORT, a

ld a, (1 << 4) | (1 << 3)
out DEBUG_PORT, a

ld a, (1 << 3) | (1 << 4)
out DEBUG_PORT, a

ld a, (1 << 2) | (1 << 5)
out DEBUG_PORT, a

ld a, (1 << 1) | (1 << 6)
out DEBUG_PORT, a

ld a, (1 << 0) | (1 << 7)
out DEBUG_PORT, a

ld a, (1 << 1) | (1 << 6)
out DEBUG_PORT, a

ld a, (1 << 2) | (1 << 5)
out DEBUG_PORT, a

ld a, (1 << 3) | (1 << 4)
out DEBUG_PORT, a

ld a, (1 << 4) | (1 << 3)
out DEBUG_PORT, a

ld a, (1 << 5) | (1 << 2)
out DEBUG_PORT, a

ld a, (1 << 6) | (1 << 1)
out DEBUG_PORT, a

ld a, (1 << 7) | (1 << 0)
out DEBUG_PORT, a

jmp start
