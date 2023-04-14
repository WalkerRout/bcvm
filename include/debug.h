#ifndef DEBUG_H
#define DEBUG_H

#include "common.h"
#include "chunk.h"

void debug_disassemble_chunk(struct Chunk *chunk, const char *message);
void debug_disassemble_value_array(struct ValueArray *value_array);

#endif // DEBUG_H