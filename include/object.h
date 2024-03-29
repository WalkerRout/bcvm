#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "value.h"
#include "memory.h"

enum ObjectType {
  OBJECT_TYPE_STRING
};

struct Object {
  enum ObjectType type;
  struct Object *next;
};

#define OBJECT_TYPE(value) ((value.as.object)->type)

struct ObjectString {
  struct Object object;
  size_t length;
  uint32_t hash;
  char buffer[]; // sizeof treats as 0
};

#define OBJECT_STRING_FROM_VALUE(value)        ((struct ObjectString *) (value).as.object)
#define OBJECT_STRING_CSTR_FROM_VALUE(value)   ((struct ObjectString *) (value).as.object)->buffer
#define OBJECT_STRING_FROM_OBJECT(object)      ((struct ObjectString *) (object))
#define OBJECT_STRING_CSTR_FROM_OBJECT(object) ((struct ObjectString *) (object))->buffer
#define OBJECT_IS_OBJECT_STRING(value)         object_is_object_type(value, OBJECT_TYPE_STRING)
static inline uint8_t object_is_object_type(struct Value value, enum ObjectType type) {
  return VALUE_IS_OBJECT(value) && OBJECT_TYPE(value) == type;
}

struct ObjectString *object_object_string_from_parts(const char *buffer, size_t length);
struct ObjectString *object_object_string_copy(struct ObjectString *string);
struct ObjectString *object_object_string_allocate(size_t length);
void object_object_string_update_hash(struct ObjectString *string);
struct Object *object_allocate_object(size_t size, enum ObjectType type);
void object_free_objects(void);
void object_print(struct Value value);

#endif // OBJECT_H