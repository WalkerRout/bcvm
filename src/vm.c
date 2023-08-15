
#include <stdio.h>

#include "vm.h"
#include "debug.h"
#include "opcode.h"

// global singleton instance
static struct VM global_vm;

// file local prototypes
static enum InterpretResult vm_run(void);

void vm_init(void) {}

void vm_free(void) {}

enum InterpretResult vm_interpret(struct Chunk *chunk) {
  global_vm.chunk = chunk;
  global_vm.ip = global_vm.chunk->buffer;
  return vm_run();
}

// file local functions

static enum InterpretResult vm_run(void) {
  uint8_t *ip = global_vm.ip;
  Value *values = global_vm.chunk->constants.buffer;

#define READ_BYTE()     (*ip++)
#define READ_CONSTANT() (values[READ_BYTE()])

  printf("\n== Running Virtal Machine ==\n");
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    debug_disassemble_instruction(global_vm.chunk, (size_t)(ip - global_vm.chunk->buffer));
#endif

    uint8_t instruction = 0;
    switch (instruction = READ_BYTE()) {
      case OPCODE_CONSTANT: {
        Value constant = READ_CONSTANT();
        //value_print(constant);
        //printf("\n");
      } break;

      case OPCODE_CONSTANT_LONG: {
        size_t value_index = 
          (READ_BYTE() << 16) |
          (READ_BYTE() << 8)  |
          (READ_BYTE() << 0);

        Value constant = values[value_index];
        //value_print(constant);
        //printf("\n");
      } break;

      case OPCODE_RETURN: {
        return INTERPRET_RESULT_OK;
      } break;

      default: {}
    }
  }

  global_vm.ip = ip;

#undef READ_BYTE
#undef READ_CONSTANT
}