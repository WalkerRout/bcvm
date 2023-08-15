#ifndef VM_H
#define VM_H

#include "common.h"
#include "chunk.h"

enum InterpretResult {
  INTERPRET_RESULT_OK,
  INTERPRET_RESULT_COMPILE_ERROR,
  INTERPRET_RESULT_RUNTIME_ERROR
};

struct VM {
  struct Chunk *chunk;
  uint8_t *ip; // instruction pointer
};

// static singleton instance is modified
void vm_init(void);
void vm_free(void);
enum InterpretResult vm_interpret(struct Chunk *chunk);

#endif // VM_H