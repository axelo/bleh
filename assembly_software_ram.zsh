#!/bin/zsh

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
