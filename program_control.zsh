#!/bin/zsh

set -euo pipefail

./compile_and_run.zsh "control.c"

read $'?\nPlace LOW rom into minipro then press [ENTER] to program bin\/control.bin\n'
minipro -p SST39SF010A --write bin/control.bin

read $'?\nPlace HIGH rom into minipro then press [ENTER] to program bin\/control.bin\n'
minipro -p SST39SF010A --write bin/control.bin
