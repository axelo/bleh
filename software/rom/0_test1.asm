#include "../../customasm.asm"

BOOT_PORT = 0
BOOT_COMMAND_GET_BYTES = 0xab

DEBUG_PORT = 1

ld sp, 0xff

wait_until_boot_device_ready:
    in a, BOOT_PORT
    out DEBUG_PORT, a
    dec a
    jnz wait_until_boot_device_ready

loop:
ld a, BOOT_COMMAND_GET_BYTES
out BOOT_PORT, a

in a, BOOT_PORT     ; Low size
out DEBUG_PORT, a

in a, BOOT_PORT     ; High size
out DEBUG_PORT, a

in a, BOOT_PORT     ; First byte
out DEBUG_PORT, a

in a, BOOT_PORT     ; Second byte
out DEBUG_PORT, a

jmp loop
