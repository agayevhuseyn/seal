#ifndef SEAL_TYPES_H
#define SEAL_TYPES_H

#include "ast.h"

typedef ast_t sealobj;
#define create_sealobj(TYPE) create_ast(TYPE) 
typedef int seal_type;
#define seal_null() ast_null()
#define seal_true() ast_true()
#define seal_false() ast_false()
#define create_const_seal_objects() create_const_asts()

#define SEAL_NULL AST_NULL
#define SEAL_INT AST_INT
#define SEAL_FLOAT AST_FLOAT
#define SEAL_STRING AST_STRING
#define SEAL_BOOL AST_BOOL
#define SEAL_LIST AST_LIST
#define SEAL_OBJECT AST_OBJECT
#define SEAL_NUMBER   (AST_LAST + 1)
#define SEAL_ITERABLE (AST_LAST + 2)
#define SEAL_ANY      (AST_LAST + 3)

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

#define IS_SEAL_ANY(obj) ( \
    obj->type == SEAL_NULL || \
    obj->type == SEAL_INT || \
    obj->type == SEAL_FLOAT || \
    obj->type == SEAL_STRING || \
    obj->type == SEAL_BOOL || \
    obj->type == SEAL_LIST || \
    obj->type == SEAL_OBJECT)

static inline const char*
seal_type_name(seal_type type)
{
  switch (type) {
    case SEAL_NULL:     return "null";
    case SEAL_INT:      return "integer";
    case SEAL_FLOAT:    return "float";
    case SEAL_STRING:   return "string";
    case SEAL_BOOL:     return "bool";
    case SEAL_LIST:     return "list";
    case SEAL_OBJECT:   return "object";
    case SEAL_NUMBER:   return "number";
    case SEAL_ITERABLE: return "iterable";
    case SEAL_ANY:      return "any";
    default:            return "SEAL TYPE NOT RECOGNIZED";
  }
}

#endif
