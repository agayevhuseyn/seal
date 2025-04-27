#ifndef SEAL_LEXER_H
#define SEAL_LEXER_H

#include "seal.h"

#include "token.h"

#define MAX_INDENT_LEVEL       128
#define MAX_NESTED_PAREN_LEVEL 256

typedef struct {
  int indent_stack[MAX_INDENT_LEVEL], paren_stack[MAX_NESTED_PAREN_LEVEL], paren_lines_stack[MAX_NESTED_PAREN_LEVEL];
  int indent_stack_ptr, paren_stack_ptr, paren_lines_ptr;
  const char* src;
  const char* file_path;
  size_t      src_size;
  int         i;
  int         line;
  token_t**   toks;
  int         tok_size;
  int         cur_indent;
  bool        encountered_word;
  bool        token_after_comment;
} lexer_t;

void init_lexer(lexer_t*, const char*);

/* main functions */
void lexer_get_tokens(lexer_t*);
static void lexer_get_token(lexer_t*);

/* char functions */
static inline char lexer_advance(lexer_t* lexer);
static inline char lexer_match(lexer_t* lexer, char c);

/* token functions */
static inline void lexer_add_token(lexer_t*, token_t*);
static token_t* lexer_get_id(lexer_t*);
static token_t* lexer_get_digit(lexer_t*, int lexeme_type);
static token_t* lexer_get_string(lexer_t*, int str_sur);
static char* lexer_get_lexeme(lexer_t*, int lexeme_type, int str_sur);

/* other functions */
static void lexer_ignore_comment(lexer_t*, int comment_type);

#endif /* SEAL_LEXER_H */
