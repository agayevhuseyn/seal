#include "ast.h"

/* static and global declarations of constant nodes */
static ast_t* __ast_null  = NULL;
static ast_t* __ast_true  = NULL;
static ast_t* __ast_false = NULL;

/* typeof return values */
static ast_t* __typeof_int    = NULL;
static ast_t* __typeof_float  = NULL;
static ast_t* __typeof_string = NULL;
static ast_t* __typeof_bool   = NULL;
static ast_t* __typeof_list   = NULL;
static ast_t* __typeof_map    = NULL;
static ast_t* __typeof_null   = NULL;

inline void create_const_asts()
{
  __ast_null = static_create_ast(AST_NULL, -1);
  __ast_true = static_create_ast(AST_BOOL, -1);
  __ast_true->boolean.val = true;
  __ast_false = static_create_ast(AST_BOOL, -1);
  __ast_false->boolean.val = false;
}

inline void create_typeof_asts()
{
  __typeof_int = static_create_ast(AST_STRING, -1);
  __typeof_int->string.val = "int";

  __typeof_float = static_create_ast(AST_STRING, -1);
  __typeof_float->string.val = "float";

  __typeof_string = static_create_ast(AST_STRING, -1);
  __typeof_string->string.val = "string";

  __typeof_bool = static_create_ast(AST_STRING, -1);
  __typeof_bool->string.val = "bool";

  __typeof_list = static_create_ast(AST_STRING, -1);
  __typeof_list->string.val = "list";

  __typeof_map = static_create_ast(AST_STRING, -1);
  __typeof_map->string.val = "map";

  __typeof_null = static_create_ast(AST_STRING, -1);
  __typeof_null->string.val = "null";
}

inline ast_t* ast_null() { return __ast_null; }
inline ast_t* ast_true() { return __ast_true; }
inline ast_t* ast_false() { return __ast_false; }

inline ast_t* typeof_int() { return __typeof_int; }
inline ast_t* typeof_float() { return __typeof_float; }
inline ast_t* typeof_string() { return __typeof_string; }
inline ast_t* typeof_bool() { return __typeof_bool; }
inline ast_t* typeof_list() { return __typeof_list; }
inline ast_t* typeof_map() { return __typeof_map; }
inline ast_t* typeof_null() { return __typeof_null; }
