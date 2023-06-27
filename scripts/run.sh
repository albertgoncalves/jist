#!/usr/bin/env bash

set -eu

if [ ! -d "$WD/bin" ]; then
    mkdir "$WD/bin"
fi

flags=(
    "-ferror-limit=1"
    -fshort-enums
    "-march=native"
    -O3
    "-std=c99"
    -Werror
    -Weverything
    -Wno-covered-switch-default
    -Wno-declaration-after-statement
    -Wno-padded
)

clang-format -i -verbose "$WD/src/"*
clang "${flags[@]}" -o "$WD/bin/main" "$WD/src/main.c"
"$WD/bin/main"
