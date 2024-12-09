#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
  token_t** tokens;
  size_t    token_size;
  unsigned  i;
  ast_t**   func_defs;
  size_t    func_size;
  ast_t**   obj_defs;
  size_t    obj_size;
} parser_t;

parser_t* init_parser(lexer_t*);

bool parser_is_end(parser_t*);
token_t* parser_peek(parser_t*);
token_t* parser_advance(parser_t*);
token_t* parser_eat(parser_t*, Token_Type);
token_t* parser_peek_offset(parser_t*, int);

ast_t* parser_parse(parser_t*);
ast_t* parser_parse_statements(parser_t*, bool is_func, bool is_ifelse, bool is_loop);
ast_t* parser_parse_statement(parser_t*, bool is_func, bool is_ifelse, bool is_loop);

ast_t* parser_parse_expr(parser_t*);
ast_t* parser_parse_or(parser_t*);
ast_t* parser_parse_and(parser_t*);
ast_t* parser_parse_equal(parser_t*);
ast_t* parser_parse_compare(parser_t*);
ast_t* parser_parse_term(parser_t*);
ast_t* parser_parse_factor(parser_t*);
ast_t* parser_parse_unary(parser_t*);
ast_t* parser_parse_primary(parser_t*);

ast_t* parser_parse_string(parser_t*);
ast_t* parser_parse_int(parser_t*);
ast_t* parser_parse_float(parser_t*);
ast_t* parser_parse_list(parser_t*);
ast_t* parser_parse_func_call(parser_t*);
ast_t* parser_parse_id(parser_t*);
ast_t* parser_parse_var(parser_t*);
ast_t* parser_parse_vardef(parser_t*);
ast_t* parser_parse_func_def(parser_t*);
ast_t* parser_parse_obj_def(parser_t*);
ast_t* parser_parse_return(parser_t*);
ast_t* parser_parse_skip(parser_t*);
ast_t* parser_parse_stop(parser_t*);
ast_t* parser_parse_include(parser_t*);
ast_t* parser_parse_if(parser_t*, bool is_func, bool is_loop);
ast_t* parser_parse_else(parser_t*, bool is_func, bool is_loop);
ast_t* parser_parse_while(parser_t*, bool is_func);

#endif
