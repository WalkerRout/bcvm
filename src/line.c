
#include <stdio.h>

#include "line.h"
#include "memory.h"

void line_array_init(struct LineArray *self) {
  self->line_struct_count = 0;
  self->line_struct_capacity = 0;
  self->lines = NULL;
}

void line_array_free(struct LineArray *self) {
  MEMORY_FREE_ARRAY(struct Line, self->lines, self->line_struct_capacity);

  line_array_init(self);
}

void line_array_write(struct LineArray *self, const size_t line) {
  size_t initial_line_struct_count = self->line_struct_count;
  size_t initial_line_struct_capacity = self->line_struct_capacity;

  size_t previous_line = 0;
  // if a line has already been initialized...
  if(initial_line_struct_count > 0) {
    previous_line = self->lines[self->line_struct_count-1].line;
  }

  // resize the backing buffer used for line debug storage
  if(initial_line_struct_capacity < initial_line_struct_count + 1) {
    self->line_struct_capacity = MEMORY_GROW_CAPACITY(initial_line_struct_capacity, CHUNK_LINE_INITIAL_CAPACITY);
    self->lines = MEMORY_GROW_ARRAY(struct Line, self->lines, initial_line_struct_capacity, self->line_struct_capacity);
  }

  // check if line is same as previous, otherwise write a new line
  if(line == previous_line && previous_line != 0) {
    // access current line structure
    self->lines[initial_line_struct_count-1].line_count += 1;
  } else {
    self->lines[initial_line_struct_count] = (struct Line) { .line = line, .line_count = 1 };
    self->line_struct_count += 1;
  }
}