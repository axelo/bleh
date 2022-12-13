#!/bin/zsh

# -e exit immediately when a command fails.
# -u treat unset variables as an error and exit immediately.
# -x print each command before executing it.
# -o pipefail sets the exit code of a pipeline to that of the rightmost command to exit with a non-zero status.
set -euo pipefail

./assembly_software_rom.zsh

read $'?\nPlace SOFTWARE rom into minipro then press [ENTER] to program bin\/software_rom.bin\n'

minipro -p SST39SF040 --write bin/software_rom.bin
