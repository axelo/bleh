#include "../bleh_rom.asm"

BOOT_PORT = 0
BOOT_COMMAND_GET_BYTES = 0xab

DEBUG_PORT = 1

BOOT_ADDRESS = 0x8000

ld sp, 0xff

wait_until_boot_device_ready:
    in a, BOOT_PORT
    out DEBUG_PORT, a
    dec a
    jnz wait_until_boot_device_ready

ld a, BOOT_COMMAND_GET_BYTES
out BOOT_PORT, a

in a, BOOT_PORT ; Read low size into a
ld b, a         ; b = low size
in a, BOOT_PORT ; Read high size into a
inc a           ; Convert to number of 256 byte pages
ld c, a         ; c = number of pages to load

start_read_bytes:
ld i, BOOT_ADDRESS ; Program destination in RAM

loop_read_bytes:
ld a, b
out DEBUG_PORT, a ; Debug low bytes left

in a, BOOT_PORT ; Read byte into a
ld [i++], a     ; Store read byte into RAM

dec b
jnz loop_read_bytes

dec c
jnz loop_read_bytes

; Temp indicator that we're done.
ld a, 0xaa
ld b, 0xbb
ld c, 0xcc
ld d, 0xdd

; Run program
jmp BOOT_ADDRESS