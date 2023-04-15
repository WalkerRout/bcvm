
#include <stdio.h>

#include "value.h"
#include "memory.h"

inline void value_array_init(struct ValueArray *self) {
  self->value_count = 0;
  self->value_capacity    = 0;
  self->buffer = NULL;
}

inline void value_array_free(struct ValueArray *self) {
  MEMORY_FREE_ARRAY(Value, self->buffer, self->value_capacity);
  
  value_array_init(self);
}

void value_array_write(struct ValueArray *self, Value value) {
  size_t initial_value_count = self->value_count;
  size_t initial_capacity = self->value_capacity;

  if(initial_capacity < initial_value_count + 1) {
    // resize the backing buffer
    self->value_capacity = MEMORY_GROW_CAPACITY(initial_capacity, VALUE_ARRAY_INITIAL_CAPACITY);
    self->buffer = MEMORY_GROW_ARRAY(Value, self->buffer, initial_capacity, self->value_capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  self->buffer[initial_value_count] = value;
  self->value_count += 1;
}

inline void value_print(Value value) {
  printf("%lf", value);
}

// --- FILE-LOCAL HELPER FUNCTIONS ---