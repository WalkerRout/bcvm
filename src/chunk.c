
#include <stdio.h>

#include "chunk.h"
#include "opcode.h"
#include "memory.h"

inline void chunk_init(struct Chunk *chunk) {
  chunk->byte_count = 0;
  chunk->byte_capacity   = 0;
  chunk->buffer = NULL;

  line_array_init(&chunk->lines);
  value_array_init(&chunk->constants);
}

inline void chunk_free(struct Chunk *chunk) {
  value_array_free(&chunk->constants);
  line_array_free(&chunk->lines);

  MEMORY_FREE_ARRAY(uint8_t, chunk->buffer, chunk->byte_capacity);

  chunk_init(chunk);
}

void chunk_write(struct Chunk *chunk, const uint8_t byte, const size_t line) {
  size_t initial_byte_count = chunk->byte_count;
  size_t initial_byte_capacity = chunk->byte_capacity;

  // resize the byte backing buffer
  if(initial_byte_capacity < initial_byte_count + 1) {
    chunk->byte_capacity = MEMORY_GROW_CAPACITY(initial_byte_capacity, CHUNK_INITIAL_CAPACITY);
    chunk->buffer = MEMORY_GROW_ARRAY(uint8_t, chunk->buffer, initial_byte_capacity, chunk->byte_capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  chunk->buffer[chunk->byte_count] = byte;
  chunk->byte_count += 1;

  line_array_write(&chunk->lines, line);
}

size_t chunk_add_constant(struct Chunk *chunk, const struct Value constant) {
  value_array_write(&chunk->constants, constant);
  // return index where the constant was inserted for future access
  return chunk->constants.value_count - 1;
}

size_t chunk_write_constant(struct Chunk *chunk, const struct Value constant, const size_t line) {
  size_t value_index = chunk_add_constant(chunk, constant);

  if (chunk->constants.value_count >= 0xFF) {
    chunk_write(chunk, OPCODE_CONSTANT_LONG, line);
    chunk_write(chunk, (value_index >> 16) & 0xFF, line);
    chunk_write(chunk, (value_index >> 8)  & 0xFF, line);
  } else {
    chunk_write(chunk, OPCODE_CONSTANT, line);
  }

  chunk_write(chunk, value_index & 0xFF, line);

  return chunk->constants.value_count - 1;
}

size_t chunk_get_line(struct Chunk *const chunk, const size_t offset) {
  size_t line_struct_count = chunk->lines.line_struct_count;

  size_t line = 0;
  size_t prev_line_count = 0;
  
  for(size_t i = 0; i < line_struct_count; ++i) {
    size_t curr_line_count = chunk->lines.lines[i].line_count + prev_line_count;

    if(prev_line_count <= offset && offset < curr_line_count) {
      line = chunk->lines.lines[i].line;
      break;
    }

    prev_line_count = curr_line_count;
  }

  return line;
}
