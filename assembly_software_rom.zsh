#!/bin/zsh

set -euo pipefail

mkdir -p ./bin/software/rom

in_files=(software/boot_device_loader.asm)
out_files=()

declare -i i=0

for FILE in $in_files
do
  out_file=./bin/software/rom/${${FILE:t}:r}
  out_file_filled=${out_file}.bin

  customasm -q $FILE -o $out_file_filled
  echo "$i: $FILE"
  i+=1
  customasm -q $FILE -p
  out_files+=($out_file_filled)
done

# Fill the unused slots with 0.
for j in {$i..15}
do
    out_files+=(zero.bin)
done

echo writing \`./bin/software_rom.bin\`...
cat $out_files > bin/software_rom.bin
