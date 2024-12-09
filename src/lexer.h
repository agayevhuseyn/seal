#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stdbool.h>

typedef struct {
  const char* src;
  size_t src_size;
  token_t** tokens;
  size_t token_size;
  unsigned i;
  unsigned line;
  int cur_indent, prev_indent;
  bool encountered_word;
} lexer_t;

lexer_t* init_lexer(const char* src);

void lexer_collect_tokens(lexer_t*);
void lexer_collect_token(lexer_t*);
char lexer_advance(lexer_t*);
char lexer_peek(lexer_t*);
char lexer_peek_offset(lexer_t*, int);
char lexer_eat(lexer_t*, char);
bool lexer_is_end(lexer_t*);
void lexer_add_token(lexer_t*, token_t*);
void lexer_get_id(lexer_t*);
void lexer_get_digit(lexer_t*, bool has_whole);
void lexer_get_string(lexer_t*);

#endif
