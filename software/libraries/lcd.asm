RS_IR = 0 << 7
RS_DR = 1 << 7

RW_WRITE = 0 << 6
RW_READ = 1 << 6

E_HIGH = 1 << 5
E_LOW = 0 << 5

lcd_init:
    call lcd_reset
    call lcd_function_set_4bit_2_lines_5x8_dots_font
    call lcd_display_on_cursor_off_blink_off
    call lcd_clear_display
    ret

lcd_reset:
    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0011
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

    nop ; TODO: Delay 4.3ms
    nop
    nop
    nop

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH  | 0b0011
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

    nop ; TODO: Delay 4.3ms
    nop
    nop
    nop

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH  | 0b0011
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0011

    nop ; TODO: Delay 100 us
    nop
    nop
    nop

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010

    ret

lcd_function_set_4bit_2_lines_5x8_dots_font:
    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    ; Function set in 4-bit interface
    ; Init 4-bit interface
    ; 0 0 1 DL NF x x
    ; DL = 0 - 4 bit mode, 1 - 8 bit mode
    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010 ; 4-bit interface
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010

    ; NF = 00 - 1 line, 5×8 dots font, 1/8 duty factor
    ; NF = 01 - 1 line, 5×10 dots font, 1/11 duty factor
    ; NF = 1x - 2 lines, 5×8 dots font, 1/16 duty factor
    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b1000 ; NF**
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b1000

    ret

lcd_display_on_cursor_off_blink_off:
    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

    ; D = 0; Display off
    ; C = 0; Cursor off
    ; B = 0; Blinking off
    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b1100 ; 1DCB
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b1100
    ret

lcd_return_home:
    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0010
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0010
    ret

lcd_clear_display:
    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0000
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0000

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0001
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0001
    ret

lcd_set_cgram_address:
    ; TODO: Currently hard coded for line two (0x28 (40))
    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b0110
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b0110

    out LCD_PORT, RS_IR | RW_WRITE | E_HIGH | 0b1000
    out LCD_PORT, RS_IR | RW_WRITE | E_LOW | 0b1000
    ret

lcd_write_char:
    ; Input:
    ;   a: char to write
    ; Destroys:
    ;   a, b, j
    ld b, a

    ld j, .not_busy
    jmp _lcd_busy_wait_ret_to_j
    .not_busy:

    ; Write high nibble
    ld a, b
    shr a
    shr a
    shr a
    shr a
    or a, RS_DR | E_HIGH
    out LCD_PORT, a
    and a, !E_HIGH
    out LCD_PORT, a

    ; Send low nibble
    ld a, b
    and a, 0xf
    or a, RS_DR | E_HIGH
    out LCD_PORT, a
    and a, !E_HIGH
    out LCD_PORT, a

    ret

lcd_write_string:
    ; Input:
    ;   i: address to 0 terminated string
    ; Destroys:
    ;   a, b, i, j

    ld a, [i++]
    or a, 0
    jz .done

    call lcd_write_char

    jmp lcd_write_string

    .done:
    ret

_lcd_busy_wait_ret_to_j:
    out LCD_PORT, RS_IR | RW_READ | E_LOW

    .try_again:
    out LCD_PORT, RS_IR | RW_READ | E_HIGH
    in a, LCD_PORT
    out LCD_PORT, RS_IR | RW_READ | E_LOW

    out LCD_PORT, RS_IR | RW_READ | E_HIGH
    out LCD_PORT, RS_IR | RW_READ | E_LOW

    shl a
    jc .try_again
    jmp j
