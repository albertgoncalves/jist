#!/usr/bin/env bash

set -eu

if [ ! -d "$WD/bin" ]; then
    mkdir "$WD/bin"
fi

flags=(
    "-ferror-limit=1"
    "-fsanitize=address"
    "-fsanitize=bounds"
    "-fsanitize=float-divide-by-zero"
    "-fsanitize=implicit-conversion"
    "-fsanitize=integer"
    "-fsanitize=nullability"
    "-fsanitize=undefined"
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

clang "${flags[@]}" -c -o "$WD/bin/prelude.o" "$WD/src/prelude.c" &
clang "${flags[@]}" -c -o "$WD/bin/inst.o"    "$WD/src/inst.c"    &
clang "${flags[@]}" -c -o "$WD/bin/expr.o"    "$WD/src/expr.c"    &
clang "${flags[@]}" -c -o "$WD/bin/asm.o"     "$WD/src/asm.c"     &

for _ in $(jobs -p); do
    wait -n
done

clang "${flags[@]}" -o "$WD/bin/main" "$WD/bin/prelude.o" "$WD/bin/inst.o" \
    "$WD/bin/expr.o" "$WD/bin/asm.o" "$WD/src/main.c"

"$WD/bin/main"
