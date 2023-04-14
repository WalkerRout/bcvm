
#include <stdio.h>

#include "debug.h"
#include "chunk.h"
#include "opcode.h"

static size_t disassemble_instruction(struct Chunk *chunk, const size_t offset);
static inline size_t display_one_byte_instruction(const char *instruction_name, const size_t offset);
static inline size_t display_two_byte_instruction(const char *instruction_name, const Value value, const size_t offset);

void debug_disassemble_chunk(struct Chunk *chunk, const char *message) {
  printf("== %s ==\n", message);
  for(size_t offset = 0; offset < chunk->byte_count;) {
    offset = disassemble_instruction(chunk, offset);
  }

  if(chunk->constants.value_count > 0)
    debug_disassemble_value_array(&chunk->constants);
}

void debug_disassemble_value_array(struct ValueArray *value_array) {
  printf("== ValueArray ==\n");
  for(size_t i = 0; i < value_array->value_count; ++i) {
    printf("%lf ", value_array->buffer[i]);
  }
  printf("\n");
}

// --- FILE-LOCAL HELPER FUNCTIONS ---

static size_t disassemble_instruction(struct Chunk *chunk, const size_t offset) {
  printf("%04zd ", offset);
  uint8_t instruction = chunk->buffer[offset];
  
  switch(instruction) {
    case OPCODE_CONSTANT:
      printf("at index %d - ", chunk->buffer[offset + 1]);
      size_t value_index = chunk->buffer[offset + 1];
      Value value = chunk->constants.buffer[value_index];
      return display_two_byte_instruction("OP_CONSTANT", value, offset);
      break; // not really needed, but good structure

    case OPCODE_RETURN:
      return display_one_byte_instruction("OP_RETURN", offset);
      break;

    default:
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
  }
}

static inline size_t display_one_byte_instruction(const char *instruction_name, const size_t offset) {
  printf("%s\n", instruction_name);
  return offset + 1;
}

static inline size_t display_two_byte_instruction(const char *instruction_name, const Value value, const size_t offset) {
  printf("%s ", instruction_name);
  value_print(value);
  printf("\n");
  return offset + 2;
}