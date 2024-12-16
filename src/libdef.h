#ifndef LIBDEF_H
#define LIBDEF_H

#include "ast.h"

#define sealobj ast_t
#define init_sealobj(TYPE) init_ast(TYPE)

typedef enum {
  SEAL_INT = AST_INT,
  SEAL_FLOAT = AST_FLOAT,
  SEAL_STRING = AST_STRING,
  SEAL_BOOL = AST_BOOL,
  SEAL_LIST = AST_LIST,
  SEAL_OBJECT = AST_OBJECT,
  SEAL_NUMBER,
  SEAL_ITERABLE,
  SEAL_DATA,
  SEAL_NOOP = AST_NOOP,
  SEAL_NULL = AST_NULL,
} seal_type;

const char* seal_type_name(seal_type type);

void seal_check_args(const char* libname,
                     const char* func_name,
                     seal_type* expected_types,
                     size_t type_size,
                     sealobj** args,
                     size_t arg_size);

sealobj* get_obj_mem(sealobj* obj, const char* mem_name);

#endif
