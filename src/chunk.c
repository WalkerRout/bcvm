
#include <stdio.h>

#include "chunk.h"
#include "opcode.h"
#include "memory.h"

inline void chunk_init(struct Chunk *self) {
  self->byte_count = 0;
  self->byte_capacity   = 0;
  self->buffer = NULL;

  line_array_init(&self->lines);
  value_array_init(&self->constants);
}

inline void chunk_free(struct Chunk *self) {
  value_array_free(&self->constants);
  line_array_free(&self->lines);

  MEMORY_FREE_ARRAY(uint8_t, self->buffer, self->byte_capacity);

  chunk_init(self);
}

void chunk_write(struct Chunk *self, const uint8_t byte, const size_t line) {
  size_t initial_byte_count = self->byte_count;
  size_t initial_byte_capacity = self->byte_capacity;

  // resize the byte backing buffer
  if(initial_byte_capacity < initial_byte_count + 1) {
    self->byte_capacity = MEMORY_GROW_CAPACITY(initial_byte_capacity, CHUNK_INITIAL_CAPACITY);
    self->buffer = MEMORY_GROW_ARRAY(uint8_t, self->buffer, initial_byte_capacity, self->byte_capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  self->buffer[self->byte_count] = byte;
  self->byte_count += 1;

  line_array_write(&self->lines, line);
}

size_t chunk_add_constant(struct Chunk *self, const Value constant) {
  value_array_write(&self->constants, constant);
  // return index where the constant was inserted for future access
  return self->constants.value_count - 1;
}

size_t chunk_write_constant(struct Chunk *self, const Value constant, const size_t line) {
  size_t value_index = chunk_add_constant(self, constant);

  if (self->constants.value_count >= 0xFF) {
    chunk_write(self, OPCODE_CONSTANT_LONG, line);
    chunk_write(self, (value_index >> 16) & 0xFF, line);
    chunk_write(self, (value_index >> 8)  & 0xFF, line);
  } else {
    chunk_write(self, OPCODE_CONSTANT, line);
  }

  chunk_write(self, value_index & 0xFF, line);

  return self->constants.value_count - 1;
}

size_t chunk_get_line(struct Chunk *const self, const size_t offset) {
  size_t line_struct_count = self->lines.line_struct_count;

  size_t line = 0;
  size_t prev_line_count = 0;
  
  for(size_t i = 0; i < line_struct_count; ++i) {
    size_t curr_line_count = self->lines.lines[i].line_count + prev_line_count;

    if(prev_line_count <= offset && offset < curr_line_count) {
      line = self->lines.lines[i].line;
      break;
    }

    prev_line_count = curr_line_count;
  }

  return line;
}
