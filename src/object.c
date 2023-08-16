
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

#define OBJECT_ALLOCATE(type, object_type) (type *) allocate_object(sizeof(type), object_type)

// file local prototypes
static struct Object *allocate_object(size_t size, enum ObjectType type);
static struct ObjectString *allocate_string(char *chars, size_t length);
static void object_free_object(struct Object *object);

struct ObjectString *object_copy_string(const char *chars, size_t length) {
  char *heap_chars = MEMORY_ALLOCATE(char, length + 1);
  memcpy(heap_chars, chars, length);
  heap_chars[length] = '\0';
  return allocate_string(heap_chars, length);
}

struct ObjectString *object_move_string(char *chars, size_t length) {
  return allocate_string(chars, length);
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

// file local functions

static struct Object *allocate_object(size_t size, enum ObjectType type) {
  struct Object *object = (struct Object *) memory_reallocate(NULL, 0, size);
  object->type = type;

  object->next = global_vm.objects;
  global_vm.objects = object;

  return object;
}

static struct ObjectString *allocate_string(char *chars, size_t length) {
  struct ObjectString *string = OBJECT_ALLOCATE(struct ObjectString, OBJECT_TYPE_STRING);
  string->length = length;
  string->buffer = chars;
  return string;
}

static void object_free_object(struct Object *object) {
  switch (object->type) {
    case OBJECT_TYPE_STRING: {
      struct ObjectString *string = OBJECT_STRING_FROM_OBJECT(object);
      MEMORY_FREE_ARRAY(char, string->buffer, string->length + 1);
      MEMORY_FREE(struct ObjectString, object);
      break;
    }
  }
}