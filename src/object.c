
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

// file local prototypes
static void object_free_object(struct Object *object);

struct ObjectString *object_object_string_from_parts(const char *buffer, size_t length) {
  struct ObjectString *new_string = object_allocate_string(length);
  memcpy(new_string->buffer, buffer, length);
  new_string->buffer[length] = '\0';
  return new_string;
}

struct ObjectString *object_copy_string(struct ObjectString *string) {
  struct ObjectString *new_string = object_allocate_string(string->length);
  memcpy(new_string->buffer, string->buffer, string->length);
  new_string->buffer[string->length] = '\0';
  return new_string;
}

void object_free_objects(void) {
  struct Object *object = global_vm.objects;
  while (object != NULL) {
    struct Object *next = object->next;
    object_free_object(object);
    object = next;
  }
}

void object_print(struct Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJECT_TYPE_STRING: printf("%s", OBJECT_STRING_CSTR_FROM_VALUE(value)); break;
  }
}

struct Object *object_allocate_object(size_t size, enum ObjectType type) {
  struct Object *object = memory_reallocate(NULL, 0, size);
  object->type = type;

  object->next = global_vm.objects;
  global_vm.objects = object;

  return object;
}

// allocate a single sized buffer with 
struct ObjectString *object_allocate_string(size_t length) {
  struct ObjectString *string = (struct ObjectString *) object_allocate_object(sizeof(struct ObjectString) + length + 1, OBJECT_TYPE_STRING);
  string->length = length;
  return string;
}

// file local functions

static void object_free_object(struct Object *object) {
  switch (object->type) {
    case OBJECT_TYPE_STRING: {
      struct ObjectString *string = OBJECT_STRING_FROM_OBJECT(object);
      memory_reallocate(string, sizeof(struct ObjectString) + string->length + 1, 0);
      break;
    }
  }
}