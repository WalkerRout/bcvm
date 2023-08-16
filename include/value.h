#ifndef VALUE_H
#define VALUE_H

#include "common.h"

#define VALUE_ARRAY_INITIAL_CAPACITY 8

enum ValueType {
  VALUE_TYPE_BOOL,
  VALUE_TYPE_NIL, 
  VALUE_TYPE_NUMBER,
};

struct Value {
  enum ValueType type;
  union {
    uint8_t boolean;
    double number;
  } as;
};

#define VALUE_IS_NIL(value)     ((value).type == VALUE_TYPE_NIL)
#define VALUE_IS_BOOL(value)    ((value).type == VALUE_TYPE_BOOL)
#define VALUE_IS_NUMBER(value)  ((value).type == VALUE_TYPE_NUMBER)

#define VALUE_NIL           ((struct Value) {.type = VALUE_TYPE_NIL, .as = {.number = 0}})
#define VALUE_BOOL(value)   ((struct Value) {.type = VALUE_TYPE_BOOL, .as = {.boolean = value}})
#define VALUE_NUMBER(value) ((struct Value) {.type = VALUE_TYPE_NUMBER, .as = {.number = value}})

struct ValueArray {
  size_t value_count;
  size_t value_capacity;
  struct Value *buffer;
};

void value_array_init(struct ValueArray *self);
void value_array_free(struct ValueArray *self);
void value_array_write(struct ValueArray *self, struct Value value);
void value_print(struct Value value);

#endif // VALUE_H