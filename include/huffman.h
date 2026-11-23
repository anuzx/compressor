#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stdio.h>
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

// bit writer structure
typedef struct {
  FILE *output;
  unsigned char buffer;
  unsigned int bit_count;
} BitWriter;

// function declarations
Node *create_node(unsigned char byte, uint64_t frequency);

Node *build_huffman_tree(Node **forest, int *node_count);

void print_tree(Node *node, int depth);

void free_tree(Node *node);

void generate_codes(Node *node, char path[256], unsigned int depth,
                    HuffmanCode codes[256]);

int write_bit(BitWriter *writer, int bit);
int flush_bits(BitWriter *writer);

int encode_file(FILE *input, BitWriter *writer, HuffmanCode codes[256]);

#endif
