; For programs loaded into RAM

#bankdef rom
{
    #addr 0x0000
    #size 0x8000
}

#bankdef ram
{
    #addr 0x8000
    #size 0x8000
    #outp 0
}

#bank ram