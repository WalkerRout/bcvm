
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

inline void chunk_init(struct Chunk *self) {
  self->byte_count = 0;
  self->byte_capacity   = 0;
  self->buffer = NULL;

  self->line_struct_count = 0;
  self->line_struct_capacity = 0;
  self->lines  = NULL;

  value_array_init(&self->constants);
}

inline void chunk_free(struct Chunk *self) {
  value_array_free(&self->constants);
  MEMORY_FREE_ARRAY(uint8_t, self->buffer, self->byte_capacity);
  MEMORY_FREE_ARRAY(sizeof(struct Line), self->lines, self->line_struct_capacity);
}

void chunk_write(struct Chunk *self, const uint8_t byte, const size_t line) {
  size_t initial_line_struct_count = self->line_struct_count;
  size_t initial_line_struct_capacity = self->line_struct_capacity;

  size_t initial_byte_count = self->byte_count;
  size_t initial_byte_capacity = self->byte_capacity;

  size_t previous_line = 0;
  // if a line has already been initialized...
  if(initial_line_struct_count > 0) {
    previous_line = self->lines[self->line_struct_count-1].line;
  }

  // resize the byte backing buffer
  if(initial_byte_capacity < initial_byte_count + 1) {
    self->byte_capacity = MEMORY_GROW_CAPACITY(initial_byte_capacity, CHUNK_INITIAL_CAPACITY);
    self->buffer = MEMORY_GROW_ARRAY(uint8_t, self->buffer, initial_byte_capacity, self->byte_capacity);
  }

  // resize the backing buffer used for line debug storage
  if(initial_line_struct_capacity < initial_line_struct_count + 1) {
    self->line_struct_capacity = MEMORY_GROW_CAPACITY(initial_line_struct_capacity, CHUNK_LINE_INITIAL_CAPACITY);
    self->lines = MEMORY_GROW_ARRAY(struct Line, self->lines, initial_line_struct_capacity, self->line_struct_capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  self->buffer[self->byte_count] = byte;
  self->byte_count += 1;

  // check if line is same as previous, otherwise write a new line
  if(line == previous_line && previous_line != 0) {
    // access current line structure
    self->lines[initial_line_struct_count-1].line_count += 1;
  } else {
    self->lines[initial_line_struct_count] = (struct Line) { .line = line, .line_count = 1 };
    self->line_struct_count += 1;
  }
}

size_t chunk_add_constant(struct Chunk *self, const Value constant) {
  value_array_write(&self->constants, constant);
  // return index where the constant was inserted for future access
  return self->constants.value_count - 1;
}

size_t chunk_get_line(struct Chunk *const self, const size_t offset) {
  size_t line_struct_count = self->line_struct_count;

  size_t line = 0;
  size_t prev_line_count = 0;
  
  for(size_t i = 0; i < line_struct_count; ++i) {
    size_t curr_line_count = self->lines[i].line_count + prev_line_count;

    if(prev_line_count <= offset && offset < curr_line_count) {
      line = self->lines[i].line;
      break;
    }

    prev_line_count = curr_line_count;
  }

  return line;
}

// --- FILE-LOCAL HELPER FUNCTIONS ---