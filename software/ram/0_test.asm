#include "../../customasm.asm"

DEBUG_PORT = 1

start:

ld a, 1
out DEBUG_PORT, a
ld a, 2
out DEBUG_PORT, a
ld a, 4
out DEBUG_PORT, a
ld a, 8
out DEBUG_PORT, a
ld a, 0x10
out DEBUG_PORT, a
ld a, 0x20
out DEBUG_PORT, a
ld a, 0x40
out DEBUG_PORT, a
ld a, 0x80
out DEBUG_PORT, a
ld a, 0x40
out DEBUG_PORT, a
ld a, 0x20
out DEBUG_PORT, a
ld a, 0x10
out DEBUG_PORT, a
ld a, 8
out DEBUG_PORT, a
ld a, 4
out DEBUG_PORT, a
ld a, 2
out DEBUG_PORT, a
ld a, 1
out DEBUG_PORT, a

jmp start
