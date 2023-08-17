
#include <stdio.h>

#include "line.h"
#include "memory.h"

void line_array_init(struct LineArray *line_array) {
  line_array->line_struct_count = 0;
  line_array->line_struct_capacity = 0;
  line_array->lines = NULL;
}

void line_array_free(struct LineArray *line_array) {
  MEMORY_FREE_ARRAY(struct Line, line_array->lines, line_array->line_struct_capacity);

  line_array_init(line_array);
}

void line_array_write(struct LineArray *line_array, const size_t line) {
  size_t initial_line_struct_count = line_array->line_struct_count;
  size_t initial_line_struct_capacity = line_array->line_struct_capacity;

  size_t previous_line = 0;
  // if a line has already been initialized...
  if(initial_line_struct_count > 0) {
    previous_line = line_array->lines[line_array->line_struct_count-1].line;
  }

  // resize the backing buffer used for line debug storage
  if(initial_line_struct_capacity < initial_line_struct_count + 1) {
    line_array->line_struct_capacity = MEMORY_GROW_CAPACITY(initial_line_struct_capacity, CHUNK_LINE_INITIAL_CAPACITY);
    line_array->lines = MEMORY_GROW_ARRAY(struct Line, line_array->lines, initial_line_struct_capacity, line_array->line_struct_capacity);
  }

  // check if line is same as previous, otherwise write a new line
  if(line == previous_line && previous_line != 0) {
    // access current line structure
    line_array->lines[initial_line_struct_count-1].line_count += 1;
  } else {
    line_array->lines[initial_line_struct_count] = (struct Line) { .line = line, .line_count = 1 };
    line_array->line_struct_count += 1;
  }
}