#include <stdio.h>

#include "chunk.h"
#include "value.h"
#include "opcode.h"
#include "debug.h"

int main(int argc, const char *argv[]) {
  struct Chunk chunk = {0};
  chunk_init(&chunk);

  size_t constant_two = chunk_add_constant(&chunk, 2.0);
  chunk_write(&chunk, OPCODE_CONSTANT, 123);
  chunk_write(&chunk, constant_two, 123);
  chunk_write(&chunk, OPCODE_RETURN, 123);

  debug_disassemble_chunk(&chunk, "test chunk");

  chunk_free(&chunk);

  return 0;
}
