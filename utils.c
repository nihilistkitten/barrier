#include <stdio.h>
#include <stdlib.h>

void *alloc(size_t num, size_t size) {
  void *out = calloc(num, size);

  if (!out) {
    fprintf(stderr, "Memory allocation failed!\n");
    exit(1);
  }

  return out;
}
