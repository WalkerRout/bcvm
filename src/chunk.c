
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

static inline size_t type_size(void);

inline void chunk_init(struct Chunk *self) {
  self->byte_count = 0;
  self->capacity   = 0;
  self->buffer = NULL;
  self->lines  = NULL;
  value_array_init(&self->constants);
}

inline void chunk_free(struct Chunk *self) {
  size_t ts = type_size();

  value_array_free(&self->constants);
  MEMORY_FREE_ARRAY(uint8_t, &self->constants, self->capacity);
}

void chunk_write(struct Chunk *self, uint8_t byte, size_t line) {
  size_t initial_byte_count = self->byte_count;
  size_t initial_capacity = self->capacity;

  if(initial_capacity < initial_byte_count + 1) {
    // resize the backing buffer
    self->capacity = MEMORY_GROW_CAPACITY(initial_capacity, CHUNK_INITIAL_CAPACITY);
    self->buffer = MEMORY_GROW_ARRAY(uint8_t, self->buffer, initial_capacity, self->capacity);
    self->lines = MEMORY_GROW_ARRAY(size_t, self->lines, initial_capacity, self->capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  self->buffer[self->byte_count] = byte;
  self->lines[self->byte_count] = line;
  self->byte_count += 1;
}

size_t chunk_add_constant(struct Chunk *self, Value constant) {
  value_array_write(&self->constants, constant);
  // return index where the constant was inserted for future access
  return self->constants.value_count - 1;
}

// --- FILE-LOCAL HELPER FUNCTIONS ---

static inline size_t type_size(void) {
  return sizeof(uint8_t);
}
