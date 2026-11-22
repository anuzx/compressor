#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "huffman.h"

int main(int argc, char *argv[]) {

  // ensure exactly one filename is provided.
  if (argc != 2) {
    printf("Usage: compressor <filename>\n");
    return 1;
  }

  // open the input file in binary mode.
  FILE *file = fopen(argv[1], "rb");

  if (file == NULL) {
    perror("Could not open file");
    return 1;
  }

  // count byte frequencies.
  uint64_t frequency[256] = {0};
  uint64_t total_bytes = 0;

  int byte;

  while ((byte = fgetc(file)) != EOF) {

    unsigned char value = (unsigned char)byte;

    frequency[value]++;
    total_bytes++;
  }

  if (ferror(file)) {
    perror("Error while reading file");
    fclose(file);
    return 1;
  }

  fclose(file);

  printf("Total bytes: %llu\n", (unsigned long long)total_bytes);

  // create the initial forest.
  Node *forest[256] = {NULL};
  int node_count = 0;

  for (int i = 0; i < 256; i++) {

    if (frequency[i] == 0) {
      continue;
    }

    Node *node = create_node((unsigned char)i, frequency[i]);

    if (node == NULL) {
      printf("Memory allocation failed\n");

      for (int j = 0; j < node_count; j++) {
        free_tree(forest[j]);
      }

      return 1;
    }

    forest[node_count++] = node;
  }

  printf("Unique bytes: %d\n", node_count);

  // handle an empty file.
  if (node_count == 0) {
    printf("Empty file. No Huffman tree required.\n");
    return 0;
  }

  // build the Huffman tree.
  Node *root = build_huffman_tree(forest, &node_count);

  if (root == NULL) {
    printf("Failed to build Huffman tree\n");

    // free all remaining independent tree roots.
    for (int i = 0; i < node_count; i++) {
      free_tree(forest[i]);
    }

    return 1;
  }

  printf("\nHuffman Tree:\n");

  print_tree(root, 0);

  // free the complete tree.
  free_tree(root);

  return 0;
}
