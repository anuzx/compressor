#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "huffman.h"


int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
    return 1;
  }

  // Do not allow the exact same path for input and output.
  if (strcmp(argv[1], argv[2]) == 0) {
    fprintf(stderr, "Input and output paths must be different.\n");
    return 1;
  }

  // First pass: count the frequency of every possible byte.
  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    perror("Could not open input file");
    return 1;
  }

  uint64_t frequency[256] = {0};
  uint64_t total_bytes = 0;
  int byte;

  while ((byte = fgetc(file)) != EOF) {
    unsigned char value = (unsigned char)byte;
    frequency[value]++;
    total_bytes++;
  }

  if (ferror(file)) {
    perror("Error while reading input file");
    fclose(file);
    return 1;
  }

  if (fclose(file) == EOF) {
    perror("Could not close input file");
    return 1;
  }

  printf("Original size: %llu bytes\n", (unsigned long long)total_bytes);

  // Create the initial forest. Each present byte gets one leaf node.
  Node *forest[256] = {NULL};
  int node_count = 0;

  for (int i = 0; i < 256; i++) {
    if (frequency[i] == 0) {
      continue;
    }

    Node *node = create_node((unsigned char)i, frequency[i]);
    if (node == NULL) {
      fprintf(stderr, "Memory allocation failed.\n");
      for (int j = 0; j < node_count; j++) {
        free_tree(forest[j]);
      }
      return 1;
    }

    forest[node_count++] = node;
  }

  printf("Unique bytes: %d\n", node_count);

  // An empty input has no tree or payload. Still create the requested output.
  if (node_count == 0) {
    FILE *output = fopen(argv[2], "wb");
    if (output == NULL) {
      perror("Could not create output file");
      return 1;
    }

    if (fclose(output) == EOF) {
      perror("Could not close output file");
      return 1;
    }

    printf("Input is empty. Created an empty output file: %s\n", argv[2]);
    return 0;
  }

  // Build the Huffman tree.
  Node *root = build_huffman_tree(forest, &node_count);
  if (root == NULL) {
    fprintf(stderr, "Failed to build Huffman tree.\n");
    // If tree construction failed, forest still contains the independent roots.
    for (int i = 0; i < node_count; i++) {
      free_tree(forest[i]);
    }
    return 1;
  }

  printf("\nHuffman tree:\n");
  print_tree(root, 0);

  // Generate the code table from the tree.
  HuffmanCode codes[256] = {0};
  char path[256];
  generate_codes(root, path, 0, codes);

  printf("\nHuffman codes:\n");
  for (int i = 0; i < 256; i++) {
    if (codes[i].length == 0) {
      continue;
    }

    printf("Byte: %d | Code: %s | Length: %u\n", i, codes[i].bits,
           codes[i].length);
  }

  // Second pass: reopen the input and encode its bytes.
  FILE *input = fopen(argv[1], "rb");
  if (input == NULL) {
    perror("Could not reopen input file");
    free_tree(root);
    return 1;
  }

  FILE *output = fopen(argv[2], "wb");
  if (output == NULL) {
    perror("Could not create output file");
    fclose(input);
    free_tree(root);
    return 1;
  }

  BitWriter writer = {.output = output, .buffer = 0, .bit_count = 0};

  int status = encode_file(input, &writer, codes);
  if (status == 0) {
    status = flush_bits(&writer);
  }

  if (fclose(input) == EOF) {
    perror("Could not close input file");
    status = -1;
  }

  // fclose also reports buffered output errors.
  if (fclose(output) == EOF) {
    perror("Could not finish writing output file");
    status = -1;
  }

  free_tree(root);

  if (status == -1) {
    fprintf(stderr, "Compression failed. The output file may be incomplete.\n");
    return 1;
  }

  printf("\nCompression completed successfully.\n");
  printf("Output file: %s\n", argv[2]);
  return 0;
}