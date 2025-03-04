#ifndef SEAL_TOKEN_H
#define SEAL_TOKEN_H

#include "seal.h"

#define TOK_EOF     0    // "end of file";
/* keywords */
#define TOK_VAR     1    // var
#define TOK_CONST   2    // const
#define TOK_IF      3    // if
#define TOK_THEN    4    // then
#define TOK_ELSE    5    // else
#define TOK_DO      6    // do
#define TOK_WHILE   7    // while
#define TOK_FOR     8    // for
#define TOK_IN      9    // in
#define TOK_TO      10   // to
#define TOK_BY      11   // by
#define TOK_SKIP    12   // skip
#define TOK_STOP    13   // stop
#define TOK_INCLUDE 14   // include
#define TOK_EXTERN  15   // extern
#define TOK_AS      16   // as
#define TOK_DEFINE  17   // define
#define TOK_RETURN  18   // return
#define TOK_STRUCT  19   // struct
#define TOK_AND     20   // and
#define TOK_OR      21   // or
#define TOK_NOT     22   // not
#define TOK_TRUE    23   // true
#define TOK_FALSE   24   // false
#define TOK_NULL    25   // null
#define TOK_ID      26   // identifier
#define TOK_INT     27   // integer value
#define TOK_FLOAT   28   // floating value
#define TOK_STRING  29   // string value
/* non-words */
#define TOK_DOT     30   // .
#define TOK_COLON   31   // :
#define TOK_COMMA   32   // ,
#define TOK_PLUS    33   // +
#define TOK_MINUS   34   // -
#define TOK_MUL     35   // *
#define TOK_DIV     36   // /
#define TOK_MOD     37   // %
#define TOK_POW     38   // ^
#define TOK_ASSIGN  39   // =
#define TOK_EQ      40   // ==
#define TOK_NE      41   // !=
#define TOK_GT      42   // >
#define TOK_GE      43   // >=
#define TOK_LT      44   // <
#define TOK_LE      45   // <=
/* parenthesis */
#define TOK_LPAREN  46   // (
#define TOK_RPAREN  47   // )
#define TOK_LBRACK  48   // [
#define TOK_RBRACK  49   // ]
#define TOK_LBRACE  50   // {
#define TOK_RBRACE  51   // }
/* block-related */
#define TOK_NEWL    52   // \n
#define TOK_INDENT  53   // \t
#define TOK_DEDENT  54   // 

#define TOK_LAST TOK_DEDENT    // last token

#define TOK_SIZE TOK_LAST + 1  // number of tokens

typedef struct {
  int type;
  const char* val;
  int line;
} token_t;

static inline const char* token_type_name(int type)
{
  switch (type) {
    case TOK_EOF    : return "TOK_EOF";
    case TOK_VAR    : return "TOK_VAR";
    case TOK_CONST  : return "TOK_CONST";
    case TOK_IF     : return "TOK_IF";
    case TOK_THEN   : return "TOK_THEN";
    case TOK_ELSE   : return "TOK_ELSE";
    case TOK_DO     : return "TOK_DO";
    case TOK_WHILE  : return "TOK_WHILE";
    case TOK_FOR    : return "TOK_FOR";
    case TOK_IN     : return "TOK_IN";
    case TOK_TO     : return "TOK_TO";
    case TOK_BY     : return "TOK_BY";
    case TOK_SKIP   : return "TOK_SKIP";
    case TOK_STOP   : return "TOK_STOP";
    case TOK_INCLUDE: return "TOK_INCLUDE";
    case TOK_EXTERN : return "TOK_EXTERN";
    case TOK_AS     : return "TOK_AS";
    case TOK_DEFINE : return "TOK_DEFINE";
    case TOK_RETURN : return "TOK_RETURN";
    case TOK_STRUCT : return "TOK_STRUCT";
    case TOK_AND    : return "TOK_AND";
    case TOK_OR     : return "TOK_OR";
    case TOK_NOT    : return "TOK_NOT";
    case TOK_TRUE   : return "TOK_TRUE";
    case TOK_FALSE  : return "TOK_FALSE";
    case TOK_NULL   : return "TOK_NULL";
    case TOK_ID     : return "TOK_ID";
    case TOK_INT    : return "TOK_INT";
    case TOK_FLOAT  : return "TOK_FLOAT";
    case TOK_STRING : return "TOK_STRING";
    case TOK_DOT    : return "TOK_DOT";
    case TOK_COLON  : return "TOK_COLON";
    case TOK_COMMA  : return "TOK_COMMA";
    case TOK_PLUS   : return "TOK_PLUS";
    case TOK_MINUS  : return "TOK_MINUS";
    case TOK_MUL    : return "TOK_MUL";
    case TOK_DIV    : return "TOK_DIV";
    case TOK_MOD    : return "TOK_MOD";
    case TOK_POW    : return "TOK_POW";
    case TOK_ASSIGN : return "TOK_ASSIGN";
    case TOK_EQ     : return "TOK_EQ";
    case TOK_NE     : return "TOK_NE";
    case TOK_GT     : return "TOK_GT";
    case TOK_GE     : return "TOK_GE";
    case TOK_LT     : return "TOK_LT";
    case TOK_LE     : return "TOK_LE";
    case TOK_LPAREN : return "TOK_LPAREN";
    case TOK_RPAREN : return "TOK_RPAREN";
    case TOK_LBRACK : return "TOK_LBRACK";
    case TOK_RBRACK : return "TOK_RBRACK";
    case TOK_LBRACE : return "TOK_LBRACE";
    case TOK_RBRACE : return "TOK_RBRACE";
    case TOK_NEWL   : return "TOK_NEWL";
    case TOK_INDENT : return "TOK_INDENT";
    case TOK_DEDENT : return "TOK_DEDENT";
    default         : return "TOKEN NOT RECOGNIZED";
  }
}

