#ifndef SEAL_LIBDEF_H
#define SEAL_LIBDEF_H

#include "seal.h"

#include "ast.h"

#define sealobj ast_t
#define create_sealobj(TYPE) create_ast(TYPE) 
#define seal_type int

#define SEAL_NULL AST_NULL
#define SEAL_INT AST_INT
#define SEAL_FLOAT AST_FLOAT
#define SEAL_STRING AST_STRING
#define SEAL_BOOL AST_BOOL
#define SEAL_LIST AST_LIST
#define SEAL_OBJECT AST_OBJECT
#define SEAL_NUMBER (AST_LAST + 1)
#define SEAL_ITERABLE (AST_LAST + 2)

#define IS_SEAL_NULL(obj) (obj->type == SEAL_NULL)
#define IS_SEAL_INT(obj) (obj->type == SEAL_INT)
#define IS_SEAL_FLOAT(obj) (obj->type == SEAL_FLOAT)
#define IS_SEAL_STRING(obj) (obj->type == SEAL_STRING)
#define IS_SEAL_BOOL(obj) (obj->type == SEAL_BOOL)
#define IS_SEAL_LIST(obj) (obj->type == SEAL_LIST)
#define IS_SEAL_OBJECT(obj) (obj->type == SEAL_OBJECT)

#define IS_SEAL_NUMBER(obj) ( \
    obj->type == SEAL_INT || \
    obj->type == SEAL_FLOAT)

#define IS_SEAL_ITERABLE(obj) ( \
    obj->type == SEAL_STRING || \
    obj->type == SEAL_LIST)

void seal_func_err(const char* libname, const char* func_name, const char* err);

void seal_check_args(const char* libname,
                     const char* func_name,
                     seal_type expected_types[],
                     size_t type_size,
                     sealobj* args[],
                     size_t arg_size);

sealobj* get_obj_mem(sealobj* obj, const char* mem_name, seal_type type);

#endif
