#!/bin/bash

REQ_FILES="../src/moddef.c"
INC_FLAGS="-I ./ -I ../src"
MOD=$1
shift
FLAGS=""
while [[ $# -ne 0 ]]; do
    FLAGS="$FLAGS $1"
    shift
done

gcc -fPIC -shared -o "$MOD.so" "$MOD.c" $REQ_FILES $INC_FLAGS $FLAGS
x86_64-w64-mingw32-gcc -shared -o "$MOD.dll" "$MOD.c" $REQ_FILES $INC_FLAGS $FLAGS
