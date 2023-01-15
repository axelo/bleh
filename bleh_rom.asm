; For programs loaded from ROM
#include "./bleh_instructions.asm"

#bankdef rom
{
    #addr 0x0000
    #size 0x8000
    #outp 0
    #fill
}

#bankdef ram
{
    #addr 0x0000
    #size 0x8000
}

#bank rom