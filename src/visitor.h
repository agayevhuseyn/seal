#ifndef VISITOR_H
#define VISITOR_H

#include "ast.h"
#include "scope.h"
#include "parser.h"
#include "module.h"

typedef struct {
  scope_t*   g_scope;
  ast_t**  func_defs;
  size_t   func_size;
  ast_t**   obj_defs;
  size_t    obj_size;
  module_t** modules;
  size_t module_size;
} visitor_t;

visitor_t* init_visitor(parser_t*);

ast_t* visitor_visit(visitor_t*, scope_t*, ast_t*);

ast_t* visitor_visit_comp(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_func_call(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_int(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_string(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_float(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_bool(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_null(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_skip(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_stop(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_include(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_list(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_mem_acc(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_subscript(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_module_fcall(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_binary(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_unary(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_vardef(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_var_ref(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_assign(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_if(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_else(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_while(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_for(visitor_t*, scope_t*, ast_t*);
ast_t* visitor_visit_return(visitor_t*, scope_t*, ast_t*);

#endif
