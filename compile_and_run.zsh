#!/bin/zsh

./compile.zsh $1 && ./bin/"${1%.*}" ${@:2}
