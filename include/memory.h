#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

#define MEMORY_GROW_CAPACITY(capacity, default_cap) \
  ((capacity) < (default_cap) ? (default_cap) : (capacity) * 2)

#define MEMORY_GROW_ARRAY(type, pointer, old_count, new_count) \
  (type*)memory_reallocate(pointer, sizeof(type) * (old_count), sizeof(type) * (new_count))

#define MEMORY_FREE_ARRAY(type, pointer, old_count) \
  (void)memory_reallocate(pointer, sizeof(type) * (old_count), 0)

void *memory_reallocate(void *buffer, size_t old_size, size_t new_size);

#endif // MEMORY_H