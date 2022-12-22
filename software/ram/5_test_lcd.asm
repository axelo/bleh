#include "../../customasm.asm"

DEBUG_PORT = 1

out DEBUG_PORT, 0b10101010

call lcd_reset
call lcd_function_set_4bit_2_lines_5x8_dots_font
call lcd_display_on_cursor_on_blink_off

loop:
    call lcd_return_home
    call lcd_clear_display

    ld i, message1
    call lcd_write_string

    call lcd_set_cgram_address

    call delay

    ld i, message2
    call lcd_write_string

    call delay

    jmp loop

delay:
    ld b, 10
    ld a, 255
    loop3:
    dec a
    jnz loop3
    dec b
    jnz loop3
    ret

message1:
    #d "God Jul ", 0b1110_1111, "nskar..\0"

message2:
    #d "   BLEH-1!!!  \0"


#include "lcd.asm"