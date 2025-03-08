#ifndef SEAL_SCOPE_H
#define SEAL_SCOPE_H

#include "seal.h"

#include "ast.h"
#include "list.h"

typedef struct scope {
  struct scope* parent;
  list_t* var_list;
} scope_t;

static inline void scope_error(const char* err, int line)
{
  fprintf(stderr, "seal: line %d\nsyntax error: %s\n", line, err);
  exit(1);
}

static inline void init_scope(scope_t* self, scope_t* parent)
{
  self->var_list = NULL;
  self->parent = parent;
}

static inline int is_var_defined(scope_t* scope, const char* name)
{
  list_iterate(scope->var_list) {
    if (strcmp(it->val->variable.name, name) == 0) return it->val->line;
  }
  return 0;
}

static inline void scope_push_var(scope_t* scope, ast_t* var)
{
  const char* name = var->variable.name;
  int line;
  if ((line = is_var_defined(scope, name)) != 0) {
    char err[ERR_LEN];
    sprintf(err, "symbol \'%s\' already declared at line \'%d\'", name, line);
    scope_error(err, var->line);
  }
  // increment reference counter
  var->variable.val->ref_counter++;
  list_push(&scope->var_list, var);
}

static ast_t* scope_get_var(scope_t* scope, const char* name, int line)
{
  while (scope) {
    list_iterate(scope->var_list) {
      if (strcmp(it->val->variable.name, name) == 0) {
        return it->val;
      }
    }
    scope = scope->parent;
  }
  /*
  char err[ERR_LEN];
  sprintf(err, "\'%s\' is undefined", name);
  scope_error(err, line);
  return ast_null();
  */
  return NULL;
}

static ast_t* list_get_var(list_t* list, const char* name, int line)
{
  list_iterate(list) {
    if (strcmp(it->val->variable.name, name) == 0) {
      return it->val;
    }
  }
  /*
  char err[ERR_LEN];
  sprintf(err, "\'%s\' is undefined", name);
  scope_error(err, line);
  return ast_null();
  */
  return NULL;
}

#endif
