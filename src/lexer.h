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
  size_t      src_size;
  int         i;
  int         line;
  token_t**   toks;
  int         tok_size;
  int         cur_indent;
  bool        encountered_word;
  bool        token_after_comment;
  bool        token_after_paren;
} lexer_t;

static inline void init_lexer(lexer_t* lexer, const char* src)
{
  lexer->src                 = src;
  lexer->src_size            = strlen(src);
  lexer->i                   = 0;
  lexer->line                = 1;
  lexer->toks                = NULL;
  lexer->tok_size            = 0;
  lexer->cur_indent          = 0;
  lexer->encountered_word    = false;
  lexer->token_after_comment = false;
  lexer->token_after_paren   = false;

  lexer->indent_stack_ptr = lexer->paren_stack_ptr = lexer->paren_lines_ptr = 0; // init to 0

  memset(lexer->indent_stack, -1, sizeof(lexer->indent_stack)); // unitialized
  lexer->indent_stack[lexer->indent_stack_ptr] = 0; // global indentation block

  memset(lexer->paren_stack, -1, sizeof(lexer->paren_stack)); // no nest
  memset(lexer->paren_lines_stack, -1, sizeof(lexer->paren_lines_stack)); // no nest
}

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

#endif
