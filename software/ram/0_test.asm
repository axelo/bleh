#include "../../customasm.asm"

start:

ld a, 1
ld a, 2
ld a, 3
ld a, 4
ld b, 1
ld b, 2
ld b, 3
ld b, 4

ld c, 0
ld c, a

ld d, 0
ld d, b

inc a
ld c, a

ld b, 0xf0
ld b, a
ld d, b

jmp start