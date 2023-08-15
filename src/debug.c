
#include <stdio.h>

#include "debug.h"
#include "chunk.h"
#include "opcode.h"

// file local prototypes

static inline size_t display_one_byte_instruction(const char *instruction_name, const size_t offset);
static inline size_t display_two_byte_instruction(const char *instruction_name, const Value value, const size_t offset);
static inline size_t display_four_byte_instruction(const char *instruction_name, const Value value, const size_t offset);

void debug_disassemble_chunk(struct Chunk *chunk, const char *message) {
  printf("== %s ==\n", message);
  for(size_t offset = 0; offset < chunk->byte_count;)
    offset = debug_disassemble_instruction(chunk, offset);

  if(chunk->constants.value_count > 0)
    debug_disassemble_value_array(&chunk->constants);
}

void debug_disassemble_value_array(struct ValueArray *value_array) {
  printf("== ValueArray ==\n");
  for(size_t i = 0; i < value_array->value_count; ++i)
    printf("%lf ", value_array->buffer[i]);
  printf("\n");
}

size_t debug_disassemble_instruction(struct Chunk *chunk, const size_t offset) {
  printf("%04d ", offset);

  if(offset > 0 && chunk_get_line(chunk, offset) == chunk_get_line(chunk, offset - 1))
    printf(" | "); // show '|' for any instruction that comes from the same source line as prev
  else
    printf("%4d ", chunk_get_line(chunk, offset));

  uint8_t instruction = chunk->buffer[offset];
  
  switch(instruction) {
    case OPCODE_CONSTANT: {
      size_t value_index = chunk->buffer[offset + 1];
      Value value = chunk->constants.buffer[value_index];
      printf("\tat index %lu - ", value_index);
      return display_two_byte_instruction("OP_CONSTANT", value, offset);
    } break; // not really needed, but good structure

    case OPCODE_CONSTANT_LONG: {
      size_t value_index = 
        (chunk->buffer[offset + 3] << 0)  |
        (chunk->buffer[offset + 2] << 8)  |
        (chunk->buffer[offset + 1] << 16);

      Value value = chunk->constants.buffer[value_index];
      printf("\tat index %lu - ", value_index);
      return display_four_byte_instruction("OP_CONSTANT_LONG", value, offset);
    } break; // not really needed, but good structure

    case OPCODE_RETURN: {
      return display_one_byte_instruction("OP_RETURN", offset);
    } break;

    default: {
      printf("Unknown opcode %d\n", instruction);
      return offset + 1;
    }
  }
}

// file local functions

static inline size_t display_one_byte_instruction(const char *instruction_name, const size_t offset) {
  printf("\t%s\n", instruction_name);
  return offset + 1;
}

static inline size_t display_two_byte_instruction(const char *instruction_name, const Value value, const size_t offset) {
  printf("%s ", instruction_name);
  value_print(value);
  printf("\n");
  return offset + 2;
}

static inline size_t display_four_byte_instruction(const char *instruction_name, const Value value, const size_t offset) {
  printf("%s ", instruction_name);
  value_print(value);
  printf("\n");
  return offset + 4;
}