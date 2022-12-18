#include "../../customasm.asm"

DEBUG_PORT = 1

LCD_PORT = 2

start:

; Reset sequence
nop

ld a, (0b0010 << 4) | 0b0011
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

nop
nop
nop

ld a, (0b0010 << 4) | 0b0011
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

nop
nop


ld a, (0b0010 << 4) | 0b0011
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

nop

; Function set in 4-bit interface
; Init 4-bit interface
; 0 0 1 DL NF x x
; DL = 0 - 4 bit mode, 1 - 8 bit mode
; ld a, 0b001_1_01_00
ld a, (0b0010 << 4) | 0b0010 ; 1(DL)
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

; NF = 00 - 1 line, 5×8 dots font, 1/8 duty factor
; NF = 01 - 1 line, 5×10 dots font, 1/11 duty factor
; NF = 1x - 2 lines, 5×8 dots font, 1/16 duty factor
ld a, (0b0010 << 4) | 0b1000 ; NF**
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

nop

ld a, (0b0010 << 4) | 0b0010 ; 1(DL)
out LCD_PORT, a
ld a, 0
out LCD_PORT, a

; Display on/off control:
ld a, (0b0010 << 4) | 0b0000
out LCD_PORT,a
ld a, 0
out LCD_PORT, a

; D = 0; Display off
; C = 0; Cursor off
; B = 0; Blinking off
ld a, (0b0010 << 4) | 0b1101 ; 1DCB
out LCD_PORT,a
ld a, 0
out LCD_PORT, a


; Clear display
ld a, (0b0010 << 4) | 0b0000
out LCD_PORT,a
ld a, 0
out LCD_PORT, a

ld a, (0b0010 << 4) | 0b0001
out LCD_PORT,a
ld a, 0
out LCD_PORT, a

nop
nop
nop

; Return home
ld a, (0b0010 << 4) | 0b0000
out LCD_PORT,a
ld a, 0
out LCD_PORT, a

ld a, (0b0010 << 4) | 0b0010
out LCD_PORT,a
ld a, 0
out LCD_PORT, a

ld i, message
call write_string

loop:
jmp loop

write_string:
    ld b, 0
    ld a, [i++]
    or a, b
    jz .done

    ld c, a

    shr a
    shr a
    shr a
    shr a
    ld b, (0b1010 << 4) | 0b0000
    or a, b
    out LCD_PORT, a
    ld a, 0
    out LCD_PORT, a

    ld a, c
    ld b, 0b00001111
    and a, b
    ld b, (0b1010 << 4) | 0b0000
    or a, b
    out LCD_PORT, a
    ld a, 0
    out LCD_PORT, a

    jmp write_string

    .done:
    ret

message:
    #d "Hellorld!\0"
