
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "vm.h"

enum LineStatus {
  LINE_STATUS_BREAK,
  LINE_STATUS_RESUME,
  LINE_STATUS_CONTINUE
};

// file local prototypes
static char *read_file(const char *file_path);
static enum LineStatus process_line(const char *line);

void repl_run(void) {
  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    enum LineStatus status = process_line(line);
    if (status == LINE_STATUS_CONTINUE)   continue;
    else if (status == LINE_STATUS_BREAK) break;
    else {} // resume

    vm_interpret(line);
  }
}

void repl_run_file(const char *file_path) {
  const char *source = read_file(file_path);
  enum InterpretResult result = vm_interpret(source);
  free((void *) source);

  if (result == INTERPRET_RESULT_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RESULT_RUNTIME_ERROR) exit(70);
}

// file local functions

static char *read_file(const char *file_path) {
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "Error - could not open file \"%s\".\n", file_path);
    exit(74);
  }

  fseek(f, 0L, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  char *buffer = (char *) malloc(file_size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Error - not enough memory to read \"%s\".\n", file_path);
    exit(74);
  }

  size_t bytes_read = fread(buffer, sizeof(char), file_size, f);
  if (bytes_read < file_size) {
    fprintf(stderr, "Error - could not read file \"%s\".\n", file_path);
    exit(74);
  }

  buffer[bytes_read] = '\0';

  fclose(f);
  return buffer;
}

static enum LineStatus process_line(const char *line) {
  if (strncmp(line, "!exit", 4) == 0) {
    printf("Goodbye!\n");
    return LINE_STATUS_BREAK;
  }

  if (strncmp(line, "!clear", 6) == 0) {
    puts("Clearing...\033[H\033[2J");
    return LINE_STATUS_CONTINUE;
  }

  return LINE_STATUS_RESUME;
}