#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>
#include "token.h"

typedef enum {
  AST_NOOP,
  AST_NULL,
  AST_COMP,
  AST_FUNC_CALL,
  AST_INT,
  AST_FLOAT,
  AST_STRING,
  AST_BOOL,
  AST_LIST,
  AST_OBJECT,
  AST_BINARY,
  AST_UNARY,
  AST_VARIABLE,
  AST_VAR_REF,
  AST_ASSIGN,
  AST_RETURN,
  AST_RETURN_VAL,
  AST_IF,
  AST_ELSE,
  AST_WHILE,
  AST_FOR,
  AST_SKIP,
  AST_STOP,
  AST_INCLUDE,

  AST_VARDEF,
  AST_FUNC_DEF,
  AST_OBJ_DEF,
  
  // unaries
  AST_MEM_ACC,
  AST_SUBSCRIPT,
  AST_MODULE_FCALL,
} AST_Type;

typedef struct ast {
  AST_Type type;

  union {
    struct {
      struct ast** stmts;
      size_t stmt_size;
    } comp;

    struct {
      const char* fname;
      struct ast** args;
      size_t arg_size;
    } func_call;

    struct {
      int val;
    } integer;

    struct {
      float val;
    } floating;

    struct {
      char* val;
    } string;

    struct {
      bool val;
    } boolean;

    struct {
      struct ast** mems;
      size_t mem_size;
    } list;

    struct {
      struct ast* def;
      struct ast** field_vars;
      size_t field_size;
    } object;

    struct {
      struct ast* left, *right;
      Token_Type op;
    } binary;

    struct {
      struct ast* expr;
      Token_Type op;
    } unary;

    struct {
      const char* name;
    } var_ref;

    /*
    struct {
      const char* name;
    } var_assign;
    */
    
    struct {
      const char* name;
      bool is_defined;
      struct ast* val;
    } variable;

    struct {
      const char** names;
      struct ast** rights;
      bool* is_defined_arr;
      size_t def_size;
    } vardef;

    struct {
      struct ast* var, *right;
    } assign;

    struct {
      struct ast* main, *mem;
    } mem_acc;

    struct {
      struct ast* main, *index;
    } subscript;

    struct {
      struct ast* module, *func_call;
    } module_fcall;

    struct {
      const char* fname;
      const char** args;
      size_t arg_size;
      struct ast* body;
    } func_def;

    struct {
      const char* oname;
      size_t field_size;
      const char** params;
      size_t param_size;

      struct field {
        const char* name;
        bool is_defined;
        struct ast* def;
      } **fields;
    } obj_def;

    struct {
      bool is_empty;
      struct ast* expr;
    } __return;

    struct {
      struct ast* val;
    } return_val;

    struct {
      struct ast* cond;
      struct ast* body;
      struct ast* else_part;
      bool has_else;
    } __if;

    struct {
      struct ast* body;
    } __else;

    struct {
      struct ast* cond;
      struct ast* body;
    } __while;

    struct {
      const char* name_iterator; // var i
      struct ast* iterated; // LIST OR STRING
      struct ast* body;
    } __for;

    struct {
      const char* module_name;
    } include;
  };
} ast_t;

void print_ast(ast_t*);
const char* ast_name(ast_t*);
ast_t* init_ast(AST_Type);
ast_t* ast_noop();
ast_t* ast_true();
ast_t* ast_false();
ast_t* ast_null();

#endif
