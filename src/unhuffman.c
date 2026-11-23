#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "huffman.h"

static int read_u16_le(FILE *file, uint16_t *value) {
  uint16_t result = 0;
  for (unsigned int i = 0; i < 2; i++) {
    int byte = fgetc(file);
    if (byte == EOF)
      return -1;
    result |= (uint16_t)((uint16_t)(unsigned char)byte << (i * 8));
  }
  *value = result;
  return 0;
}

static int read_u64_le(FILE *file, uint64_t *value) {
  uint64_t result = 0;
  for (unsigned int i = 0; i < 8; i++) {
    int byte = fgetc(file);
    if (byte == EOF)
      return -1;
    result |= (uint64_t)(unsigned char)byte << (i * 8);
  }
  *value = result;
  return 0;
}

static int read_header(FILE *input, uint64_t *original_size,
                       uint64_t frequency[256], uint16_t *symbol_count) {
  unsigned char magic[4];
  if (fread(magic, 1, 4, input) != 4 || memcmp(magic, "HF01", 4) != 0) {
    fprintf(stderr, "Invalid file: missing HF01 signature.\n");
    return -1;
  }
  if (read_u64_le(input, original_size) == -1 ||
      read_u16_le(input, symbol_count) == -1) {
    fprintf(stderr, "Invalid or truncated file header.\n");
    return -1;
  }
  if (*symbol_count > 256 || ((*original_size == 0) != (*symbol_count == 0))) {
    fprintf(stderr, "Invalid header values.\n");
    return -1;
  }

  uint64_t total = 0;
  for (uint16_t i = 0; i < *symbol_count; i++) {
    int symbol = fgetc(input);
    uint64_t count;
    if (symbol == EOF || read_u64_le(input, &count) == -1 || count == 0) {
      fprintf(stderr, "Invalid or truncated frequency table.\n");
      return -1;
    }
    if (frequency[(unsigned char)symbol] != 0) {
      fprintf(stderr, "Duplicate symbol in frequency table.\n");
      return -1;
    }
    frequency[(unsigned char)symbol] = count;
    if (UINT64_MAX - total < count) {
      fprintf(stderr, "Frequency total overflow.\n");
      return -1;
    }
    total += count;
  }

  if (total != *original_size) {
    fprintf(stderr, "Frequency table does not match original size.\n");
    return -1;
  }
  return 0;
}

static int decode_payload(FILE *input, FILE *output, Node *root,
                          uint64_t original_size) {
  if (original_size == 0)
    return 0;
  if (root == NULL)
    return -1;

  /* A one-symbol tree has a one-bit code: 0. */
  if (root->left == NULL && root->right == NULL) {
    unsigned char buffer = 0;
    unsigned int bits_left = 0;
    for (uint64_t i = 0; i < original_size; i++) {
      if (bits_left == 0) {
        int next = fgetc(input);
        if (next == EOF)
          return -1;
        buffer = (unsigned char)next;
        bits_left = 8;
      }
      int bit = (buffer >> 7) & 1;
      buffer <<= 1;
      bits_left--;
      if (bit != 0 || fputc(root->byte, output) == EOF)
        return -1;
    }
    return 0;
  }

  Node *current_node = root;
  uint64_t written = 0;
  unsigned char buffer = 0;
  unsigned int bits_left = 0;

  while (written < original_size) {
    if (bits_left == 0) {
      int next = fgetc(input);
      if (next == EOF) {
        fprintf(stderr,
                "Compressed payload ended before decoding completed.\n");
        return -1;
      }
      buffer = (unsigned char)next;
      bits_left = 8;
    }

    int bit = (buffer >> 7) & 1;
    buffer <<= 1;
    bits_left--;

    current_node = bit == 0 ? current_node->left : current_node->right;
    if (current_node == NULL) {
      fprintf(stderr, "Invalid Huffman bit sequence.\n");
      return -1;
    }

    if (current_node->left == NULL && current_node->right == NULL) {
      if (fputc(current_node->byte, output) == EOF)
        return -1;
      written++;
      current_node = root;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input.huff> <recovered-output>\n", argv[0]);
    return 1;
  }
  if (strcmp(argv[1], argv[2]) == 0) {
    fprintf(stderr, "Input and output paths must be different.\n");
    return 1;
  }

  FILE *input = fopen(argv[1], "rb");
  if (input == NULL) {
    perror("Could not open compressed file");
    return 1;
  }

  uint64_t original_size = 0;
  uint64_t frequency[256] = {0};
  uint16_t symbol_count = 0;
  if (read_header(input, &original_size, frequency, &symbol_count) == -1) {
    fclose(input);
    return 1;
  }

  Node *forest[256] = {0};
  int node_count = 0;
  for (int i = 0; i < 256; i++) {
    if (frequency[i] == 0)
      continue;
    Node *leaf = create_node((unsigned char)i, frequency[i]);
    if (leaf == NULL) {
      fprintf(stderr, "Memory allocation failed.\n");
      for (int j = 0; j < node_count; j++)
        free_tree(forest[j]);
      fclose(input);
      return 1;
    }
    forest[node_count++] = leaf;
  }

  Node *root = NULL;
  if (node_count > 0) {
    root = build_huffman_tree(forest, &node_count);
    if (root == NULL) {
      fprintf(stderr, "Could not rebuild Huffman tree.\n");
      for (int i = 0; i < node_count; i++)
        free_tree(forest[i]);
      fclose(input);
      return 1;
    }
  }

  FILE *output = fopen(argv[2], "wb");
  if (output == NULL) {
    perror("Could not create recovered file");
    free_tree(root);
    fclose(input);
    return 1;
  }

  int status = decode_payload(input, output, root, original_size);
  if (fclose(input) == EOF)
    status = -1;
  if (fclose(output) == EOF)
    status = -1;
  free_tree(root);

  if (status != 0) {
    fprintf(stderr,
            "Decompression failed. Recovered output may be incomplete.\n");
    return 1;
  }

  printf("Decompression completed. Recovered %llu bytes to %s\n",
         (unsigned long long)original_size, argv[2]);
  return 0;
}
