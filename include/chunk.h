#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "line.h"
#include "value.h"

#define CHUNK_INITIAL_CAPACITY 8

struct Chunk {
  // dynamic array
  size_t byte_count;
  size_t byte_capacity;
  uint8_t *buffer;

  struct LineArray lines; // compressed line representation for bytecode in buffer
  struct ValueArray constants; // constant pool
};

void chunk_init(struct Chunk *self);
void chunk_free(struct Chunk *self);
void chunk_write(struct Chunk *self, const uint8_t byte, const size_t line);
size_t chunk_add_constant(struct Chunk *self, const Value constant);
size_t chunk_write_constant(struct Chunk *self, const Value constant, const size_t line);
size_t chunk_get_line(struct Chunk *const self, const size_t offset);

#endif // CHUNK_H