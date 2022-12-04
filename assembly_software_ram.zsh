#!/bin/zsh

# -e exit immediately when a command fails.
# -u treat unset variables as an error and exit immediately.
# -x print each command before executing it.
# -o pipefail sets the exit code of a pipeline to that of the rightmost command to exit with a non-zero status.
set -euo pipefail

mkdir -p ./bin/software/ram

declare -i i=0

for FILE in software/ram/*.asm
do
  out_file=./bin/software/ram/${${FILE:t}:r}
  out_file_actual=${out_file}.bin

  customasm -q customasm_ram.asm $FILE -o $out_file_actual
  echo "$i: $FILE"
  i+=1
  customasm -q customasm_ram.asm $FILE -p
done
