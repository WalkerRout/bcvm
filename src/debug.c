
#include <stdio.h>

#include "debug.h"
#include "chunk.h"
#include "opcode.h"

// file local prototypes

static inline size_t display_one_byte_instruction(const char *instruction_name, const size_t offset);
static inline size_t display_two_byte_instruction(const char *instruction_name, const struct Value value, const size_t offset);
static inline size_t display_four_byte_instruction(const char *instruction_name, const struct Value value, const size_t offset);

void debug_disassemble_chunk(struct Chunk *chunk, const char *message) {
  printf("== %s ==\n", message);
  for(size_t offset = 0; offset < chunk->byte_count;) {
    offset = debug_disassemble_instruction(chunk, offset);
  }

  if(chunk->constants.value_count > 0)
    debug_disassemble_value_array(&chunk->constants, message);
}

void debug_disassemble_value_array(struct ValueArray *value_array, const char *message) {
  printf("== %s ==\n", message);
  for(size_t i = 0; i < value_array->value_count; ++i) {
    value_print(value_array->buffer[i]);
    printf(" ");
  }
  printf("\n");
}

size_t debug_disassemble_instruction(struct Chunk *chunk, const size_t offset) {
  printf("%04lu", offset);

  if(offset > 0 && chunk_get_line(chunk, offset) == chunk_get_line(chunk, offset - 1))
    printf("   | "); // show '|' for any instruction that comes from the same source line as prev
  else
    printf("%4lu ", chunk_get_line(chunk, offset));

  assert(offset < chunk->byte_count);
  uint8_t instruction = chunk->buffer[offset];

  switch(instruction) {
    case OPCODE_CONSTANT: {
      assert(offset+1 < chunk->byte_count);
      size_t value_index = chunk->buffer[offset + 1];
      struct Value value = chunk->constants.buffer[value_index];
      printf("\tat index %lu - ", value_index);
      return display_two_byte_instruction("OPCODE_CONSTANT", value, offset);
    } break; // not really needed, but good structure
    
    case OPCODE_CONSTANT_LONG: {
      size_t value_index = 
        (chunk->buffer[offset + 3] << 0)  |
        (chunk->buffer[offset + 2] << 8)  |
        (chunk->buffer[offset + 1] << 16);

      struct Value value = chunk->constants.buffer[value_index];
      printf("\tat index %lu - ", value_index);
      return display_four_byte_instruction("OPCODE_CONSTANT_LONG", value, offset);
    } break; // not really needed, but good structure

    case OPCODE_NIL:   return display_one_byte_instruction("OPCODE_NIL", offset);   break;
    case OPCODE_TRUE:  return display_one_byte_instruction("OPCODE_TRUE", offset);  break;
    case OPCODE_FALSE: return display_one_byte_instruction("OPCODE_FALSE", offset); break;

    case OPCODE_BANG_EQUAL:    return display_one_byte_instruction("OPCODE_BANG_EQUAL", offset);    break;
    case OPCODE_EQUAL_EQUAL:   return display_one_byte_instruction("OPCODE_EQUAL_EQUAL", offset);   break;
    case OPCODE_GREATER:       return display_one_byte_instruction("OPCODE_GREATER", offset);       break;
    case OPCODE_GREATER_EQUAL: return display_one_byte_instruction("OPCODE_GREATER_EQUAL", offset); break;
    case OPCODE_LESS:          return display_one_byte_instruction("OPCODE_LESS", offset);          break;
    case OPCODE_LESS_EQUAL:    return display_one_byte_instruction("OPCODE_LESS_EQUAL", offset);    break;

    case OPCODE_ADD:      return display_one_byte_instruction("OPCODE_ADD", offset);      break;
    case OPCODE_SUBTRACT: return display_one_byte_instruction("OPCODE_SUBTRACT", offset); break;
    case OPCODE_MULTIPLY: return display_one_byte_instruction("OPCODE_MULTIPLY", offset); break;
    case OPCODE_DIVIDE:   return display_one_byte_instruction("OPCODE_DIVIDE", offset);   break;

    case OPCODE_NOT:    return display_one_byte_instruction("OPCODE_NOT", offset);    break;
    case OPCODE_NEGATE: return display_one_byte_instruction("OPCODE_NEGATE", offset); break;

    case OPCODE_RETURN: return display_one_byte_instruction("OPCODE_RETURN", offset); break;

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

static inline size_t display_two_byte_instruction(const char *instruction_name, const struct Value value, const size_t offset) {
  printf("%s ", instruction_name);
  value_print(value);
  printf("\n");
  return offset + 2;
}

static inline size_t display_four_byte_instruction(const char *instruction_name, const struct Value value, const size_t offset) {
  printf("%s ", instruction_name);
  value_print(value);
  printf("\n");
  return offset + 4;
}