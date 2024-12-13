#include "token.h"
#include <stdio.h>

const char* token_name(Token_Type type)
{
  switch (type) {
    // ids
    case TOK_VAR: return "TOK_VAR";
    case TOK_ID: return "TOK_ID";
    case TOK_OBJECT: return "TOK_OBJECT";
    case TOK_DEFINE: return "TOK_DEFINE";
    case TOK_INCLUDE: return "TOK_INCLUDE";
    case TOK_AS: return "TOK_AS";
    // parens
    case TOK_LPAREN: return "TOK_LPAREN";
    case TOK_RPAREN: return "TOK_RPAREN";
    case TOK_LBRACK: return "TOK_LBRACK";
    case TOK_RBRACK: return "TOK_RBRACK";
    case TOK_LBRACE: return "TOK_LBRACE";
    case TOK_RBRACE: return "TOK_RBRACE";
    // op
    case TOK_PLUS: return "TOK_PLUS";
    case TOK_MINUS: return "TOK_MINUS";
    case TOK_MUL: return "TOK_MUL";
    case TOK_DIV: return "TOK_DIV";
    case TOK_MOD: return "TOK_MOD";
    // assignment 
    case TOK_ASSIGN: return "TOK_ASSIGN";
    // compare
    case TOK_EQ: return "TOK_EQ";
    case TOK_NE: return "TOK_NE";
    case TOK_GT: return "TOK_GT";
    case TOK_GE: return "TOK_GE";
    case TOK_LT: return "TOK_LT";
    case TOK_LE: return "TOK_LE";
    // logical
    case TOK_AND: return "TOK_AND";
    case TOK_OR: return "TOK_OR";
    case TOK_NOT: return "TOK_NOT";
    // literals
    case TOK_INT: return "TOK_INT";
    case TOK_FLOAT: return "TOK_FLOAT";
    case TOK_STRING: return "TOK_STRING";
    case TOK_TRUE: return "TOK_TRUE";
    case TOK_FALSE: return "TOK_FALSE";
    case TOK_NULL: return "TOK_NULL";
    // misc
    case TOK_DOT: return "TOK_DOT";
    case TOK_COMMA: return "TOK_COMMA";
    case TOK_COLON: return "TOK_COLON";
    case TOK_DCOLON: return "TOK_DCOLON";
    case TOK_IN: return "TOK_IN";
    // blocks
    case TOK_NEWL: return "TOK_NEWL";
    case TOK_INDENT: return "TOK_INDENT";
    case TOK_DEDENT: return "TOK_DEDENT";
    case TOK_SEMI: return "TOK_SEMI";
    case TOK_IF: return "TOK_IF";
    case TOK_ELSE: return "TOK_ELSE";
    case TOK_WHILE: return "TOK_WHILE";
    case TOK_FOR: return "TOK_FOR";
    // flow op
    case TOK_RETURN: return "TOK_RETURN";
    case TOK_SKIP: return "TOK_SKIP";
    case TOK_STOP: return "TOK_STOP";
    // eof
    case TOK_EOF: return "TOK_EOF";
  }
}

void print_tokens(token_t** tokens, size_t size)
{
  for (int i = 0; i < size; i++) {
    printf("%s, value: %s, line: %u\n", token_name(tokens[i]->type), tokens[i]->value, tokens[i]->line);
  }
}

token_t* init_token(Token_Type type, char* value, unsigned line)
{
  token_t* tok = calloc(1, sizeof(token_t));

  tok->type = type;
  tok->value = value;
  tok->line = line;

  return tok;
}
