.PHONY: all run clean

all:
	mkdir -p build
	gcc -Wall -Wextra -std=c11 src/main.c -o build/compressor

run: all
	./build/compressor public/sample.txt

clean:
	rm -rf build

