
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "compiler.h"
#include "scanner.h"
#include "opcode.h"
#include "debug.h"
#include "object.h"

struct Parser {
  struct Token current;
  struct Token previous;
  uint8_t had_error;
  uint8_t panic_mode;
};

enum Precedence {
  PRECEDENCE_NONE,
  PRECEDENCE_ASSIGNMENT,  // =
  PRECEDENCE_OR,          // or
  PRECEDENCE_AND,         // and
  PRECEDENCE_EQUALITY,    // == !=
  PRECEDENCE_COMPARISON,  // < > <= >=
  PRECEDENCE_TERM,        // + -
  PRECEDENCE_FACTOR,      // * /
  PRECEDENCE_UNARY,       // ! -
  PRECEDENCE_CALL,        // . ()
  PRECEDENCE_PRIMARY
};

struct ParseRule {
  void (*prefix)(void);
  void (*infix)(void);
  enum Precedence precedence;
};

// global singleton instance
static struct Parser global_parser = {0};

static struct Chunk *global_active_chunk = {0};

// file local prototypes
static void compiler_end_compile(void);
static void parser_init(void);
static void parser_advance(void);
static void parser_expression(void);
static void parser_expression_number(void);
static void parser_expression_string(void);
static void parser_expression_grouping(void);
static void parser_expression_unary(void);
static void parser_expression_binary(void);
static void parser_expression_literal(void);
static void parser_precedence(enum Precedence precedence);
static void parser_consume(enum TokenType, const char *error_message);
static void parser_error_at(struct Token *token, const char *error_message);
static void parser_error_at_current(const char *error_message);
static void parser_error_at_previous(const char *error_message);
static struct Chunk *current_chunk(void);
static void emit_byte(uint8_t byte);
static void emit_bytes(uint8_t byte_a, uint8_t byte_b);
static void emit_return(void);
static void emit_constant(struct Value value);
static uint8_t make_constant(struct Value value);
static struct ParseRule* get_rule(enum TokenType type);

struct ParseRule parser_rules[] = {
  [TOKEN_TYPE_LEFT_PAREN]    = {parser_expression_grouping, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_RIGHT_PAREN]   = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_LEFT_BRACE]    = {NULL, NULL, PRECEDENCE_NONE}, 
  [TOKEN_TYPE_RIGHT_BRACE]   = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_COMMA]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_DOT]           = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_MINUS]         = {parser_expression_unary, parser_expression_binary, PRECEDENCE_TERM},
  [TOKEN_TYPE_PLUS]          = {NULL, parser_expression_binary, PRECEDENCE_TERM},
  [TOKEN_TYPE_SEMICOLON]     = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_SLASH]         = {NULL, parser_expression_binary, PRECEDENCE_FACTOR},
  [TOKEN_TYPE_STAR]          = {NULL, parser_expression_binary, PRECEDENCE_FACTOR},
  [TOKEN_TYPE_BANG]          = {parser_expression_unary, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_BANG_EQUAL]    = {NULL, parser_expression_binary, PRECEDENCE_EQUALITY},
  [TOKEN_TYPE_EQUAL]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_EQUAL_EQUAL]   = {NULL, parser_expression_binary, PRECEDENCE_EQUALITY},
  [TOKEN_TYPE_GREATER]       = {NULL, parser_expression_binary, PRECEDENCE_COMPARISON},
  [TOKEN_TYPE_GREATER_EQUAL] = {NULL, parser_expression_binary, PRECEDENCE_COMPARISON},
  [TOKEN_TYPE_LESS]          = {NULL, parser_expression_binary, PRECEDENCE_COMPARISON},
  [TOKEN_TYPE_LESS_EQUAL]    = {NULL, parser_expression_binary, PRECEDENCE_COMPARISON},
  [TOKEN_TYPE_IDENTIFIER]    = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_STRING]        = {parser_expression_string, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_NUMBER]        = {parser_expression_number, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_AND]           = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_CLASS]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_ELSE]          = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_FALSE]         = {parser_expression_literal, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_FOR]           = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_FUN]           = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_IF]            = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_NIL]           = {parser_expression_literal, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_OR]            = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_PRINT]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_RETURN]        = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_SUPER]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_THIS]          = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_TRUE]          = {parser_expression_literal, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_VAR]           = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_WHILE]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_ERROR]         = {NULL, NULL, PRECEDENCE_NONE},
  [TOKEN_TYPE_EOF]           = {NULL, NULL, PRECEDENCE_NONE},
};

uint8_t compiler_compile(const char *source, struct Chunk *chunk) {
  scanner_init(source);
  global_active_chunk = chunk;

  parser_init();
  parser_expression();
  parser_consume(TOKEN_TYPE_EOF, "Error - expect end of expression");
  compiler_end_compile();

  return !global_parser.had_error;
}

// file local functions

static void compiler_end_compile(void) {
  emit_return();
#ifdef DEBUG_PRINT_CODE
  if (!global_parser.had_error) {
    debug_disassemble_chunk(current_chunk(), "code");
  }
#endif
}

static void parser_init(void) {
  global_parser.had_error  = FALSE;
  global_parser.panic_mode = FALSE;
  parser_advance();
}

static void parser_advance(void) {
  global_parser.previous = global_parser.current;

  for (;;) {
    global_parser.current = scanner_scan_token();
    if (global_parser.current.type != TOKEN_TYPE_ERROR) break;

    parser_error_at_current(global_parser.current.start);
  }
}

