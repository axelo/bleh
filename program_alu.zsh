#!/bin/zsh

# -e exit immediately when a command fails.
# -u treat unset variables as an error and exit immediately.
# -o pipefail sets the exit code of a pipeline to that of the rightmost command to exit with a non-zero status.
set -euo pipefail

./run.zsh alu.c

read $'?\nPlace ALU LOW rom into minipro then press [ENTER] to program bin\/alu_low.bin\n'
minipro -p SST39SF010A --write bin/alu_low.bin

read $'?\nPlace ALU HIGH rom into minipro then press [ENTER] to program bin\/alu_high.bin\n'
minipro -p SST39SF010A --write bin/alu_high.bin
