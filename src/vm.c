
#include <stdio.h>
#include <stdarg.h>

#include "vm.h"
#include "debug.h"
#include "opcode.h"
#include "scanner.h"
#include "compiler.h"

// global singleton instance
static struct VM global_vm = {0};

// file local prototypes
static enum InterpretResult vm_run(void);
static void vm_reset_stack(void);
static struct Value vm_peek(size_t distance);
static void vm_runtime_error(const char *format, ...);

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
  //enum InterpretResult result = INTERPRET_RESULT_OK;

  chunk_free(&chunk);
  return result;
}

void vm_push(struct Value value) {
  *global_vm.stack_top = value;
  global_vm.stack_top += 1;
}

struct Value vm_pop() {
  global_vm.stack_top -= 1;
  return *global_vm.stack_top;
}

// file local functions

static enum InterpretResult vm_run(void) {

#define READ_BYTE()     (*global_vm.ip++)
#define READ_CONSTANT() (global_vm.chunk->constants.buffer[READ_BYTE()])
#define BINARY_OP(op) do {         \
    double b = vm_pop().as.number; \
    double a = vm_pop().as.number; \
    vm_push(VALUE_NUMBER(a op b)); \
  } while(0)

  printf("\n== Running Virtal Machine ==\n");
  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("stack:\t");
    for (struct Value *slot = global_vm.stack; slot < global_vm.stack_top; ++slot) {
      printf("[ ");
      value_print(*slot);
      printf(" ]");
    }
    printf("\n");

    size_t offset = (size_t)(global_vm.ip - global_vm.chunk->buffer);
    assert(offset < global_vm.chunk->byte_count);
    debug_disassemble_instruction(global_vm.chunk, offset);
#endif

    uint8_t instruction = 0;
    switch (instruction = READ_BYTE()) {
      case OPCODE_CONSTANT: {
        struct Value constant = READ_CONSTANT();
        vm_push(constant);
      } break;

      case OPCODE_CONSTANT_LONG: {
        size_t value_index = 
          (READ_BYTE() << 16) |
          (READ_BYTE() << 8)  |
          (READ_BYTE() << 0);

        struct Value constant = global_vm.chunk->constants.buffer[value_index];
        vm_push(constant);
      } break;

      case OPCODE_ADD: { BINARY_OP(+); } break;

      case OPCODE_SUBTRACT: { BINARY_OP(-); } break;

      case OPCODE_MULTIPLY: { BINARY_OP(*); } break;

      case OPCODE_DIVIDE: { BINARY_OP(/); } break;

      case OPCODE_NEGATE: {
        if (!VALUE_IS_NUMBER(vm_peek(0))) {
          vm_runtime_error("Error - operand must be a number");
          return INTERPRET_RESULT_RUNTIME_ERROR;
        }
        vm_push(VALUE_NUMBER(-vm_pop().as.number));
      } break; // top of stack, index back by 1

      case OPCODE_RETURN: {
        value_print(vm_pop());
        printf("\n");
        return INTERPRET_RESULT_OK; 
      } break;

      default: {}
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

static void vm_reset_stack(void) {
  global_vm.stack_top = global_vm.stack;
}

static struct Value vm_peek(size_t distance) {
  return global_vm.stack_top[-1 - distance];
}

static void vm_runtime_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = global_vm.ip - global_vm.chunk->buffer - 1;
  size_t line = chunk_get_line(global_vm.chunk, instruction);
  fprintf(stderr, "[line %d] in script\n", line);
  vm_reset_stack();
}