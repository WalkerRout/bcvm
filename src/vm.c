
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "vm.h"
#include "debug.h"
#include "opcode.h"
#include "scanner.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"

// global singleton instance (declared extern in header)
struct VM global_vm = {0};
 
// file local prototypes
static enum InterpretResult vm_run(void);
static void vm_reset_stack(void);
static struct Value vm_peek(size_t distance);
static void vm_runtime_error(const char *format, ...);
static uint8_t is_falsey(struct Value value);
static void string_concatenate(void);
// binary op functions
static uint8_t gt(double a, double b);
static uint8_t gt_eq(double a, double b);
static uint8_t lt(double a, double b);
static uint8_t lt_eq(double a, double b);
static double add(double a, double b);
static double subtract(double a, double b);
static double multiply(double a, double b);
static double divide(double a, double b);

void vm_init(void) {
  vm_reset_stack();
  global_vm.objects = NULL;
}

void vm_free(void) {
  object_free_objects();
}

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
#define BINARY_OP(value_type, op) do {                      \
    if (!VALUE_IS_NUMBER(vm_peek(0)) ||                     \
        !VALUE_IS_NUMBER(vm_peek(1))) {                     \
      vm_runtime_error("Error - operands must be numbers"); \
      return INTERPRET_RESULT_RUNTIME_ERROR;                \
    }                                                       \
    double b = vm_pop().as.number;                          \
    double a = vm_pop().as.number;                          \
    vm_push(value_type(op(a, b)));                          \
  } while (FALSE)

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

      case OPCODE_NIL:   vm_push(VALUE_NIL());         break;
      case OPCODE_TRUE:  vm_push(VALUE_BOOL(TRUE));   break;
      case OPCODE_FALSE: vm_push(VALUE_BOOL(FALSE)); break;

      case OPCODE_BANG_EQUAL: {
        struct Value b = vm_pop();
        struct Value a = vm_pop();
        vm_push(VALUE_BOOL(!value_equal(a, b)));
      } break;
      case OPCODE_EQUAL_EQUAL: {
        struct Value b = vm_pop();
        struct Value a = vm_pop();
        vm_push(VALUE_BOOL(value_equal(a, b)));
      } break;
      case OPCODE_GREATER:       BINARY_OP(VALUE_BOOL, gt);    break;
      case OPCODE_GREATER_EQUAL: BINARY_OP(VALUE_BOOL, gt_eq); break; // a >= b <-> !(a < b)
      case OPCODE_LESS:          BINARY_OP(VALUE_BOOL, lt);    break;
      case OPCODE_LESS_EQUAL:    BINARY_OP(VALUE_BOOL, lt_eq); break; // a <= b <-> !(a > b)

      case OPCODE_ADD: {
        if (OBJECT_IS_OBJECT_STRING(vm_peek(0)) && OBJECT_IS_OBJECT_STRING(vm_peek(1))) {
          string_concatenate();
        } else if (VALUE_IS_NUMBER(vm_peek(0)) && VALUE_IS_NUMBER(vm_peek(1))) {
          double b = vm_pop().as.number;
          double a = vm_pop().as.number;
          vm_push(VALUE_NUMBER(add(a, b)));
        } else {
          vm_runtime_error("Error - operands must be two numbers or two strings");
          return INTERPRET_RESULT_RUNTIME_ERROR;
        }
      } break;
      case OPCODE_SUBTRACT: BINARY_OP(VALUE_NUMBER, subtract); break;
      case OPCODE_MULTIPLY: BINARY_OP(VALUE_NUMBER, multiply); break;
      case OPCODE_DIVIDE:   BINARY_OP(VALUE_NUMBER, divide);   break;

      case OPCODE_NOT: vm_push(VALUE_BOOL(is_falsey(vm_pop()))); break;
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
  fprintf(stderr, "[line %lu] in script\n", line);
  vm_reset_stack();
}

static uint8_t is_falsey(struct Value value) {
  // && short circuits, access is safe
  return VALUE_IS_NIL(value) || (VALUE_IS_BOOL(value) && !value.as.boolean);
}

static void string_concatenate(void) {
  struct ObjectString *b = OBJECT_STRING_FROM_VALUE(vm_pop());
  struct ObjectString *a = OBJECT_STRING_FROM_VALUE(vm_pop());

  size_t length = a->length + b->length;

  // call to object_allocate_object adds new node to allocation list
  struct ObjectString *result = object_allocate_string(length);

  memcpy(result->buffer, a->buffer, a->length);
  memcpy(result->buffer + a->length, b->buffer, b->length);
  result->buffer[length] = '\0';

  object_object_string_update_hash(result);

  vm_push(VALUE_OBJECT(result));
}

static uint8_t gt(double a, double b)      { return a > b;  }
static uint8_t gt_eq(double a, double b)   { return a >= b; }
static uint8_t lt(double a, double b)      { return a < b;  }
static uint8_t lt_eq(double a, double b)   { return a <= b; }
static double add(double a, double b)      { return a + b;  }
static double subtract(double a, double b) { return a - b;  }
static double multiply(double a, double b) { return a * b;  }
static double divide(double a, double b)   { return a / b;  }

