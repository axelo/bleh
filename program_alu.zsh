#!/bin/zsh

set -euo pipefail

./compile_and_run.zsh alu.c

read $'?\nPlace ALU LOW rom into minipro then press [ENTER] to program bin\/alu_low.bin\n'
minipro -p SST39SF010A --write bin/alu_low.bin

read $'?\nPlace ALU HIGH rom into minipro then press [ENTER] to program bin\/alu_high.bin\n'
minipro -p SST39SF010A --write bin/alu_high.bin