static void parser_expression(void) {
  parser_precedence(PRECEDENCE_ASSIGNMENT);
}

static void parser_expression_number(void) {
  double value = strtod(global_parser.previous.start, NULL);
  emit_constant(VALUE_NUMBER(value));
}

static void parser_expression_string(void) {
  emit_constant(
    VALUE_OBJECT(
      object_object_string_from_parts(
        global_parser.previous.start + 1,
        global_parser.previous.length - 2
      )
    )
  );
}

static void parser_expression_grouping(void) {
  parser_expression();
  parser_consume(TOKEN_TYPE_RIGHT_PAREN, "Error - expect ')' after expression");
}

static void parser_expression_unary(void) {
  enum TokenType ot = global_parser.previous.type;

  // compile operand
  parser_precedence(PRECEDENCE_UNARY);

  // emit operator instruction
  switch (ot) {
    case TOKEN_TYPE_BANG:  emit_byte(OPCODE_NOT);    break;
    case TOKEN_TYPE_MINUS: emit_byte(OPCODE_NEGATE); break;
    default: return; // unreachable
  }
}

static void parser_expression_binary(void) {
  enum TokenType ot = global_parser.previous.type;
  struct ParseRule *rule = get_rule(ot);
  parser_precedence((enum Precedence) (rule->precedence + 1));

  switch (ot) {
    case TOKEN_TYPE_BANG_EQUAL:    emit_byte(OPCODE_BANG_EQUAL);    break;
    case TOKEN_TYPE_EQUAL_EQUAL:   emit_byte(OPCODE_EQUAL_EQUAL);   break;
    case TOKEN_TYPE_GREATER:       emit_byte(OPCODE_GREATER);       break;
    case TOKEN_TYPE_GREATER_EQUAL: emit_byte(OPCODE_GREATER_EQUAL); break;
    case TOKEN_TYPE_LESS:          emit_byte(OPCODE_LESS);          break;
    case TOKEN_TYPE_LESS_EQUAL:    emit_byte(OPCODE_LESS_EQUAL);    break;

    case TOKEN_TYPE_PLUS:  emit_byte(OPCODE_ADD);      break;
    case TOKEN_TYPE_MINUS: emit_byte(OPCODE_SUBTRACT); break;
    case TOKEN_TYPE_STAR:  emit_byte(OPCODE_MULTIPLY); break;
    case TOKEN_TYPE_SLASH: emit_byte(OPCODE_DIVIDE);   break;
    default: return; // unreachable
  }
}

static void parser_expression_literal(void) {
  switch (global_parser.previous.type) {
    case TOKEN_TYPE_NIL:   emit_byte(OPCODE_NIL);   break;
    case TOKEN_TYPE_TRUE:  emit_byte(OPCODE_TRUE);  break;
    case TOKEN_TYPE_FALSE: emit_byte(OPCODE_FALSE); break;
    default: return; // unreachable
  }
}

static void parser_precedence(enum Precedence precedence) {
  parser_advance();
  void (*prefix_rule)(void) = get_rule(global_parser.previous.type)->prefix;

  if (prefix_rule == NULL) {
    parser_error_at_previous("Error - expect precedence expression");
    return;
  }

  prefix_rule();

  while (precedence <= get_rule(global_parser.current.type)->precedence) {
    parser_advance();
    void (*infix_rule)(void) = get_rule(global_parser.previous.type)->infix;
    infix_rule();
  }
}

static void parser_consume(enum TokenType type, const char *error_message) {
  if (global_parser.current.type == type) {
    parser_advance();
    return;
  }

  parser_error_at_current(error_message);
}

static void parser_error_at(struct Token *token, const char *error_message) {
  if (global_parser.panic_mode) return;
  global_parser.panic_mode = TRUE;

  fprintf(stderr, "[line %lu] Error ", token->line);

  if (token->type == TOKEN_TYPE_EOF) {
    fprintf(stderr, "at end");
  } else if (token->type == TOKEN_TYPE_ERROR) {
    {} // nothing
  } else {
    assert(token->length <= INT_MAX);
    fprintf(stderr, "at '%.*s'", (int) token->length, token->start);
  }

  fprintf(stderr, ": %s\n", error_message);
  global_parser.had_error = TRUE;
}

static void parser_error_at_current(const char *error_message) {
  parser_error_at(&global_parser.current, error_message);
}

static void parser_error_at_previous(const char *error_message) {
  parser_error_at(&global_parser.previous, error_message);
}

static struct Chunk *current_chunk(void) {
  return global_active_chunk;
}

static void emit_byte(uint8_t byte) {
  chunk_write(current_chunk(), byte, global_parser.previous.line);
}

static void emit_bytes(uint8_t byte_a, uint8_t byte_b) {
  emit_byte(byte_a);
  emit_byte(byte_b);
}

static void emit_return(void) {
  emit_byte(OPCODE_RETURN);
}

static void emit_constant(struct Value value) {
  emit_bytes(OPCODE_CONSTANT, make_constant(value));
}

static uint8_t make_constant(struct Value value) {
  size_t constant = chunk_add_constant(current_chunk(), value);
  if (constant > UINT8_MAX) {
    parser_error_at_previous("Error - too many constants in one chunk");
    return 0;
  }

  return (uint8_t) constant;
}

static struct ParseRule* get_rule(enum TokenType type) {
  return &parser_rules[type];
}