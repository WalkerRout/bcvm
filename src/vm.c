
#include <stdio.h>

#include "vm.h"
#include "debug.h"
#include "opcode.h"
#include "compiler.h"

// global singleton instance
static struct VM global_vm = {0};

// file local prototypes
static enum InterpretResult vm_run(void);
static void vm_reset_stack(void);

void vm_init(void) {
  vm_reset_stack();
}

void vm_free(void) {}

enum InterpretResult vm_interpret(const char *source) {
  struct Chunk chunk = {0};
  chunk_init(&chunk);

  if (!compiler_compile(source, &chunk)) {
    chunk_free(&chunk);
    return INTERPRET_RESULT_COMPILE_ERROR;
  }

  global_vm.chunk = &chunk;
  global_vm.ip = global_vm.chunk->buffer;

  enum InterpretResult result = vm_run();

  chunk_free(&chunk);
  return result;
}

void vm_push(Value value) {
  *global_vm.stack_top = value;
  global_vm.stack_top += 1;
}

Value vm_pop() {
  global_vm.stack_top -= 1;
  return *global_vm.stack_top;
}

// file local functions

static enum InterpretResult vm_run(void) {
  uint8_t *ip = global_vm.ip;
  Value *values = global_vm.chunk->constants.buffer;

#define READ_BYTE()     (*ip++)
#define READ_CONSTANT() (values[READ_BYTE()])
#define BINARY_OP(op)     do { \
    double b = vm_pop();       \
    double a = vm_pop();       \
    vm_push(a op b);           \
  } while(0)

  printf("\n== Running Virtal Machine ==\n");
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = global_vm.stack; slot < global_vm.stack_top; ++slot) {
      printf("[ ");
      value_print(*slot);
      printf(" ]");
    }
    printf("\n");

    debug_disassemble_instruction(global_vm.chunk, (size_t)(ip - global_vm.chunk->buffer));
#endif

    uint8_t instruction = 0;
    switch (instruction = READ_BYTE()) {
      case OPCODE_CONSTANT: {
        Value constant = READ_CONSTANT();
        vm_push(constant);
      } break;

      case OPCODE_CONSTANT_LONG: {
        size_t value_index = 
          (READ_BYTE() << 16) |
          (READ_BYTE() << 8)  |
          (READ_BYTE() << 0);

        Value constant = values[value_index];
        vm_push(constant);
      } break;

      case OPCODE_ADD: { BINARY_OP(+); } break;

      case OPCODE_SUBTRACT: { BINARY_OP(-); } break;

      case OPCODE_MULTIPLY: { BINARY_OP(*); } break;

      case OPCODE_DIVIDE: { BINARY_OP(/); } break;

      case OPCODE_NEGATE: { global_vm.stack_top[-1] = -global_vm.stack_top[-1]; } break; // top of stack, index back by 1

      case OPCODE_RETURN: { 
        value_print(vm_pop());
        printf("\n");
        return INTERPRET_RESULT_OK; 
      } break;

      default: {}
    }
  }

  global_vm.ip = ip;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

static void vm_reset_stack(void) {
  global_vm.stack_top = global_vm.stack;
}