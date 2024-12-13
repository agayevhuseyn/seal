#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include <stdio.h>

typedef struct {
  const char* name;
  void* handle;
  ast_t* (*(*functions)) (ast_t** args, size_t arg_size);
  const char** function_names;
  size_t function_size;
} module_t;

module_t* init_module(const char* name, bool has_alias, const char* alias_name);
ast_t* module_function_call(module_t* module, const char* func_name, ast_t** args, size_t arg_size);

#endif
