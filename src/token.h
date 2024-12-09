#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>

typedef enum {
  // ids
  TOK_VAR, TOK_ID, TOK_OBJECT, TOK_DEFINE, TOK_INCLUDE,
  // parens
  TOK_LPAREN, TOK_RPAREN, TOK_LBRACK, TOK_RBRACK, TOK_LBRACE, TOK_RBRACE,
  // op
  TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_MOD,
  // assignment
  TOK_ASSIGN,
  // compare
  TOK_EQ, TOK_NE, TOK_GT, TOK_GE, TOK_LT, TOK_LE,
  // logical
  TOK_AND, TOK_OR, TOK_NOT,
  // literals
  TOK_INT, TOK_FLOAT, TOK_STRING, TOK_TRUE, TOK_FALSE, TOK_NULL,
  // misc
  TOK_DOT, TOK_COMMA, TOK_COLON, TOK_DCOLON, TOK_IN,
  // blocks
  TOK_NEWL, TOK_INDENT, TOK_DEDENT, TOK_SEMI, TOK_IF, TOK_ELSE, TOK_WHILE,
  // flow op
  TOK_RETURN, TOK_SKIP, TOK_STOP,
  // eof
  TOK_EOF
} Token_Type;

typedef struct {
  Token_Type type;
  char* value;
  unsigned line;
} token_t;

const char* token_name(Token_Type type);
void print_tokens(token_t** tokens, size_t size);
token_t* init_token(Token_Type type, char* value, unsigned line);

#endif
