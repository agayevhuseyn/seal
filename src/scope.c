#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scope.h"

static inline void scope_error(const char* msg) {
  printf("Scope-> Error: %s\n", msg);
  exit(1);
}

ast_t* init_var(const char* name, ast_t* val, bool is_defined)
{
  ast_t* ast = init_ast(AST_VARIABLE);
  ast->variable.name = name;
  ast->variable.is_defined = is_defined;
  if (is_defined) {
    ast_t* copied = (void*)0;
    switch (val->type) {
      case AST_INT:
        copied = init_ast(AST_INT);
        copied->integer.val = val->integer.val;
        break;
      case AST_FLOAT:
        copied = init_ast(AST_FLOAT);
        copied->floating.val = val->floating.val;
        break;
      default:
        copied = val;
    }
    ast->variable.val = copied;
    //ast->variable.val = val;
  } else {
    ast->variable.val = ast_null();
  }
  return ast;
}

scope_t* init_scope()
{
  scope_t* scope = calloc(1, sizeof(scope_t));

  scope->vars = (void*)0;
  scope->var_size = 0;
  scope->prev_scope = (void*)0;
  scope->is_global = false;

  return scope;
}

void scope_add_var(scope_t* scope, ast_t* var)
{
  if (scope_is_var_declared(scope, var->variable.name)) {
    char msg[128];
    sprintf(msg, "the symbol \"%s\" has already been declared", var->variable.name);
    scope_error(msg);
  }

  scope->var_size++;
  scope->vars = realloc(scope->vars, scope->var_size * sizeof(ast_t*));
  scope->vars[scope->var_size - 1] = var;
  var->variable.val->ref_counter++;
}

ast_t* scope_get_var(scope_t* scope, const char* name)
{
  while (scope) {
    for (int i = 0; i < scope->var_size; i++) {
      if (strcmp(scope->vars[i]->variable.name, name) == 0) {
/*    
      if (!scope->vars[i]->is_defined) {
          char msg[128];
          sprintf(msg, "\"%s\" is not defined", name);
          scope_error(msg);
        }
        */
        return scope->vars[i];
      }
    }
    scope = scope->prev_scope;
  }
  char msg[128];
  sprintf(msg, "\"%s\" is not declared", name);
  scope_error(msg);
  return (void*)0;
}

bool scope_is_var_declared(scope_t* scope, const char* name)
{
  for (int i = 0; i < scope->var_size; i++) {
    if (strcmp(scope->vars[i]->variable.name, name) == 0)
      return true;
  }
  return false;
}

void print_scope(scope_t* scope)
{
  for (int i = 0; i < scope->var_size; i++) {
    printf("var: name: %s, value:\n", scope->vars[i]->variable.name);
    printf("\t");
    if (!scope->vars[i]->variable.is_defined) {
      printf("undefined\n");
    } else {
      print_ast(scope->vars[i]->variable.val);
    }
  }
}
