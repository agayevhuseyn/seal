#ifndef SCOPE_H
#define SCOPE_H

#include "ast.h"

typedef struct scope {
  ast_t** vars;
  size_t var_size;
  struct scope* prev_scope;
  bool is_global;
} scope_t;

ast_t* init_var(const char* name, ast_t* val, bool is_defined);
scope_t* init_scope();
void scope_add_var(scope_t*, ast_t*);
ast_t* scope_get_var(scope_t*, const char* name);
bool scope_is_var_declared(scope_t*, const char* name);
void print_scope(scope_t*);

#endif
