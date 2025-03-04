#ifndef SEAL_PARSER_H
#define SEAL_PARSER_H

#include "seal.h"

#include "lexer.h"
#include "ast.h"

typedef struct {
  token_t** toks;
  size_t tok_size;
  int i;
} parser_t;

void init_parser(parser_t*, lexer_t*);

/* main function */
ast_t* parser_parse(parser_t*);

/* parsing statement */
static ast_t* parser_parse_statements(parser_t* parser,
                                      bool is_func,
                                      bool is_ifelse,
                                      bool is_loop,
                                      bool is_inline);
static ast_t* parser_parse_statement(parser_t* parser,
                                     bool is_func,
                                     bool is_ifelse,
                                     bool is_loop,
                                     bool is_inline);
/* parse datas */
static inline ast_t* parser_parse_int(parser_t*);
static inline ast_t* parser_parse_float(parser_t*);
static inline ast_t* parser_parse_string(parser_t*);
static ast_t* parser_parse_list(parser_t*);
static ast_t* parser_parse_object(parser_t*);
static inline ast_t* parser_parse_id(parser_t*);
static inline ast_t* parser_parse_var_ref(parser_t*);
static ast_t* parser_parse_func_call(parser_t*);
/* parse blocks */
static ast_t* parser_parse_var_def(parser_t*);
static ast_t* parser_parse_if(parser_t*, bool can_be_ternary, bool is_func, bool is_loop);
static ast_t* parser_parse_else(parser_t*, bool is_func, bool is_loop);
static ast_t* parser_parse_dowhile(parser_t*, bool is_func);
static ast_t* parser_parse_while(parser_t*, bool is_func);
static ast_t* parser_parse_for(parser_t*, bool is_func);
static ast_t* parser_parse_func_def(parser_t*);
static ast_t* parser_parse_struct_def(parser_t*);
/* parse block control */
static inline ast_t* parser_parse_skip(parser_t*);
static inline ast_t* parser_parse_stop(parser_t*);
static inline ast_t* parser_parse_return(parser_t*);
/* parse operations & expressions */
static inline ast_t* parser_parse_expr(parser_t*);
static ast_t* parser_parse_ternary(parser_t*);
static ast_t* parser_parse_or(parser_t*);
static ast_t* parser_parse_and(parser_t*);
static ast_t* parser_parse_equal(parser_t*);
static ast_t* parser_parse_compare(parser_t*);
static ast_t* parser_parse_term(parser_t*);
static ast_t* parser_parse_factor(parser_t*);
static ast_t* parser_parse_pow(parser_t*);
static ast_t* parser_parse_unary(parser_t*);
static ast_t* parser_parse_primary(parser_t*);
/* parse others */
static ast_t* parser_parse_include(parser_t*);

#endif
