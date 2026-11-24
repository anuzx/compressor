# File Compressor

A lossless file compression utility implemented in **C** using the Huffman coding algorithm. This project explores data compression, binary file handling, bit manipulation, and memory management.

## Huffman Coding

Huffman coding is a lossless data compression algorithm that assigns variable-length binary codes to symbols based on their frequency.

More frequent symbols receive shorter codes, while less frequent symbols receive longer codes. This reduces the average number of bits required to represent the input data.

### How It Works

The algorithm constructs a binary prefix tree, known as a **Huffman Tree**, using a greedy approach:

1. **Frequency Analysis:** Count the frequency of each byte in the input file.
2. **Node Creation:** Create a leaf node for every byte with a non-zero frequency.
3. **Tree Construction:** Repeatedly select the two nodes with the lowest frequencies and combine them into a new internal node. The frequency of the new node is the sum of its children's frequencies.
4. **Code Generation:** Traverse the completed tree. Assign `0` to left edges and `1` to right edges. The path from the root to each leaf represents that byte's Huffman code.
5. **Encoding:** Replace each input byte with its corresponding variable-length code.
6. **Bit Packing:** Pack the generated bits into 8-bit buffers before writing them to the compressed file.

The resulting codes are prefix-free, which means no code is a prefix of another. This allows the encoded data to be decoded unambiguously.

## Features

- Lossless file compression and decompression.
- Frequency-based Huffman tree construction.
- Variable-length prefix-free binary encoding.
- Bit-level packing using bitwise operations.
- Custom binary file format with compression metadata.
- Binary-safe file processing.
- Round-trip verification using `cmp` and `sha256sum`.

## Build and Usage

### Requirements

- GCC
- GNU Make
- Linux or WSL

### Build

```bash
make
```

### Compress a file

```bash
make run
```

This compresses `public/sample.txt` and generates `public/sample.huff`.

### Decompress a file

```bash
make decompress
```

This generates `public/recovered.txt`.

### Verify lossless compression

```bash
cmp public/sample.txt public/recovered.txt
```

If the files are identical, `cmp` produces no output.

You can also compare their SHA-256 hashes:

```bash
sha256sum public/sample.txt public/recovered.txt
```

### Compare file sizes

```bash
stat -c '%n: %s bytes' public/sample.txt public/sample.huff
```

### Clean build files

```bash
make clean
```
