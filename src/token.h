#ifndef SEAL_TOKEN_H
#define SEAL_TOKEN_H

#include "sealconf.h"

#define TOK_EOF     0    /* "end of file" */
/* keywords */
#define TOK_IF      1    // if
#define TOK_THEN    2    // then
#define TOK_ELSE    3    // else
#define TOK_DO      4    // do
#define TOK_WHILE   5    // while
#define TOK_FOR     6    // for
#define TOK_IN      7    // in
#define TOK_TO      8    // to
#define TOK_BY      9    // by
#define TOK_SKIP    10   // skip
#define TOK_STOP    11   // stop
#define TOK_INCLUDE 12   // include
#define TOK_AS      13   // as
#define TOK_DEFINE  14   // define
#define TOK_RETURN  15   // return
#define TOK_TYPEOF  16   // typeof
#define TOK_AND     17   // and
#define TOK_OR      18   // or
#define TOK_NOT     19   // not
#define TOK_TRUE    20   // true
#define TOK_FALSE   21   // false
#define TOK_NULL    22   // null
#define TOK_ID      23   // identifier
#define TOK_INT     24   // integer value
#define TOK_FLOAT   25   // floating value
#define TOK_STRING  26   // string value
/* non-words */
#define TOK_DOLLAR  27   // $
#define TOK_PERIOD  28   // .
#define TOK_DPERIOD 29   // ..
#define TOK_COLON   30   // :
#define TOK_COMMA   31   // ,
#define TOK_PLUS    32   // +
#define TOK_MINUS   33   // -
#define TOK_MUL     34   // *
#define TOK_DIV     35   // /
#define TOK_MOD     36   // %
#define TOK_BAND    37   // &
#define TOK_BOR     38   // |
#define TOK_XOR     39   // ^
#define TOK_SHL     40   // <<
#define TOK_SHR     41   // >>
#define TOK_BNOT    42   // ~
#define TOK_INC     43   // ++
#define TOK_DEC     44   // ++
#define TOK_ASSIGN  45   // =
#define TOK_PLUS_ASSIGN   46   // +=
#define TOK_MINUS_ASSIGN  47   // -=
#define TOK_MUL_ASSIGN    48   // *=
#define TOK_DIV_ASSIGN    49   // /=
#define TOK_MOD_ASSIGN    50   // %=
#define TOK_BAND_ASSIGN   51   // &=
#define TOK_BOR_ASSIGN    52   // |=
#define TOK_XOR_ASSIGN    53   // ^=
#define TOK_SHL_ASSIGN    54   // <<=
#define TOK_SHR_ASSIGN    55   // >>=
#define TOK_EQ      56   // ==
#define TOK_NE      57   // !=
#define TOK_GT      58   // >
#define TOK_GE      59   // >=
#define TOK_LT      60   // <
#define TOK_LE      61   // <=
/* parenthesis */
#define TOK_LPAREN  62   // (
#define TOK_RPAREN  63   // )
#define TOK_LBRACK  64   // [
#define TOK_RBRACK  65   // ]
#define TOK_LBRACE  66   // {
#define TOK_RBRACE  67   // }
/* block-related */
#define TOK_NEWL    68   // \n
#define TOK_INDENT  69   // \t or \s
#define TOK_DEDENT  70   // <-\t or \s

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
    case TOK_AS     : return "TOK_AS";
    case TOK_DEFINE : return "TOK_DEFINE";
    case TOK_RETURN : return "TOK_RETURN";
    case TOK_TYPEOF : return "TOK_TYPEOF";
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
    case TOK_DOLLAR : return "TOK_DOLLAR";
    case TOK_PERIOD : return "TOK_PERIOD";
    case TOK_DPERIOD: return "TOK_DPERIOD";
    case TOK_COLON  : return "TOK_COLON";
    case TOK_COMMA  : return "TOK_COMMA";
    case TOK_PLUS   : return "TOK_PLUS";
    case TOK_MINUS  : return "TOK_MINUS";
    case TOK_MUL    : return "TOK_MUL";
    case TOK_DIV    : return "TOK_DIV";
    case TOK_MOD    : return "TOK_MOD";
    case TOK_BAND   : return "TOK_BAND";
    case TOK_BOR    : return "TOK_BOR";
    case TOK_XOR    : return "TOK_XOR";
    case TOK_SHL    : return "TOK_SHL";
    case TOK_SHR    : return "TOK_SHR";
    case TOK_BNOT   : return "TOK_BNOT";
    case TOK_INC    : return "TOK_INC";
    case TOK_DEC    : return "TOK_DEC";
    case TOK_ASSIGN : return "TOK_ASSIGN";
    case TOK_PLUS_ASSIGN  : return "TOK_PLUS_ASSIGN";
    case TOK_MINUS_ASSIGN : return "TOK_MINUS_ASSIGN";
    case TOK_MUL_ASSIGN   : return "TOK_MUL_ASSIGN";
    case TOK_DIV_ASSIGN   : return "TOK_DIV_ASSIGN";
    case TOK_MOD_ASSIGN   : return "TOK_MOD_ASSIGN";
    case TOK_BAND_ASSIGN  : return "TOK_BAND_ASSIGN";
    case TOK_BOR_ASSIGN   : return "TOK_BOR_ASSIGN";
    case TOK_XOR_ASSIGN   : return "TOK_XOR_ASSIGN";
    case TOK_SHL_ASSIGN   : return "TOK_SHL_ASSIGN";
    case TOK_SHR_ASSIGN   : return "TOK_SHR_ASSIGN";
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
    case TOK_AS     : return "as";
    case TOK_DEFINE : return "define";
    case TOK_RETURN : return "return";
    case TOK_TYPEOF : return "typeof";
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
    case TOK_DOLLAR : return "$";
    case TOK_PERIOD : return ".";
    case TOK_DPERIOD: return "..";
    case TOK_COLON  : return ":";
    case TOK_COMMA  : return ",";
    case TOK_PLUS   : return "+";
    case TOK_MINUS  : return "-";
    case TOK_MUL    : return "*";
    case TOK_DIV    : return "/";
    case TOK_MOD    : return "%";
    case TOK_BAND   : return "&";
    case TOK_BOR    : return "|";
    case TOK_XOR    : return "^";
    case TOK_SHL    : return "<<";
    case TOK_SHR    : return ">>";
    case TOK_BNOT   : return "~";
    case TOK_INC    : return "++";
    case TOK_DEC    : return "--";
    case TOK_ASSIGN : return "=";
    case TOK_PLUS_ASSIGN  : return "+=";
    case TOK_MINUS_ASSIGN : return "-=";
    case TOK_MUL_ASSIGN   : return "*=";
    case TOK_DIV_ASSIGN   : return "/=";
    case TOK_MOD_ASSIGN   : return "%=";
    case TOK_BAND_ASSIGN  : return "&=";
    case TOK_BOR_ASSIGN   : return "|=";
    case TOK_XOR_ASSIGN   : return "^=";
    case TOK_SHL_ASSIGN   : return "<<=";
    case TOK_SHR_ASSIGN   : return ">>=";
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

#endif /* SEAL_TOKEN_H */
