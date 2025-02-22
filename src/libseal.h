#ifndef SEAL_LIBSEAL_H
#define SEAL_LIBSEAL_H

#include "seal.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "ast.h"

typedef struct {
  const char* name;
  void* handle;
  ast_t* (*(*funcs)) (ast_t**, size_t);
  const char** func_names;
  size_t func_size;
} libseal_t;

libseal_t* create_libseal(const char* name, bool has_alias, const char* alias_name);
ast_t* libseal_function_call(libseal_t* lib, const char* func_name, ast_t** args, size_t arg_size);

#endif
