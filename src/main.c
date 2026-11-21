#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// step 1 : read a file and count byte frequencies

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

  fclose(file);

  printf("Total bytes: %llu\n", (unsigned long long)total_bytes);
  printf("\nByte frequencies:\n");

  for (int i = 0; i < 256; i++) {
    if (frequency[i] > 0) {
      printf("Byte: %3d | Frequency: %llu\n", i,
             (unsigned long long)frequency[i]);
    }
  }

  return 0;
}
