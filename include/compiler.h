#ifndef COMPILER_H
#define COMPILER_H

#include "common.h"
#include "chunk.h"

uint8_t compiler_compile(const char *source, struct Chunk *chunk);

#endif // COMPILER_H