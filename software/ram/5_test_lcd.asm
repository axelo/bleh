#include "../../customasm.asm"

DEBUG_PORT = 1

LCD_PORT = 2

RS_IR = 0 << 7
RS_DR = 1 << 7

RW_WRITE = 0 << 6
RW_READ = 1 << 6

E_HIGH = 1 << 5
E_LOW = 0 << 5

start:

; Reset sequence
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0011
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

nop
nop
nop
nop

out LCD_PORT, RS_IR | RW_WRITE | E_HIGH  | 0b0011
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

nop
nop
nop
nop

out LCD_PORT, RS_IR | RW_WRITE | E_HIGH  | 0b0011
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

nop
nop
nop
nop

out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010

; Function set in 4-bit interface
; Init 4-bit interface
; 0 0 1 DL NF x x
; DL = 0 - 4 bit mode, 1 - 8 bit mode
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010 ; 1(DL)
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010

; NF = 00 - 1 line, 5×8 dots font, 1/8 duty factor
; NF = 01 - 1 line, 5×10 dots font, 1/11 duty factor
; NF = 1x - 2 lines, 5×8 dots font, 1/16 duty factor
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b1000 ; NF**
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b1000

; ld b, 0x08
out LCD_PORT, RS_IR | RW_READ | E_LOW
wait1:
out LCD_PORT, RS_IR | RW_READ | E_HIGH
in a, LCD_PORT

out LCD_PORT, RS_IR | RW_READ | E_LOW
out LCD_PORT, RS_IR | RW_READ | E_HIGH
out LCD_PORT, RS_IR | RW_READ | E_LOW

shl a
jc wait1

; Display on/off control:
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

; D = 0; Display off
; C = 0; Cursor off
; B = 0; Blinking off
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b1101 ; 1DCB
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b1101

loop:

; ld b, 0x08
out LCD_PORT, RS_IR | RW_READ | E_LOW
wait2:
out LCD_PORT, RS_IR | RW_READ | E_HIGH
in a, LCD_PORT

out LCD_PORT, RS_IR | RW_READ | E_LOW
out LCD_PORT, RS_IR | RW_READ | E_HIGH
out LCD_PORT, RS_IR | RW_READ | E_LOW

; and a, b
shl a
jc wait2

; Clear display
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0001
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0001

; ld b, 0x08
out LCD_PORT, RS_IR | RW_READ | E_LOW
wait3:
out LCD_PORT, RS_IR | RW_READ | E_HIGH
in a, LCD_PORT

out LCD_PORT, RS_IR | RW_READ | E_LOW
out LCD_PORT, RS_IR | RW_READ | E_HIGH
out LCD_PORT, RS_IR | RW_READ | E_LOW

shl a
jc wait3

ld b, 1
ld a, 255
loop2:
dec a
jnz loop2
dec b
jnz loop2

; Return home
out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010
out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010

; ld b, 0x08
out LCD_PORT, RS_IR | RW_READ | E_LOW
wait4:
out LCD_PORT, RS_IR | RW_READ | E_HIGH
in a, LCD_PORT

out LCD_PORT, RS_IR | RW_READ | E_LOW
out LCD_PORT, RS_IR | RW_READ | E_HIGH
out LCD_PORT, RS_IR | RW_READ | E_LOW

shl a
jc wait4

ld i, message
call write_string

ld b, 1
ld a, 255
loop3:
dec a
jnz loop3
dec b
jnz loop3

jmp loop

write_string:
    ld b, 0
    ld a, [i++]
    or a, b
    jz .done

    ld c, a

    ;ld b, 0x08
    out LCD_PORT, RS_IR | RW_READ | E_LOW
    .wait5:
    out LCD_PORT, RS_IR | RW_READ | E_HIGH
    in a, LCD_PORT

    out LCD_PORT, RS_IR | RW_READ | E_LOW
    out LCD_PORT, RS_IR | RW_READ | E_HIGH
    out LCD_PORT, RS_IR | RW_READ | E_LOW

    shl a
    jc .wait5

    ld a, c

    ; Send high nibble
    shr a
    shr a
    shr a
    shr a
    ld b, RS_DR | E_HIGH
    or a, b
    out LCD_PORT, a
    ld b, !E_HIGH
    and a, b
    out LCD_PORT, a

    ; Send low nibble
    ld a, c
    ld b, 0xf
    and a, b
    ld b, RS_DR | E_HIGH
    or a, b
    out LCD_PORT, a
    ld b, !E_HIGH
    and a, b
    out LCD_PORT, a

    jmp write_string

    .done:
    ret

message:
    #d "Hellorld!\0"
