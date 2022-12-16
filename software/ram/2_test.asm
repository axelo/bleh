#include "../../customasm.asm"

DEBUG_PORT = 1


ld a, 1
ld b, 2
ld c, 3

start:

ld a, 1
out DEBUG_PORT, a
out DEBUG_PORT, a

dec c
dec c

ld a, 2
out DEBUG_PORT, a
out DEBUG_PORT, a

dec b
dec b

jmp start

jmp start
jmp start
jmp start
jmp start
jmp start
jmp start
jmp start
jmp start
jmp start