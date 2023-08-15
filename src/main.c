#include <stdio.h>

#include "vm.h"

#include "chunk.h"
#include "debug.h"
#include "opcode.h"

int main(int argc, const char *argv[]) {
  (void) argc;
  (void) argv;

  struct Chunk chunk = {0};
  chunk_init(&chunk);

  size_t constant_two   = chunk_add_constant(&chunk, 2.0);
  size_t constant_three = chunk_add_constant(&chunk, 3.0);
  size_t constant_four  = chunk_add_constant(&chunk, 4.0);
  size_t constant_five  = chunk_add_constant(&chunk, 5.0);

  chunk_write(&chunk, OPCODE_CONSTANT, 123);
  chunk_write(&chunk, constant_two, 123);

  chunk_write(&chunk, OPCODE_CONSTANT, 123);
  chunk_write(&chunk, constant_three, 123);

  chunk_write(&chunk, OPCODE_CONSTANT, 123);
  chunk_write(&chunk, constant_four, 123);

  chunk_write(&chunk, OPCODE_CONSTANT_LONG, 124);
  chunk_write(&chunk, 0, 124);
  chunk_write(&chunk, 0, 124);
  chunk_write(&chunk, constant_five, 124);

  chunk_write(&chunk, OPCODE_RETURN, 124);

  debug_disassemble_chunk(&chunk, "test chunk");

  vm_init();
  vm_interpret(&chunk);
  vm_free();

  chunk_free(&chunk);

  return 0;
}
