#ifndef LINE_H
#define LINE_H

#include "common.h"

#define CHUNK_LINE_INITIAL_CAPACITY 4

struct Line {
  size_t line;
  size_t line_count;
};

struct LineArray {
  size_t line_struct_count;
  size_t line_struct_capacity;
  struct Line *lines;
};

void line_array_init(struct LineArray *self);
void line_array_free(struct LineArray *self);
void line_array_write(struct LineArray *self, const size_t line);

#endif // LINE_H