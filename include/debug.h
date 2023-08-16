#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"
#include "chunk.h"

void debug_disassemble_chunk(struct Chunk *chunk, const char *message);
void debug_disassemble_value_array(struct ValueArray *value_array, const char *message);
size_t debug_disassemble_instruction(struct Chunk *chunk, const size_t offset);

#endif // DEBUG_H