static inline const char* htoken_type_name(int type)
{
  switch (type) {
    case TOK_EOF    : return "end of file";
    case TOK_VAR    : return "var";
    case TOK_CONST  : return "const";
    case TOK_IF     : return "if";
    case TOK_THEN   : return "then";
    case TOK_ELSE   : return "else";
    case TOK_DO     : return "do";
    case TOK_WHILE  : return "while";
    case TOK_FOR    : return "for";
    case TOK_IN     : return "in";
    case TOK_TO     : return "to";
    case TOK_BY     : return "by";
    case TOK_SKIP   : return "skip";
    case TOK_STOP   : return "stop";
    case TOK_INCLUDE: return "include";
    case TOK_EXTERN : return "extern";
    case TOK_AS     : return "as";
    case TOK_DEFINE : return "define";
    case TOK_RETURN : return "return";
    case TOK_STRUCT : return "struct";
    case TOK_AND    : return "and";
    case TOK_OR     : return "or";
    case TOK_NOT    : return "not";
    case TOK_TRUE   : return "true";
    case TOK_FALSE  : return "false";
    case TOK_NULL   : return "null";
    case TOK_ID     : return "identifier";
    case TOK_INT    : return "<int value>";
    case TOK_FLOAT  : return "<float value>";
    case TOK_STRING : return "<string value>";
    case TOK_DOT    : return ".";
    case TOK_COLON  : return ":";
    case TOK_COMMA  : return ",";
    case TOK_PLUS   : return "+";
    case TOK_MINUS  : return "-";
    case TOK_MUL    : return "*";
    case TOK_DIV    : return "/";
    case TOK_MOD    : return "%";
    case TOK_POW    : return "^";
    case TOK_ASSIGN : return "=";
    case TOK_EQ     : return "==";
    case TOK_NE     : return "!=";
    case TOK_GT     : return ">";
    case TOK_GE     : return ">=";
    case TOK_LT     : return "<";
    case TOK_LE     : return "<=";
    case TOK_LPAREN : return "(";
    case TOK_RPAREN : return ")";
    case TOK_LBRACK : return "[";
    case TOK_RBRACK : return "]";
    case TOK_LBRACE : return "{";
    case TOK_RBRACE : return "}";
    case TOK_NEWL   : return "<newline>";
    case TOK_INDENT : return "<indentation>";
    case TOK_DEDENT : return "<dedentation>";
    default         : return "<TOKEN NAME NOT FOUND>";
  }
}

static inline token_t* create_token(int type, const char* val, int line)
{
  token_t* tok = (token_t*)SEAL_MALLOC(sizeof(token_t));

  tok->type = type;
  tok->val  = val == NULL ? htoken_type_name(type) : val;
  tok->line = line;

  return tok;
}

static void print_tokens(token_t** tokens, size_t token_size)
{
  printf("token size: %zu\ntokens:\n", token_size);
  for (int i = 0; i < token_size; i++) {
    token_t* tok = tokens[i];
    printf("%d, %s, %s\n", tok->line, htoken_type_name(tok->type), tok->val);
  }
}

#endif
