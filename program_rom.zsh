#!/bin/zsh

set -euo pipefail

./assembly_software_rom.zsh

read $'?\nPlace SOFTWARE rom into minipro then press [ENTER] to program bin\/software_rom.bin\n'

minipro -p SST39SF040 --write bin/software_rom.bin
