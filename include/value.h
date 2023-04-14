#ifndef VALUE_H
#define VALUE_H

#include "common.h"

#define VALUE_ARRAY_INITIAL_CAPACITY 8

typedef double Value;

struct ValueArray {
  size_t value_count;
  size_t capacity;
  Value* buffer;
};

void value_array_init(struct ValueArray *self);
void value_array_free(struct ValueArray *self);
void value_array_write(struct ValueArray *self, Value value);
void value_print(Value value);

#endif // VALUE_H