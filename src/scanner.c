
#include <stdio.h>
#include <string.h>

#include "scanner.h"

struct Scanner {
  const char *start;
  const char *current;
  size_t line;
};

// global singleton instance
static struct Scanner global_scanner = {0};

// file local prototypes
static uint8_t scanner_at_end(void);
static struct Token make_identifier(void);
static struct Token make_number(void);
static struct Token make_string(void);
static struct Token make_token(enum TokenType type);
static struct Token make_error(const char *error_message);
static char scanner_advance(void);
static uint8_t scanner_match(char expected);
static void scanner_skip_whitespace(void);
static char scanner_peek(void);
static char scanner_peek_next(void);
static struct Token make_string(void);
static uint8_t is_alpha(char c);
static uint8_t is_digit(char c);
static enum TokenType identifier_type(void);
static enum TokenType check_keyword(size_t start, size_t length, const char *rest, enum TokenType type);

void scanner_init(const char *source) {
  global_scanner.start = source;
  global_scanner.current = source;
  global_scanner.line = 1;
}

struct Token scanner_scan_token(void) {
  scanner_skip_whitespace();

  global_scanner.start = global_scanner.current;

  if (scanner_at_end()) return make_token(TOKEN_TYPE_EOF);
  
  char c = scanner_advance();

  if (is_alpha(c)) return make_identifier();
  if (is_digit(c)) return make_number();

  switch (c) {
    case '"': return make_string();
    case '(': return make_token(TOKEN_TYPE_LEFT_PAREN);
    case ')': return make_token(TOKEN_TYPE_RIGHT_PAREN);
    case '{': return make_token(TOKEN_TYPE_LEFT_BRACE);
    case '}': return make_token(TOKEN_TYPE_RIGHT_BRACE);
    case ';': return make_token(TOKEN_TYPE_SEMICOLON);
    case ',': return make_token(TOKEN_TYPE_COMMA);
    case '.': return make_token(TOKEN_TYPE_DOT);
    case '-': return make_token(TOKEN_TYPE_MINUS);
    case '+': return make_token(TOKEN_TYPE_PLUS);
    case '/': return make_token(TOKEN_TYPE_SLASH);
    case '*': return make_token(TOKEN_TYPE_STAR);
    case '!':
      return make_token(scanner_match('=')
        ? TOKEN_TYPE_BANG_EQUAL
        : TOKEN_TYPE_BANG);
    case '=':
      return make_token(scanner_match('=')
        ? TOKEN_TYPE_EQUAL_EQUAL
        : TOKEN_TYPE_EQUAL);
    case '<':
      return make_token(scanner_match('=')
        ? TOKEN_TYPE_LESS_EQUAL
        : TOKEN_TYPE_LESS);
    case '>':
      return make_token(scanner_match('=')
        ? TOKEN_TYPE_GREATER_EQUAL
        : TOKEN_TYPE_GREATER);
    default: {}
  }

  return make_error("Error - unexpected character");
}

// file local functions

static uint8_t scanner_at_end(void) {
  return *global_scanner.current == '\0';
}

static struct Token make_identifier(void) {
  while (is_alpha(scanner_peek()) || is_digit(scanner_peek())) 
    scanner_advance();
  return make_token(identifier_type());
}

static struct Token make_number(void) {
  while (is_digit(scanner_peek())) scanner_advance();

  // lex fractional part if it exists
  if (scanner_peek() == '.' && is_digit(scanner_peek_next())) {
    scanner_advance(); // consume decimal '.'
    while (is_digit(scanner_peek())) scanner_advance();
  }

  return make_token(TOKEN_TYPE_NUMBER);
}

static struct Token make_string(void) {
  while (scanner_peek() != '"' && !scanner_at_end()) {
    if (scanner_peek() == '\n') global_scanner.line += 1;
    scanner_advance();
  }

  if (scanner_at_end()) return make_error("Error - unterminated string literal");

