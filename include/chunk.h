#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"

#define CHUNK_INITIAL_CAPACITY 8

struct Chunk {
  // dynamic array
  size_t byte_count;
  size_t capacity;
  uint8_t *buffer;
  size_t *lines;
  // constant pool
  struct ValueArray constants;
};

void chunk_init(struct Chunk *self);
void chunk_free(struct Chunk *self);
void chunk_write(struct Chunk *self, uint8_t byte, size_t line);
size_t chunk_add_constant(struct Chunk *self, Value constant);

#endif // CHUNK_H