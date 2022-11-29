#include "../customasm.asm"

ld sp, 0xff

ld a, 1
ld b, 2
ld c, 3

push a
push b
push c

pop d

ld a, [sp+1]
ld a, [sp-0]
ld a, [sp-1]


ld a, 2
dec a
jz zero

ld sp, 0xdd

zero:
ld sp, 0xff

call proc1

ld i, 0x1234
ld j, 0xabcd

push i
push j

pop i
pop j

ld a, 0x81
ld b, 0xb1
ld c, 0xc1
ld d, 0xd1

push a
push b
push c
push d

pop a
pop b
pop c
pop d

ror a
ror a

shl a
shl a
shl a

ld b, 0x2
or a, b

in a, 1

; ld a, 0xfa
out 1, a

jmp boom

ld i, 0x1234

boom:
ld j, 0xabcd

; push si
; push di
; pop si
; pop di

ld a, 0xa
ld b, 0xb
ld c, 0xc
ld d, 0xd

; push a
; push b
; push c
; push d

; ld a, 0xff
; ld b, 0xff
; ld c, 0xff
; ld d, 0xff

; ld a, [sp-0]
; ld a, [sp-1]
; ld a, [sp-2]

; pop a
; pop b
; pop c
; pop d


; in a, 1
; out 1, a

; call proc1

; ld si, 0x1234
; push si

; ld si, 0xaabb
; pop si

; ld a, 1
; push a

; ld a, 2
; push a

; ld a, 3
; push a

; ld a, 4

; pop a
; pop a
; pop a

; add b, a

; or a, a
; ld a, 0
; or a, a
; jz go

; ld a, 0xa
; go:
; ld b, 0xb

; jmp setup

ld b, a
ld c, b
ld d, c

; ld i, 0xfff1
; ld a, 0xba
; ld [i++], a
; ld [i], a

ld i, message
ld a, [i++]
ld a, [i]

; ld si, 0x1234
; ld di, 0xabcd

; ld a, 0x1
; ld b, 0x2
; ld c, 0x3
; ld d, 0xd0
; ld a, 0x2
; ld b, 0x3
; ld c, 0x4
; ld d, 0xd1
; ld a, 0x5
; ld b, 0x6
; ld c, 0x7
; ld d, 0xd2
; ld a, 0x8
; ld b, 0x9
; ld c, 0xa
; ld d, 0xd3





; ld sp

; halt

; ld a, 0x11
; push a
; ld a, 0x22
; push a
; ld a, [sp-2]

; call proc1

; ld a, 0x1
; push a
; push a
; push a
; ld a, 0xff
; pop a
; ld a, 0xfe
; pop a
; ld a, 0xfd
; pop a

; ld a, 0xa
; ld b, a
; ld c, b
; ld d, c

; ;halt

; ld si, 0xfff2
; ld a, 0xba
; ld [si++], a
; ld [si], a

; ld a, [si++]
; ld a, [si]

; ld a, 1
; ld b, 2

; nop

; ld a, 0xff
; inc a
; jnz setup

; ld a, 0x60
; ld b, 9
; add a, b

; jmp setup

; setup_done:
; ld a, 0x88
; out 5, a

; ld a, 0x12
; in a, 5

; ; ld si, 0x1234
; ; push si
; ; pop di
; ; pop a
; ; pop a

; ld a, 0x11
; push a
; ld a, 0x22
; push a
; ld a, 0x33
; push a
; ld a, 0xff
; pop a
; pop a
; pop a

; ld a, 0x12
; ld b, a
; ld c, b
; ld d, c

; ld si, buffer
; ld a, 0xba
; ld [si++], a
; ld [si], a

; ld si, message

; ld a, [si++]
; ld a, [si]



; ld a, 1
; ld b, 0x22
; ld c, 0x33
; ld d, 0x44

; nop
; halt

#addr 0x1201
message: #d  "Hello!\0"

; setup:
;     ld a, 0x1
;     ld b, 0xfe
;     jmp setup_done

proc1:
    ld a, 0x11
    call proc2
    ld a, 0x01
    ret

proc2:
    ld a, 0x22
    call proc3
    ld a, 0x02
    ret

proc3:
    ld a, 0x33
    ret

#bank ram
#addr 0x8010
message_ptr: #res 2

buffer: #res 32
