#ifndef SEAL_VISITOR_H
#define SEAL_VISITOR_H

#include "seal.h"

#include "ast.h"
#include "scope.h"
#include "list.h"
#include "gc.h"
#include "libseal.h"

#define SEAL_MAX_STATE_SIZE 4

typedef struct state state_t;

typedef struct visitor {
  gc_t gc;
  list_t* func_defs;
  size_t func_size;
  list_t* struct_defs;
  size_t struct_size;
  int func_call_size;
  libseal_t** libseals;
  size_t libseal_size;
  state_t* states[SEAL_MAX_STATE_SIZE];
  size_t state_size;
  list_t* ext_func_defs;
} visitor_t;

static inline void init_visitor(visitor_t* visitor)
{
  visitor->func_defs      = NULL;
  visitor->func_size      = 0;
  visitor->struct_defs    = NULL;
  visitor->struct_size    = 0;
  visitor->func_call_size = 0;
  visitor->gc.tracked     = NULL;
  visitor->gc.ret_tracked = NULL;
  visitor->libseals       = NULL;
  visitor->libseal_size   = 0;
  visitor->state_size     = 0;
}

/* main function */
ast_t* visitor_visit(visitor_t*, scope_t*, ast_t*);

/* datas */
static ast_t* visitor_visit_list_lit(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_object_lit(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_var_ref(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_func_call(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_constructor(ast_t* constructor, visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_subscript(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_memacc(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_lib_func_call(visitor_t*, scope_t*, ast_t*);
/* blocks */
static ast_t* visitor_visit_comp(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_var_def(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_if(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_else(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_dowhile(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_while(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_for(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_func_def(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_struct_def(visitor_t*, scope_t*, ast_t*);
/* block control */
static ast_t* visitor_visit_return(visitor_t*, scope_t*, ast_t*);
/* operations */
static ast_t* visitor_visit_unary(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_binary(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_binary_bool(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_ternary(visitor_t*, scope_t*, ast_t*);
static ast_t* visitor_visit_assign(visitor_t*, scope_t*, ast_t*);
/* others */
static ast_t* visitor_visit_include(visitor_t*, scope_t*, ast_t*);

#endif
