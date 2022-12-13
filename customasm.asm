#bits 8

#ruledef {
    nop => 0x00
    halt => 0xff

    ld a, {imm: i8} => 0x01 @ imm
    ld b, {imm: i8} => 0x02 @ imm
    ld c, {imm: i8} => 0x03 @ imm
    ld d, {imm: i8} => 0x04 @ imm

    ld i, {imm: i16} => 0x05 @ le(imm)
    ld j, {imm: i16} => 0x06 @ le(imm)

    ld a, [i] => 0x07
    ld a, [j] => 0x08

    ld a, [i++] => 0x09
    ld a, [j++] => 0x0a

    ld [i], a => 0x0b
    ld [j], a => 0x0c

    ld [i++], a => 0x0d
    ld [j++], a => 0x0e

    ; ld a, a => 0x20
    ld a, b => 0x21
    ld a, c => 0x22
    ld a, d => 0x23
    ld b, a => 0x24
    ; ld b, b => 0x25
    ld b, c => 0x26
    ld b, d => 0x27
    ld c, a => 0x28
    ld c, b => 0x29
    ; ld c, c => 0x2a
    ld c, d => 0x2b
    ld d, a => 0x2c
    ld d, b => 0x2d
    ld d, c => 0x2e
    ; ld d, d => 0x2f

    inc a => 0x40
    shl a => 0x41
    shr a => 0x42
    not a => 0x43
    dec a => 0x44
    ror a => 0x45

    add a, b => 0x50
    or a, b => 0x51
    and a, b => 0x52
    xor a, b => 0x53

    dec b => 0x60
    dec c => 0x61

    in a, {port: u3} => (0x80 + port)`8
    out {port: u3}, a => (0x88 + port)`8
    ; 0x90

    jmp {imm: i16} => 0x92 @ le(imm)
    jz  {imm: i16} => 0x93 @ le(imm)
    jnz {imm: i16} => 0x94 @ le(imm)
    jc  {imm: i16} => 0x95 @ le(imm)
    jnc {imm: i16} => 0x96 @ le(imm)
    jo {imm: i16} => 0x97 @ le(imm)
    jno {imm: i16} => 0x98 @ le(imm)
    js {imm: i16} => 0x99 @ le(imm)
    jns {imm: i16} => 0x9a @ le(imm)

    ld sp, {imm: i8} => 0xa0 @ imm

    push a => 0xa2
    push b => 0xa3
    push c => 0xa4
    push d => 0xa5
    push i => 0xa6
    push j => 0xa7

    pop a => 0xa8
    pop b => 0xa9
    pop c => 0xaa
    pop d => 0xab
    pop i => 0xac
    pop j => 0xad

    call {imm: i16} => 0xb0 @ le(imm)
    ret => 0xb1

    ld a, [sp-{imm:u8}] => 0xb2 @ (-imm)`8
    ld a, [sp+{imm:u8}] => 0xb2 @ imm`8
}