// The two tokens a regular expression cannot lex:
//
// - `content`: the text between Jinja tags, up to the next `{{`, `{%` or
//   `{#`. It keeps its whitespace and may end in a lone `{`.
// - `raw_content`: everything inside `{% raw %}`, up to the tag that closes
//   it, `{%` `-`? `endraw` `-`? `%}` with any whitespace around the name.
//
// The scanner keeps no state.

#include "tree_sitter/parser.h"

#include <stdbool.h>

enum TokenType {
  CONTENT,
  RAW_CONTENT,
};

void *tree_sitter_dbt_external_scanner_create(void) { return NULL; }
void tree_sitter_dbt_external_scanner_destroy(void *payload) { (void)payload; }
unsigned tree_sitter_dbt_external_scanner_serialize(void *payload, char *buffer) {
  (void)payload;
  (void)buffer;
  return 0;
}
void tree_sitter_dbt_external_scanner_deserialize(void *payload, const char *buffer, unsigned length) {
  (void)payload;
  (void)buffer;
  (void)length;
}

static bool is_space(int32_t c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static bool scan_content(TSLexer *lexer) {
  bool has_content = false;
  while (!lexer->eof(lexer)) {
    if (lexer->lookahead == '{') {
      lexer->mark_end(lexer);
      lexer->advance(lexer, false);
      int32_t next = lexer->lookahead;
      if (next == '{' || next == '%' || next == '#') {
        lexer->result_symbol = CONTENT;
        return has_content;
      }
      has_content = true;
      continue;
    }
    lexer->advance(lexer, false);
    has_content = true;
  }
  lexer->mark_end(lexer);
  lexer->result_symbol = CONTENT;
  return has_content;
}

// Having read `{%`, reads the rest of an endraw tag. True when it was one.
static bool scan_endraw_rest(TSLexer *lexer) {
  if (lexer->lookahead == '-' || lexer->lookahead == '+') lexer->advance(lexer, false);
  while (is_space(lexer->lookahead)) lexer->advance(lexer, false);
  const char *word = "endraw";
  for (const char *c = word; *c; c++) {
    if (lexer->lookahead != *c) return false;
    lexer->advance(lexer, false);
  }
  while (is_space(lexer->lookahead)) lexer->advance(lexer, false);
  if (lexer->lookahead == '-' || lexer->lookahead == '+') lexer->advance(lexer, false);
  if (lexer->lookahead != '%') return false;
  lexer->advance(lexer, false);
  return lexer->lookahead == '}';
}

static bool scan_raw_content(TSLexer *lexer) {
  bool has_content = false;
  lexer->result_symbol = RAW_CONTENT;
  while (!lexer->eof(lexer)) {
    if (lexer->lookahead == '{') {
      lexer->mark_end(lexer);
      lexer->advance(lexer, false);
      if (lexer->lookahead == '%') {
        lexer->advance(lexer, false);
        if (scan_endraw_rest(lexer)) return has_content;
      }
      has_content = true;
      continue;
    }
    lexer->advance(lexer, false);
    has_content = true;
  }
  lexer->mark_end(lexer);
  return has_content;
}

bool tree_sitter_dbt_external_scanner_scan(void *payload, TSLexer *lexer, const bool *valid_symbols) {
  (void)payload;
  // In error recovery every symbol is valid; the two cannot both be, so let
  // the internal lexer recover.
  if (valid_symbols[CONTENT] && valid_symbols[RAW_CONTENT]) return false;
  if (valid_symbols[RAW_CONTENT]) return scan_raw_content(lexer);
  if (valid_symbols[CONTENT]) return scan_content(lexer);
  return false;
}
