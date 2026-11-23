#include "huffman.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Write a 16-bit integer in little-endian byte order. */
static int write_u16_le(FILE *file, uint16_t value) {
  for (unsigned int i = 0; i < 2; i++) {
    unsigned char byte = (unsigned char)(value >> (i * 8));
    if (fputc(byte, file) == EOF) {
      return -1;
    }
  }

  return 0;
}

/* Write a 64-bit integer in little-endian byte order. */
static int write_u64_le(FILE *file, uint64_t value) {
  for (unsigned int i = 0; i < 8; i++) {
    unsigned char byte = (unsigned char)(value >> (i * 8));
    if (fputc(byte, file) == EOF) {
      return -1;
    }
  }

  return 0;
}

/*
 * File format:
 *   4 bytes: magic signature "HF01"
 *   8 bytes: original file size (little-endian)
 *   2 bytes: number of unique symbols (little-endian)
 *   For each symbol:
 *     1 byte: symbol value
 *     8 bytes: symbol frequency (little-endian)
 *   Remaining bytes: Huffman-encoded payload
 */
static int write_header(FILE *output, const uint64_t frequency[256],
                        uint64_t original_size, uint16_t symbol_count) {
  if (fwrite("HF01", 1, 4, output) != 4) {
    return -1;
  }

  if (write_u64_le(output, original_size) == -1) {
    return -1;
  }

  if (write_u16_le(output, symbol_count) == -1) {
    return -1;
  }

  for (int i = 0; i < 256; i++) {
    if (frequency[i] == 0) {
      continue;
    }

    if (fputc(i, output) == EOF) {
      return -1;
    }

    if (write_u64_le(output, frequency[i]) == -1) {
      return -1;
    }
  }

  return 0;
}

// main function
int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input> <output.huff>\n", argv[0]);
    return 1;
  }

  if (strcmp(argv[1], argv[2]) == 0) {
    fprintf(stderr, "Input and output paths must be different.\n");
    return 1;
  }

  /* First pass: count the frequency of every possible byte. */
  FILE *input = fopen(argv[1], "rb");
  if (input == NULL) {
    perror("Could not open input file");
    return 1;
  }

  uint64_t frequency[256] = {0};
  uint64_t original_size = 0;
  int byte;

  while ((byte = fgetc(input)) != EOF) {
    frequency[(unsigned char)byte]++;
    original_size++;
  }

  if (ferror(input)) {
    perror("Could not read input file");
    fclose(input);
    return 1;
  }

  if (fclose(input) == EOF) {
    perror("Could not close input file");
    return 1;
  }

  /* Build the initial forest from bytes that occur in the input. */
  uint16_t symbol_count = 0;
  Node *forest[256] = {0};
  int node_count = 0;

  for (int i = 0; i < 256; i++) {
    if (frequency[i] == 0) {
      continue;
    }

    Node *leaf = create_node((unsigned char)i, frequency[i]);
    if (leaf == NULL) {
      fprintf(stderr, "Memory allocation failed.\n");
      for (int j = 0; j < node_count; j++) {
        free_tree(forest[j]);
      }
      return 1;
    }

    forest[node_count++] = leaf;
    symbol_count++;
  }

  /* Build the Huffman tree. An empty input has no tree. */
  Node *root = NULL;
  if (node_count > 0) {
    root = build_huffman_tree(forest, &node_count);
    if (root == NULL) {
      fprintf(stderr, "Could not build Huffman tree.\n");
      for (int i = 0; i < node_count; i++) {
        free_tree(forest[i]);
      }
      return 1;
    }
  }

  /* Generate the Huffman code for each byte. */
  HuffmanCode codes[256] = {0};
  if (root != NULL) {
    char path[256];
    generate_codes(root, path, 0, codes);
  }

  /* Create the compressed output and write its metadata. */
  FILE *output = fopen(argv[2], "wb");
  if (output == NULL) {
    perror("Could not create output file");
    free_tree(root);
    return 1;
  }

  int status = write_header(output, frequency, original_size, symbol_count);

  /* Second pass: encode the original bytes and pack the bits. */
  if (status == 0 && original_size > 0) {
    input = fopen(argv[1], "rb");
    if (input == NULL) {
      perror("Could not reopen input file");
      status = -1;
    } else {
      BitWriter writer = {
          .output = output,
          .buffer = 0,
          .bit_count = 0,
      };

      status = encode_file(input, &writer, codes);

      if (status == 0) {
        status = flush_bits(&writer);
      }

      if (fclose(input) == EOF) {
        perror("Could not close input file after encoding");
        status = -1;
      }
    }
  }

  /* Check output errors before releasing resources. */
  if (fclose(output) == EOF) {
    perror("Could not close output file");
    status = -1;
  }

  free_tree(root);

  if (status != 0) {
    fprintf(stderr, "Compression failed. Output may be incomplete.\n");
    return 1;
  }

  printf("Compression completed successfully.\n");
  printf("Original size: %llu bytes\n", (unsigned long long)original_size);
  printf("Unique symbols: %u\n", (unsigned int)symbol_count);
  printf("Output file: %s\n", argv[2]);

  return 0;
}
