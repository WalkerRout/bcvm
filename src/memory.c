
#include <stdio.h>
#include <stdlib.h>

#include "memory.h"

void *memory_reallocate(void *buffer, size_t old_size, size_t new_size) {
  (void) old_size;

  if(new_size == 0) {
    free(buffer);
    return NULL;
  }

  void *result = realloc(buffer, new_size);
  
  if(result == NULL) {
    fprintf(stderr, "Error - result in memory_reallocate is NULL");
    exit(1);
  }

  return result;
}