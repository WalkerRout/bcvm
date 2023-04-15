#ifndef CHUNK_H
#define CHUNK_H

#include "common.h"
#include "value.h"

#define CHUNK_INITIAL_CAPACITY 8
#define CHUNK_LINE_INITIAL_CAPACITY 4

struct Line {
  size_t line;
  size_t line_count;
};

struct Chunk {
  // dynamic array
  size_t byte_count;
  size_t byte_capacity;
  uint8_t *buffer;

  // lines
  size_t line_struct_count;
  size_t line_struct_capacity;
  struct Line *lines;

  // constant pool
  struct ValueArray constants;
};

void chunk_init(struct Chunk *self);
void chunk_free(struct Chunk *self);
void chunk_write(struct Chunk *self, const uint8_t byte, const size_t line);
size_t chunk_add_constant(struct Chunk *self, const Value constant);
size_t chunk_get_line(struct Chunk *const self, const size_t offset);

#endif // CHUNK_H