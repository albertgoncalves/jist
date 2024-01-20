MAKEFLAGS += --silent
CC = mold -run clang
CFLAGS = \
    -D_DEFAULT_SOURCE \
    -ferror-limit=1 \
    -fsanitize=address \
    -fsanitize=bounds \
    -fsanitize=float-divide-by-zero \
    -fsanitize=implicit-conversion \
    -fsanitize=integer \
    -fsanitize=nullability \
    -fsanitize=undefined \
    -fshort-enums \
    -march=native \
    -O3 \
    -std=c99 \
    -Werror \
    -Weverything \
    -Wno-covered-switch-default \
    -Wno-declaration-after-statement \
    -Wno-disabled-macro-expansion \
    -Wno-padded \
    -Wno-unsafe-buffer-usage
MODULES = \
	prelude \
	inst \
	expr \
	asm
OBJECTS = $(foreach x,$(MODULES),build/$(x).o)

.PHONY: all
all: bin/main

.PHONY: clean
clean:
	rm -rf bin/
	rm -rf build/

.PHONY: run
run: all
	./bin/main

bin/main: $(OBJECTS) src/main.c
	mkdir -p bin/
	clang-format -i src/main.c
	$(CC) $(CFLAGS) -o bin/main $(OBJECTS) src/main.c

$(OBJECTS): build/%.o: src/%.h src/%.c
	mkdir -p build/
	clang-format -i $^
	$(CC) $(CFLAGS) -c -o $@ $(word 2,$^)
