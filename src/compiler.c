
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"

struct Parser {
  struct Token current;
  struct Token previous;
  uint8_t had_error;
};

// global singleton instance
static struct Parser global_parser = {0};

// file local prototypes
static void parser_advance(void);
static void parser_error_at(struct Token *token, const char *error_message);
static void parser_error_at_current(const char *error_message);
static void parser_error_at_previous(const char *error_message);

uint8_t compiler_compile(const char *source, struct Chunk *chunk) {
  scanner_init(source);
  parser_advance();
  //parser_expression();
  //parser_consume(TOKEN_TYPE_EOF, "Expect end of expression.");

  return !global_parser.had_error;
}

// file local functions

static void parser_advance(void) {
  global_parser.previous = global_parser.current;

  for (;;) {
    global_parser.current = scanner_scan_token();
    if (global_parser.current.type != TOKEN_TYPE_ERROR) break;

    parser_error_at_current(global_parser.current.start);
  }
}

static void parser_error_at(struct Token *token, const char *error_message) {
  fprintf(stderr, "[line %d] Error ", token->line);

  if (token->type == TOKEN_TYPE_EOF) {
    fprintf(stderr, "at end");
  } else if (token->type == TOKEN_TYPE_ERROR) {
    {} // nothing
  } else {
    fprintf(stderr, "at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", error_message);
  global_parser.had_error = true;
}

static void parser_error_at_current(const char *error_message) {
  parser_error_at(&global_parser.current, error_message);
}
static void parser_error_at_previous(const char *error_message) {
  parser_error_at(&global_parser.previous, error_message);
}