  scanner_advance(); // closing quote '"'
  return make_token(TOKEN_TYPE_STRING);
}

static struct Token make_token(enum TokenType type) {
  struct Token token;
  token.type = type;
  token.start = global_scanner.start;
  token.length = (size_t) (global_scanner.current - global_scanner.start);
  token.line = global_scanner.line;
  return token;
}

static struct Token make_error(const char *error_message) {
  struct Token token;
  token.type = TOKEN_TYPE_ERROR;
  token.start = error_message;
  token.length = (size_t) strlen(error_message);
  token.line = global_scanner.line;
  return token;
}

static char scanner_advance(void) {
  global_scanner.current += 1;
  return global_scanner.current[-1];
}

static uint8_t scanner_match(char expected) {
  if (scanner_at_end()) return 0;
  if (*global_scanner.current != expected) return 0;

  global_scanner.current += 1;

  return 1;
}

static void scanner_skip_whitespace(void) {
  for (;;) {
    switch (scanner_peek()) {
      case ' ':
      case '\r':
      case '\t':
        scanner_advance();
        break;

      case '\n':
        global_scanner.line += 1;
        scanner_advance();
        break;

      // skip comments, treat as whitespace
      case '/':
        if (scanner_peek_next() == '/') {
          while (scanner_peek() != '\n' && !scanner_at_end()) scanner_advance();
        } else {
          return;
        }
        break;

      default: return;
    }
  }
}

static char scanner_peek(void) {
  return global_scanner.current[0];
}

static char scanner_peek_next(void) {
  if (scanner_at_end()) return '\0';
  return global_scanner.current[1];
}

static uint8_t is_alpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static uint8_t is_digit(char c) {
  return c >= '0' && c <= '9';
}

static enum TokenType identifier_type(void) {
  switch (global_scanner.start[0]) {
    case 'a': return check_keyword(1, 2, "nd",    TOKEN_TYPE_AND);
    case 'c': return check_keyword(1, 4, "lass",  TOKEN_TYPE_CLASS);
    case 'e': return check_keyword(1, 3, "lse",   TOKEN_TYPE_ELSE);
    case 'f': {
      if (global_scanner.current - global_scanner.start > 1) {
        switch (global_scanner.start[1]) {
          case 'a': return check_keyword(2, 3, "lse", TOKEN_TYPE_FALSE);
          case 'o': return check_keyword(2, 1, "r",   TOKEN_TYPE_FOR);
          case 'u': return check_keyword(2, 1, "n",   TOKEN_TYPE_FUN);
        }
      }
    } break;
    case 'i': return check_keyword(1, 1, "f",     TOKEN_TYPE_IF);
    case 'n': return check_keyword(1, 2, "il",    TOKEN_TYPE_NIL);
    case 'o': return check_keyword(1, 1, "r",     TOKEN_TYPE_OR);
    case 'p': return check_keyword(1, 4, "rint",  TOKEN_TYPE_PRINT);
    case 'r': return check_keyword(1, 5, "eturn", TOKEN_TYPE_RETURN);
    case 's': return check_keyword(1, 4, "uper",  TOKEN_TYPE_SUPER);
    case 't': {
      if (global_scanner.current - global_scanner.start > 1) {
        switch (global_scanner.start[1]) {
          case 'h': return check_keyword(2, 2, "is", TOKEN_TYPE_THIS);
          case 'r': return check_keyword(2, 2, "ue", TOKEN_TYPE_TRUE);
        }
      }
    } break;
    case 'v': return check_keyword(1, 2, "ar",    TOKEN_TYPE_VAR);
    case 'w': return check_keyword(1, 4, "hile",  TOKEN_TYPE_WHILE);
  }

  return TOKEN_TYPE_IDENTIFIER;
}

static enum TokenType check_keyword(size_t start, size_t length, const char *rest, enum TokenType type) {
  if ((size_t) (global_scanner.current - global_scanner.start) == start + length &&
      memcmp(global_scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_TYPE_IDENTIFIER;
}