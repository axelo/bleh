#!/bin/zsh

# -e exit immediately when a command fails.
# -u treat unset variables as an error and exit immediately.
# -x print each command before executing it.
# -o pipefail sets the exit code of a pipeline to that of the rightmost command to exit with a non-zero status.
set -euo pipefail

mkdir -p ./bin/software

# Compile every asm file individually, keep track of the outputted bin filenames.
out_files=()

declare -i i=0

for FILE in software/*.asm
do
  out_file=./bin/software/${${FILE:t}:r}
  out_file_filled=${out_file}_filled.bin
  out_file_actual=${out_file}_actual.bin

  customasm -q customasm_fill_rom.asm $FILE -o $out_file_filled
  customasm -q customasm_rom.asm $FILE -o $out_file_actual
  echo "$i: $FILE"
  i+=1
  customasm -q customasm_rom.asm $FILE -p
  out_files+=($out_file_filled)
done

# Fill the unused slots with 0.
for j in {$i..15}
do
    out_files+=(zero.bin)
done

echo writing \`./bin/software.bin\`...
cat $out_files > bin/software.bin
