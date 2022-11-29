#!/bin/zsh

# -e exit immediately when a command fails.
# -u treat unset variables as an error and exit immediately.
# -o pipefail sets the exit code of a pipeline to that of the rightmost command to exit with a non-zero status.
set -euo pipefail

#./run.zsh control.c
./run.zsh "control 5.4.c"

#read $'?\nPlace LOW rom into minipro then press [ENTER] to program bin\/control_low.bin\n'
#minipro -p SST39SF010A --write bin/control_low.bin

#read $'?\nPlace HIGH rom into minipro then press [ENTER] to program bin\/control_high.bin\n'
#minipro -p SST39SF010A --write bin/control_high.bin

read $'?\nPlace control rom into minipro then press [ENTER] to program bin\/control_5.4.bin\n'
minipro -p SST39SF010A --write bin/control_5.4.bin
