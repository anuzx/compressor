CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -O2
CPPFLAGS := -Iinclude

SRC := src/main.c src/huffman.c
HDR := include/huffman.h

.PHONY: all run clean

all: build/compressor compile_commands.json

build:
	mkdir -p build

build/compressor: $(SRC) $(HDR) | build
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SRC) -o $@

compile_commands.json: Makefile $(SRC) $(HDR)
	@{ \
		echo '['; \
		sep=; \
		for f in $(SRC); do \
			if [ -n "$$sep" ]; then echo ','; fi; \
			printf '  {"directory": "%s", "file": "%s", "command": "%s %s %s -c %s"}' \
				"$(CURDIR)" "$$f" "$(CC)" "$(CFLAGS)" "$(CPPFLAGS)" "$$f"; \
			sep=1; \
		done; \
		echo; \
		echo ']'; \
	} > $@

run: all
	./build/compressor public/sample.txt

clean:
	rm -rf build compile_commands.json
