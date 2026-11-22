#include <stdio.h>
#include <stdlib.h>

#include "huffman.h"

// Create a new node
Node *create_node(unsigned char byte, uint64_t frequency) {

  Node *node = malloc(sizeof(Node));

  if (node == NULL) {
    return NULL;
  }

  node->byte = byte;
  node->frequency = frequency;

  node->left = NULL;
  node->right = NULL;

  return node;
}

// Build the Huffman tree
Node *build_huffman_tree(Node **forest, int *node_count) {

  while (*node_count > 1) {

    int min1 = -1;
    int min2 = -1;

    // Find the two nodes with the lowest frequencies.
    for (int i = 0; i < *node_count; i++) {

      if (min1 == -1 || forest[i]->frequency < forest[min1]->frequency) {

        min2 = min1;
        min1 = i;

      } else if (min2 == -1 || forest[i]->frequency < forest[min2]->frequency) {

        min2 = i;
      }
    }

    // Keep the smaller index in min1.
    if (min1 > min2) {
      int temp = min1;
      min1 = min2;
      min2 = temp;
    }

    // Get the two minimum nodes.
    Node *left = forest[min1];
    Node *right = forest[min2];

    // Create their parent.
    Node *parent = create_node(0, left->frequency + right->frequency);

    if (parent == NULL) {
      return NULL;
    }

    // Connect the children.
    parent->left = left;
    parent->right = right;

    // Remove the second node.
    forest[min2] = forest[*node_count - 1];
    (*node_count)--;

    // Remove the first node.
    forest[min1] = forest[*node_count - 1];
    (*node_count)--;

    // Insert the parent.
    forest[*node_count] = parent;
    (*node_count)++;
  }

  // Return the root.
  return *node_count == 1 ? forest[0] : NULL;
}

// Print the Huffman tree
void print_tree(Node *node, int depth) {

  if (node == NULL) {
    return;
  }

  printf("%*s", depth * 2, "");

  if (node->left == NULL && node->right == NULL) {

    printf("Leaf: byte = %d, frequency = %llu\n", node->byte,
           (unsigned long long)node->frequency);

  } else {

    printf("Internal: frequency = %llu\n", (unsigned long long)node->frequency);
  }

  // Depth-first traversal.
  print_tree(node->left, depth + 1);
  print_tree(node->right, depth + 1);
}

// Free the entire tree
void free_tree(Node *node) {

  if (node == NULL) {
    return;
  }

  free_tree(node->left);
  free_tree(node->right);

  free(node);
}

// recursive fn to generate code
void generate_codes(
    Node *node, 
    char path[256], 
    unsigned int depth,
    HuffmanCode codes[256]
) {

  if (node == NULL) {
    return;
  }

  // check if this is a leaf

  if (node->left == NULL && node->right == NULL) {

    if (depth == 0) {
      path[0] = '0';
      depth = 1;
    }

    path[depth] = '\0';

    // save this code for the byte
    for (unsigned int i = 0; i < depth; i++) {
      codes[node->byte].bits[i] = path[i];
    }

    codes[node->byte].bits[depth] = '\0';
    codes[node->byte].length = depth;

    return;
  }

  // left means 0
  path[depth] = '0';
  generate_codes(node->left, path, depth + 1, codes);

  // right means 1
  path[depth] = '1';
  generate_codes(node->right, path, depth + 1, codes);
}
