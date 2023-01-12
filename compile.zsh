#!/bin/zsh

WARNINGS=()
# https://clang.llvm.org/docs/DiagnosticsReference.html#wall
WARNINGS+=(-Wall)
# https://clang.llvm.org/docs/DiagnosticsReference.html#wpedantic
WARNINGS+=(-Wpedantic)
# https://clang.llvm.org/docs/DiagnosticsReference.html#wextra
WARNINGS+=(-Wextra)
# https://clang.llvm.org/docs/DiagnosticsReference.html#wconversion
WARNINGS+=(-Wconversion)
# https://clang.llvm.org/docs/DiagnosticsReference.html#wassign-enum
WARNINGS+=(-Wassign-enum)
# https://clang.llvm.org/docs/DiagnosticsReference.html#wshadow-all
WARNINGS+=(-Wshadow-all)

DISABLED_WARNINGS=()
# DISABLED_WARNINGS+=(-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function)

OPTIMIZATION_LEVEL=-O1

CLANG_SANITIZE=()
# https://clang.llvm.org/docs/AddressSanitizer.html#usage
CLANG_SANITIZE+=(-fsanitize=address -fno-omit-frame-pointer)
# https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html#usage
CLANG_SANITIZE+=(-fsanitize=undefined,integer,nullability -fno-sanitize-recover)

set -euo pipefail

mkdir -p ./bin

set -x

clang $CLANG_SANITIZE \
    $WARNINGS $DISABLED_WARNINGS -Werror -ferror-limit=256 \
    $OPTIMIZATION_LEVEL -std=c17 \
    --debug \
    -o ./bin/"${1%.*}" "$1"
