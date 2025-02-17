#include "ast.h"

/* static and global declarations of constant nodes */
static ast_t* __ast_null  = NULL;
static ast_t* __ast_true  = NULL;
static ast_t* __ast_false = NULL;

inline void create_const_asts()
{
  __ast_null = static_create_ast(AST_NULL, -1);
  __ast_true = static_create_ast(AST_BOOL, -1);
  __ast_true->boolean.val = true;
  __ast_false = static_create_ast(AST_BOOL, -1);
  __ast_false->boolean.val = false;
}

inline ast_t* ast_null()
{
  return __ast_null;
}

inline ast_t* ast_true()
{
  return __ast_true;
}

inline ast_t* ast_false()
{
  return __ast_false;
}
