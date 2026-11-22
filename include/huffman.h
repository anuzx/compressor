#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>

// huffman tree node
typedef struct Node {
  unsigned char byte;
  uint64_t frequency;

  struct Node *left;
  struct Node *right;
} Node;

// structure to store codes
typedef struct {
  char bits[256];
  unsigned int length;
} HuffmanCode;

// function declarations
Node *create_node(unsigned char byte, uint64_t frequency);

Node *build_huffman_tree(Node **forest, int *node_count);

void print_tree(Node *node, int depth);

void free_tree(Node *node);

void generate_codes(Node *node, char path[256], unsigned int depth,
                    HuffmanCode codes[256]);

#endif
