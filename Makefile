.PHONY: all run decompress clean

all: build/compressor build/unhuffman

build:
	mkdir -p build

build/compressor: src/main.c src/huffman.c include/huffman.h | build
	gcc -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude src/main.c src/huffman.c -o build/compressor

build/unhuffman: src/unhuffman.c src/huffman.c include/huffman.h | build
	gcc -Wall -Wextra -Wpedantic -std=c11 -O2 -Iinclude src/unhuffman.c src/huffman.c -o build/unhuffman

run: all
	./build/compressor public/sample.txt public/sample.huff

decompress: all
	./build/unhuffman public/sample.huff public/recovered.txt

clean:
	rm -rf build
	rm -f public/sample.huff public/recovered.txt
