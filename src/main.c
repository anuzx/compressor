#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// structure
typedef struct Node {
  unsigned char byte;
  uint64_t frequency;

  struct Node *left;
  struct Node *right;
} Node;

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

int main(int argc, char *argv[]) {

  // to ensure the user provide exactly one filename
  if (argc != 2) {
    printf("Usage: compressor <filname>\n");
    return 1;
  }

  FILE *file = fopen(argv[1], "rb");

  if (file == NULL) {
    perror("Could not open file");
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
    perror("Error while reading file");
    fclose(file);
    return 1;
  }

  // forest as it contains multiple independent trees
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
        free(forest[j]);
      }

      fclose(file);
      return 1;
    }

    forest[node_count] = node;
    node_count++;
  }

  fclose(file);

  printf("Total bytes: %llu\n", (unsigned long long)total_bytes);
  printf("Unique bytes: %d\n", node_count);

  for (int i = 0; i < node_count; i++) {
    printf("Byte: %d | Frequency: %llu\n", forest[i]->byte,
           (unsigned long long)forest[i]->frequency);
  }

  for (int i = 0; i < node_count; i++) {
    free(forest[i]);
  }

  return 0;
}
