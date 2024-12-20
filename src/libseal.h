#ifndef LIBSEAL_H
#define LIBSEAL_H

#include "ast.h"
#include <stdio.h>

typedef struct {
  const char* name;
  void* handle;
  ast_t* (*(*functions)) (ast_t** args, size_t arg_size);
  const char** function_names;
  size_t function_size;
} libseal_t;

libseal_t* init_libseal(const char* name, bool has_alias, const char* alias_name);
ast_t* libseal_function_call(libseal_t* lib, const char* func_name, ast_t** args, size_t arg_size);

#endif
