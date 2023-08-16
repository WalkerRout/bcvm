
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "value.h"
#include "memory.h"
#include "object.h"

#define EPSILON 1e-10

// file local prototypes
uint8_t double_approx(double a, double b, double epsilon);
uint8_t string_equal(struct Object *a, struct Object *b);

uint8_t value_equal(struct Value a, struct Value b) {
  if (a.type != b.type) return FALSE;
  switch (a.type) {
    case VALUE_TYPE_BOOL:   return a.as.boolean == b.as.boolean;                     break;
    case VALUE_TYPE_NIL:    return TRUE;                                             break;
    case VALUE_TYPE_NUMBER: return double_approx(a.as.number, b.as.number, EPSILON); break;
    case VALUE_TYPE_OBJECT: return string_equal(a.as.object, b.as.object);
    default:                return FALSE; // unreachable
  }
}

inline void value_array_init(struct ValueArray *self) {
  self->value_count = 0;
  self->value_capacity = 0;
  self->buffer = NULL;
}

inline void value_array_free(struct ValueArray *self) {
  MEMORY_FREE_ARRAY(struct Value, self->buffer, self->value_capacity);
  
  value_array_init(self);
}

void value_array_write(struct ValueArray *self, struct Value value) {
  size_t initial_value_count = self->value_count;
  size_t initial_capacity = self->value_capacity;

  if(initial_capacity < initial_value_count + 1) {
    // resize the backing buffer
    self->value_capacity = MEMORY_GROW_CAPACITY(initial_capacity, VALUE_ARRAY_INITIAL_CAPACITY);
    self->buffer = MEMORY_GROW_ARRAY(struct Value, self->buffer, initial_capacity, self->value_capacity);
  }

  // write a byte (0-indexed, so can just use byte_count)
  self->buffer[initial_value_count] = value;
  self->value_count += 1;
}

inline void value_print(struct Value value) {
  switch (value.type) {
    case VALUE_TYPE_NIL:    printf("nil");                               break;
    case VALUE_TYPE_BOOL:   printf(value.as.boolean ? "true" : "false"); break;
    case VALUE_TYPE_NUMBER: printf("%g", value.as.number);               break;
    case VALUE_TYPE_OBJECT: object_print(value);                         break;
    default: return; // unreachable
  }
}

// file local functions

uint8_t double_approx(double a, double b, double epsilon) {
  double diff = fabs(a - b);
    
  return diff <= epsilon
    ? TRUE
    : FALSE;
}

uint8_t string_equal(struct Object *a, struct Object *b) {
  struct ObjectString *a_str = OBJECT_STRING_FROM_OBJECT(a);
  struct ObjectString *b_str = OBJECT_STRING_FROM_OBJECT(b);
  return a_str->length == b_str->length &&
         memcmp(a_str->buffer, b_str->buffer, a_str->length) == 0;
}