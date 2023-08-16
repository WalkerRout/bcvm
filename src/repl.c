
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "vm.h"

// file local prototypes
static char *read_file(const char *file_path);

void repl_run(void) {
  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin) ||
        strncmp(line, "exit", 4) == 0) {
      printf("\n");
      break;
    }

    vm_interpret(line);
  }
}

void repl_run_file(const char *file_path) {
  const char *source = read_file(file_path);
  enum InterpretResult result = vm_interpret(source);
  free(source);

  if (result == INTERPRET_RESULT_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RESULT_RUNTIME_ERROR) exit(70);
}

// file local functions

static char *read_file(const char *file_path) {
  FILE *f = fopen(file_path, "rb");

  fseek(f, 0L, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  char *buffer = (char *) malloc(file_size + 1);
  size_t bytes_read = fread(buffer, sizeof(char), file_size, f);
  buffer[bytes_read] = '\0';

  fclose(f);
  return buffer;
}