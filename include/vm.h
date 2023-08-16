#ifndef VM_H
#define VM_H

#include "common.h"
#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

enum InterpretResult {
  INTERPRET_RESULT_OK,
  INTERPRET_RESULT_COMPILE_ERROR,
  INTERPRET_RESULT_RUNTIME_ERROR
};

struct VM {
  struct Chunk *chunk;
  uint8_t *ip; // instruction pointer
  struct Value stack[STACK_MAX];
  struct Value *stack_top;
};

// static singleton instance is modified
void vm_init(void);
void vm_free(void);
enum InterpretResult vm_interpret(const char *source);
void vm_push(struct Value value);
struct Value vm_pop();

#endif // VM_H