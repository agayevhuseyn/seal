#include "visitor.h"
#include "gc.h"
#include "builtin.h"
#include "state.h"

/* for loop */
#define FOR_INT     0
#define FOR_STRING  1
#define FOR_LIST    2
#define FOR_NUMTO   3
#define FOR_NUMBY   4
#define FOR_NUMTOBY 5

static inline ast_t* visitor_error(visitor_t* visitor, ast_t* node, const char* err)
{
  fprintf(stderr, "seal: at line: %d\nerror: %s\n", node->line, err);
  exit(1);
  return ast_null();
}

static inline ast_t* search_func(list_t* list, const char* name)
{
  list_iterate(list) {
    if (strcmp(name, it->val->func_def.name) == 0) {
      return it->val;
    }
  }
  return NULL;
}

static inline ast_t* search_struct(list_t* list, const char* name)
{
  list_iterate(list) {
    if (strcmp(name, it->val->struct_def.name) == 0) {
      return it->val;
    }
  }
  return NULL;
}

static inline void kill_if_index_non_int(visitor_t* visitor, ast_t* index, ast_t* node)
{
  if (index->type == AST_INT) return;
  char err[ERR_LEN];
  sprintf(err, "indices must be integers");
  visitor_error(visitor, node, err);
}

static inline void kill_if_non_iterable(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_LIST || main->type == AST_STRING) return;
  char err[ERR_LEN];
  sprintf(err, "iterable value required");
  visitor_error(visitor, node, err);
}

static inline void kill_if_non_object(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_OBJECT) return;
  char err[ERR_LEN];
  sprintf(err, "object value required");
  visitor_error(visitor, node, err);
}

static inline void kill_if_non_list(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_LIST) return;
  char err[ERR_LEN];
  sprintf(err, "list value required");
  visitor_error(visitor, node, err);
}

static inline ast_t* get_object_mem(visitor_t* visitor, ast_t* main, ast_t* mem)
{
  kill_if_non_object(visitor, main, mem); // TODO change mem to main unvisited node later
  for (int i = 0; i < main->object.field_size; i++) {
    if (strcmp(mem->var_ref.name, main->object.field_names[i]) == 0) {
      return main->object.field_vals[i];
    }
  }
  char err[ERR_LEN];
  sprintf(err,
          "%s%sobject has no field named \'%s\'",
          !main->object.is_lit ? main->object.def->struct_def.name : "",
          !main->object.is_lit ? " type " : "",
          mem->var_ref.name);
  return visitor_error(visitor, mem, err);
}

static inline void kill_if_non_bool(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_BOOL) return;
  char err[ERR_LEN];
  sprintf(err, "boolean value required");
  visitor_error(visitor, node, err);
}

static inline void kill_if_non_number(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_INT || main->type == AST_FLOAT) return;
  char err[ERR_LEN];
  sprintf(err, "numerical value required");
  visitor_error(visitor, node, err);
}

static inline void kill_if_non_int(visitor_t* visitor, ast_t* main, ast_t* node)
{
  if (main->type == AST_INT) return;
  char err[ERR_LEN];
  sprintf(err, "integer value required");
  visitor_error(visitor, node, err);
}

static inline void kill_if_argsize_ne(visitor_t* visitor,
                                      ast_t* node,
                                      bool is_func,
                                      const char* name,
                                      size_t param_size,
                                      size_t arg_size)
{
  if (arg_size != param_size) { // arguments and parameters size must match
    char err[ERR_LEN];
    sprintf(err,
            "%s \'%s\' requires %zu argument%s, got %zu",
            is_func ? "function" : "constructor",
            name,
            param_size,
            param_size == 1 ? "" : "s",
            arg_size);
    visitor_error(visitor, node, err);
  }
}

static inline void binary_op_error(visitor_t* visitor, ast_t* node, const char* type, int op_type)
{
  char err[ERR_LEN];
  sprintf(err, "\'%s\' operator not supported for %ss", htoken_type_name(op_type), type);
  visitor_error(visitor, node, err);
}

static void track_library_returned_object(visitor_t* visitor, ast_t* node, bool is_main)
{
  switch (node->type) {
    case AST_INT:
    case AST_FLOAT:
    case AST_STRING:
      break;
    case AST_LIST:
      for (int i = 0; i < node->list.mem_size; i++) {
        track_library_returned_object(visitor, node->list.mems[i], false);
      }
      break;
    case AST_OBJECT:
      for (int i = 0; i < node->object.field_size; i++) {
        track_library_returned_object(visitor, node->object.field_vals[i], false);
      }
      break;
    default:
      return;
  }
  if (!is_main) {
    gc_track(visitor->gc, node);
    gc_retain(node);
  }
}

/* main function */
ast_t* visitor_visit(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  // printf("fsize : %d\n", visitor->func_call_size);
  ast_t* visited;
  switch (node->type) {
    case AST_NULL:
    case AST_INT:
    case AST_FLOAT:
    case AST_STRING:
    case AST_BOOL:
    case AST_LIST:
    case AST_OBJECT:
    case AST_SKIP:
    case AST_STOP:
      return node;
    case AST_RETURNED_VAL:
      return node->returned_val.val;
    case AST_LIST_LIT:
      visited = visitor_visit_list_lit(visitor, scope, node);
      break;
    case AST_OBJECT_LIT:
      visited = visitor_visit_object_lit(visitor, scope, node);
      break;
    case AST_VAR_REF:
      return visitor_visit_var_ref(visitor, scope, node);
    case AST_FUNC_CALL:
      return visitor_visit_func_call(visitor, scope, node);
    case AST_SUBSCRIPT:
      visited = visitor_visit_subscript(visitor, scope, node);
      break;
    case AST_MEMACC:
      visited = visitor_visit_memacc(visitor, scope, node);
      break;
    case AST_LIB_FUNC_CALL:
      visited = visitor_visit_lib_func_call(visitor, scope, node);
      break;
    case AST_COMP:
      return visitor_visit_comp(visitor, scope, node);
    case AST_VAR_DEF:
      return visitor_visit_var_def(visitor, scope, node);
    case AST_IF:
      return visitor_visit_if(visitor, scope, node);
    case AST_ELSE:
      return visitor_visit_else(visitor, scope, node);
    case AST_DOWHILE:
      return visitor_visit_dowhile(visitor, scope, node);
    case AST_WHILE:
      return visitor_visit_while(visitor, scope, node);
    case AST_FOR:
      return visitor_visit_for(visitor, scope, node);
    case AST_FUNC_DEF:
      return visitor_visit_func_def(visitor, scope, node);
    case AST_STRUCT_DEF:
      return visitor_visit_struct_def(visitor, scope, node);
    case AST_RETURN:
      return visitor_visit_return(visitor, scope, node);
    case AST_UNARY:
      visited = visitor_visit_unary(visitor, scope, node);
      break;
    case AST_BINARY:
      visited = visitor_visit_binary(visitor, scope, node);
      break;
    case AST_BINARY_BOOL:
      visited = visitor_visit_binary_bool(visitor, scope, node);
      break;
    case AST_TERNARY:
      visited = visitor_visit_ternary(visitor, scope, node);
      break;
    case AST_ASSIGN:
      visited = visitor_visit_assign(visitor, scope, node);
      break;
    case AST_INCLUDE:
      return visitor_visit_include(visitor, scope, node);
    default: {
      char err[ERR_LEN];
      sprintf(err, "%d type not expected", node->type);
      return visitor_error(visitor, node, err);
    }
  }
  gc_track(visitor->gc, visited);
  return visited;
}

/* datas */
static ast_t* visitor_visit_list_lit(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* list = create_ast(AST_LIST);
  size_t size = node->list_lit.mem_size;
  list->list.mem_size = size;
  list->list.mems = SEAL_CALLOC(size, sizeof(ast_t*));
  for (int i = 0; i < size; i++) {
    list->list.mems[i] = visitor_visit(visitor, scope, node->list_lit.mems[i]);
    gc_retain(list->list.mems[i]); // add 1 reference
  }
  return list;
}
static ast_t* visitor_visit_object_lit(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* object = create_ast(AST_OBJECT);
  size_t size = node->object_lit.field_size;
  object->object.is_lit = true; // made up by literal
  object->object.field_size = size; // field size
  object->object.field_names = node->object_lit.field_names; // assign names
  object->object.field_vals = SEAL_CALLOC(size, sizeof(ast_t*)); // allocate memory for fields values
  for (int i = 0; i < size; i++) {
    object->object.field_vals[i] = visitor_visit(visitor, scope, node->object_lit.field_vals[i]);
    gc_retain(object->object.field_vals[i]); // add 1 reference
  }
  return object;
}
static ast_t* visitor_visit_var_ref(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* val = scope_get_var(scope, node->var_ref.name, node->line);
  if (!val) {
    bool found = false;
    for (int i = 0; i < visitor->state_size; i++) {
      if ((val = list_get_var(visitor->states[i]->visitor->ext_vars, node->var_ref.name, node->line))) {
        found = true;
        break;
      }
    }
    if (!found) {
      char err[ERR_LEN];
      sprintf(err, "\'%s\' is undefined", node->var_ref.name);
      return visitor_error(visitor, node, err);
    }
  }
  return val->variable.val;
}
static ast_t* visitor_visit_func_call(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  /* built-in functions */
  ast_t* visited = NULL; // only for builtin functions
  if (strcmp(node->func_call.name, "writeln") == 0) {
    size_t arg_size = node->func_call.arg_size;
    ast_t* args[arg_size];
    for (int i = 0; i < arg_size; i++) {
      args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
    }
    visited = builtin_writeln(args, arg_size, true, true);
  } else if (strcmp(node->func_call.name, "readln") == 0) {
    size_t arg_size = node->func_call.arg_size;
    if (arg_size > 1) {
      visitor_error(visitor, node, "readln can get at most 1 argument");
    }
    ast_t* args[arg_size];
    for (int i = 0; i < arg_size; i++) { // push variables to local scope
      args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
      if (args[i]->type != AST_STRING) {
        visitor_error(visitor, node, "readln can only get string as argument");
      }
    }
    
    visited = builtin_readln(args, arg_size);
  } else if (strcmp(node->func_call.name, "len") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "len",
                       1,
                       arg_size);
    ast_t* arg = visitor_visit(visitor, scope, node->func_call.args[0]);
    kill_if_non_iterable(visitor, arg, node->func_call.args[0]);

    visited = builtin_len(arg);
  } else if (strcmp(node->func_call.name, "push") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "push",
                       2,
                       arg_size);
    ast_t* args[2];
    args[0] = visitor_visit(visitor, scope, node->func_call.args[0]);
    args[1] = visitor_visit(visitor, scope, node->func_call.args[1]);
    kill_if_non_iterable(visitor, args[0], node->func_call.args[0]);

    visited = builtin_push(args);
  } else if (strcmp(node->func_call.name, "pop") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "pop",
                       1,
                       arg_size);
    ast_t* arg;
    arg = visitor_visit(visitor, scope, node->func_call.args[0]);
    kill_if_non_list(visitor, arg, node->func_call.args[0]);

    visited = builtin_pop(arg, node);
  } else if (strcmp(node->func_call.name, "format") == 0) {
    size_t arg_size = node->func_call.arg_size;
    ast_t* args[arg_size];
    for (int i = 0; i < arg_size; i++) {
      args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
    }
    visited = builtin_format(node, args, arg_size);
  } else if (strcmp(node->func_call.name, "open") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "open",
                       2,
                       arg_size);
    ast_t* args[arg_size];
    for (int i = 0; i < arg_size; i++) {
      args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
      if (args[i]->type != AST_STRING)
        visitor_error(visitor, node, "open can only get string as arguments");
    }
    visited = builtin_open(args);
  } else if (strcmp(node->func_call.name, "read") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "read",
                       1,
                       arg_size);
    ast_t* arg = visitor_visit(visitor, scope, node->func_call.args[0]);
    if (arg->type != AST_INT)
      visitor_error(visitor, node, "read requires file");
    visited = builtin_read(arg);
  } else if (strcmp(node->func_call.name, "close") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "close",
                       1,
                       arg_size);
    ast_t* arg = visitor_visit(visitor, scope, node->func_call.args[0]);
    if (arg->type != AST_INT)
      visitor_error(visitor, node, "read requires file");
    visited = builtin_close(arg);
  } else if (strcmp(node->func_call.name, "write") == 0) {
    size_t arg_size = node->func_call.arg_size;
    kill_if_argsize_ne(visitor, // arguments and parameters size must match
                       node,
                       true,
                       "write",
                       2,
                       arg_size);
    ast_t* args[arg_size];
    for (int i = 0; i < arg_size; i++) {
      args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
    }
    if (args[0]->type != AST_INT)
      visitor_error(visitor, node, "open requires file");
    if (args[1]->type != AST_STRING)
      visitor_error(visitor, node, "open requires string for file");
    visited = builtin_write(args);
  }
  if (visited) {
    gc_track(visitor->gc, visited); // only builtin
    return visited;
  }

  ast_t* called;
  if ((called = search_func(visitor->func_defs, node->func_call.name)) == NULL) {
    for (int i = 0; i < visitor->state_size; i++) {
      if ((called = search_func(visitor->states[i]->visitor->ext_func_defs, node->func_call.name)) != NULL) {
        size_t arg_size = node->func_call.arg_size;
        ast_t* args[arg_size];
        for (int i = 0; i < arg_size; i++) {
          args[i] = visitor_visit(visitor, scope, node->func_call.args[i]);
        }
        return state_call_func(visitor->states[i], called, args, arg_size);
      }
    }
    char err[ERR_LEN];
    sprintf(err, "no function named \'%s\'", node->func_call.name);
    return visitor_error(visitor, node, err);
  }
  /*
   * evaluate as normal function call
   * evaluate args
   */
  scope_t local_scope; // allocate local scope on stack
  init_scope(&local_scope, scope); // initialize local scope

  size_t arg_size = node->func_call.arg_size;

  kill_if_argsize_ne(visitor, // arguments and parameters size must match
                     node,
                     true,
                     called->func_def.name,
                     called->func_def.param_size,
                     arg_size);

  for (int i = 0; i < arg_size; i++) { // push variables to local scope
    ast_t* arg_var = create_var_ast(called->func_def.param_names[i],
                                    visitor_visit(visitor, scope, node->func_call.args[i]),
                                    false,
                                    node->line);
    scope_push_var(&local_scope, arg_var);
  }
  
  /* TODO:
   * free local scope after visiting body
   * push returned value (if exist) to garbage collector's list
   */
  visitor->func_call_size++;

  // visit body and return
  ast_t* returned_val = visitor_visit(visitor, &local_scope, called->func_def.comp); // should be visited
  // free scope
  gc_free_scope(&local_scope);
  gc_flush(visitor->gc);

  visitor->func_call_size--;

  if (visitor->func_call_size <= 0) gc_flush_ret(visitor->gc);
  /*gc_print_ret(visitor->gc);*/
  /*gc_print(visitor->gc);*/
  return visitor_visit(visitor, NULL, returned_val);
}
static ast_t* visitor_visit_constructor(ast_t* constructor, visitor_t* visitor, scope_t* scope, ast_t* node)
{
  // constructor is 'struct_def'
  scope_t local_scope; // allocate local scope on stack
  init_scope(&local_scope, NULL); // initialize local scope without outer scope

  size_t arg_size = node->func_call.arg_size;


  kill_if_argsize_ne(visitor, // arguments and parameters size must match
                     node,
                     false,
                     constructor->struct_def.name,
                     constructor->struct_def.param_size,
                     arg_size);

  for (int i = 0; i < arg_size; i++) { // push variables to local scope
    ast_t* arg_var = create_var_ast(constructor->struct_def.param_names[i],
                                    visitor_visit(visitor, scope, node->func_call.args[i]),
                                    false,
                                    node->line);
    scope_push_var(&local_scope, arg_var);
  }

  // create object, assign its fields and return
  ast_t* object = create_ast(AST_OBJECT);
  object->object.is_lit = false;
  object->object.def = constructor;
  object->object.field_size = constructor->struct_def.field_size;
  object->object.field_names = constructor->struct_def.field_names;
  // allocate memory, visit and assign its fields
  object->object.field_vals = SEAL_CALLOC(constructor->struct_def.field_size, sizeof(ast_t*));
  for (int i = 0; i < constructor->struct_def.field_size; i++) {
    object->object.field_vals[i] = visitor_visit(visitor, &local_scope, constructor->struct_def.field_exprs[i]);
    gc_retain(object->object.field_vals[i]);
  }
  
  gc_free_scope(&local_scope);

  return object;
}
static ast_t* visitor_visit_subscript(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* main = visitor_visit(visitor, scope, node->subscript.main);
  kill_if_non_iterable(visitor, main, node->subscript.main);
  ast_t* index = visitor_visit(visitor, scope, node->subscript.index);
  kill_if_index_non_int(visitor, index, node->subscript.index);

  switch (main->type) {
    case AST_LIST:
      if (main->list.mem_size <= index->integer.val) {
        goto error;
      }
      return main->list.mems[index->integer.val];
    case AST_STRING:
      if (main->list.mem_size > index->integer.val) {
        ast_t* chr = create_ast(AST_STRING);
        chr->string.val = SEAL_CALLOC(2, sizeof(char));
        chr->string.val[0] = main->string.val[index->integer.val];
        chr->string.val[1] = '\0';
        return chr;
      }
      goto error;
    error:
      return visitor_error(visitor, node, "index out of bounds");
  }
  return ast_null();
}
static ast_t* visitor_visit_memacc(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* main = visitor_visit(visitor, scope, node->memacc.main);
  kill_if_non_object(visitor, main, node);
  return get_object_mem(visitor, main, node->memacc.mem);
}
static ast_t* visitor_visit_lib_func_call(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* libseal = node->lib_func_call.lib;
  for (int i = 0; i < visitor->libseal_size; i++) {
    if (strcmp(libseal->var_ref.name, visitor->libseals[i]->name) == 0) {
      size_t arg_size = node->lib_func_call.func_call->func_call.arg_size;
      ast_t* args[arg_size];
      for (int i = 0; i < arg_size; i++) {
        args[i] = visitor_visit(visitor, scope, node->lib_func_call.func_call->func_call.args[i]);
      }
      ast_t* res = libseal_function_call(visitor->libseals[i], node->lib_func_call.func_call->func_call.name, args, arg_size);
      if (res->type == AST_OBJECT || res->type == AST_LIST) {
        track_library_returned_object(visitor, res, true);
      }
      return res;
    }
  }

  char err[ERR_LEN];
  sprintf(err, "library \'%s\' is not included", libseal->var_ref.name);
  return visitor_error(visitor, node, err);
}
/* blocks */
static ast_t* visitor_visit_comp(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  for (int i = 0; i < node->comp.stmt_size; i++) {
    ast_t* visited = visitor_visit(visitor, scope, node->comp.stmts[i]);
    switch (visited->type) {
      case AST_SKIP:
      case AST_STOP:
      case AST_RETURNED_VAL:
        return visited;
    }
  }
  return ast_null();
}
static ast_t* visitor_visit_var_def(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  bool is_const = node->var_def.is_const;
  for (int i = 0; i < node->var_def.size; i++) {
    ast_t* var = create_var_ast(node->var_def.names[i],
                                visitor_visit(visitor, scope, node->var_def.exprs[i]),
                                is_const,
                                node->line);
    scope_push_var(scope, var);
    if (node->var_def.is_extern)
      list_push(&visitor->ext_vars, var);
  }
  return ast_null();
}
static ast_t* visitor_visit_if(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* cond = visitor_visit(visitor, scope, node->_if.cond);
  kill_if_non_bool(visitor, cond, node->_if.cond);
  if (cond->boolean.val == true) {
    scope_t local_scope;
    init_scope(&local_scope, scope);

    ast_t* visited = visitor_visit(visitor, &local_scope, node->_if.comp);

    gc_free_scope(&local_scope);
    gc_flush(visitor->gc);

    return visited;
  }
  return visitor_visit(visitor, scope, node->_if.has_else ? node->_if._else : ast_null());
}
static ast_t* visitor_visit_else(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  scope_t local_scope;
  init_scope(&local_scope, scope);
  ast_t* visited = visitor_visit(visitor, &local_scope, node->_else.comp);

  gc_free_scope(&local_scope);
  gc_flush(visitor->gc);

  return visited;
}
static ast_t* visitor_visit_dowhile(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  loop:
  {
    scope_t local_scope;
    init_scope(&local_scope, scope);
    ast_t* visited = visitor_visit(visitor, &local_scope, node->_while.comp);

    gc_free_scope(&local_scope);
    gc_flush(visitor->gc);

    switch (visited->type) {
      case AST_RETURNED_VAL:
        return visited;
      case AST_STOP:
        return ast_null();
    }
    // TOOD free scope
  }

  ast_t* cond = visitor_visit(visitor, scope, node->_while.cond);
  kill_if_non_bool(visitor, cond, node->_while.cond);
  if (cond->boolean.val == true) goto loop;
  return ast_null();
}
static ast_t* visitor_visit_while(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  loop:
  {
    ast_t* cond = visitor_visit(visitor, scope, node->_while.cond);
    kill_if_non_bool(visitor, cond, node->_while.cond);
    if (cond->boolean.val == false) return ast_null();

    scope_t local_scope;
    init_scope(&local_scope, scope);
    ast_t* visited = visitor_visit(visitor, &local_scope, node->_while.comp);

    // TOOD free scope
    gc_free_scope(&local_scope);
    gc_flush(visitor->gc);

    switch (visited->type) {
      case AST_RETURNED_VAL:
        return visited;
      case AST_STOP:
        return ast_null();
    }
    
    goto loop;
  }

  return ast_null();
}
static ast_t* visitor_visit_for(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  bool is_numerical = node->_for.is_numerical;
  
  if (is_numerical) {
    ast_t* start = node->_for.start,
         * end   = node->_for.end,
         * step  = node->_for.step;

    Seal_int index = 0, max_index, istep = 1;
    kill_if_non_int(visitor, (start = visitor_visit(visitor, scope, start)), node);
    gc_retain(start); // keep ited until for loop ends
    if (end && step) {
      kill_if_non_int(visitor, (end  = visitor_visit(visitor, scope, end)), node);
      kill_if_non_int(visitor, (step = visitor_visit(visitor, scope, step)), node);
      gc_retain(end); // keep ited until for loop ends
      gc_retain(step); // keep ited until for loop ends
      index     = start->integer.val;
      max_index = end->integer.val;
      istep     = step->integer.val;
    } else if (end) {
      kill_if_non_int(visitor, (end  = visitor_visit(visitor, scope, end)), node);
      gc_retain(end); // keep ited until for loop ends
      index     = start->integer.val;
      max_index = end->integer.val;
    } else if (step) {
      kill_if_non_int(visitor, (step = visitor_visit(visitor, scope, step)), node);
      gc_retain(step); // keep ited until for loop ends
      index     = 0;
      max_index = start->integer.val;
      istep     = step->integer.val;
    }

    // in order to avoid segfault in jump_exit when freeing scope,
    // declare it here
    scope_t for_scope;
    init_scope(&for_scope, scope);

    if (index >= max_index) {
      // free memory
      goto jump_exit;
    }

    ast_t* iter_var = create_var_ast(node->_for.it_name,
                                     visitor_visit(visitor, scope, ast_null()),
                                     false,
                                     node->line);
    scope_push_var(&for_scope, iter_var);
    ast_t* cur_iter = NULL; // currently iterated item

    loop0:
    {
      cur_iter = create_ast(AST_INT);
      cur_iter->integer.val = index;

      gc_track(visitor->gc, cur_iter);
      gc_release(iter_var->variable.val);
      gc_retain(cur_iter);
      iter_var->variable.val = cur_iter;

      scope_t local_scope;
      init_scope(&local_scope, &for_scope);
      ast_t* visited = visitor_visit(visitor, &local_scope, node->_for.comp);

      // TOOD free scope
      gc_free_scope(&local_scope);
      //gc_flush(visitor->gc);

      switch (visited->type) {
        case AST_RETURNED_VAL:
          if (end)  gc_release(end);
          if (step) gc_release(step);
          gc_release(start);
          gc_free_scope(&for_scope);
          gc_flush(visitor->gc);
          return visited;
        case AST_STOP:
          goto jump_exit;
      }
      
      gc_flush(visitor->gc);
      if ((index += istep) < max_index) goto loop0;
    }

    jump_exit:
      if (end)  gc_release(end);
      if (step) gc_release(step);
      gc_release(start);
      gc_free_scope(&for_scope);
      gc_flush(visitor->gc);
  }

  if (!is_numerical) {
    int type;
    Seal_int index = 0;
    // non-numerical
    ast_t* ited = visitor_visit(visitor, scope, node->_for.ited);
    gc_retain(ited); // keep ited until for loop ends
    Seal_int max_index;

    switch (ited->type) {
      case AST_INT:
        max_index = ited->integer.val;
        type = FOR_INT;
        break;
      case AST_STRING:
        max_index = strlen(ited->string.val);
        type = FOR_STRING;
        break;
      case AST_LIST:
        max_index = ited->list.mem_size;
        type = FOR_LIST;
        break;
      default: {
        char err[ERR_LEN];
        sprintf(err, "\'%s\' is not iterable in for loop", hast_type_name(ited->type));
        return visitor_error(visitor, node->_for.ited, err);
      }
    }

    if (max_index == 0) {
      // free memory
      gc_release(ited);
      gc_flush(visitor->gc);
      return ast_null();
    }

    scope_t for_scope;
    init_scope(&for_scope, scope);

    ast_t* iter_var = create_var_ast(node->_for.it_name,
                                     visitor_visit(visitor, scope, ast_null()),
                                     false,
                                     node->line);
    scope_push_var(&for_scope, iter_var);
    ast_t* cur_iter = NULL; // currently iterated item

    loop1:
    {
      switch (type) {
        case FOR_INT:
          cur_iter = create_ast(AST_INT);
          cur_iter->integer.val = index;
          break;
        case FOR_STRING:
          cur_iter = create_ast(AST_STRING);
          cur_iter->string.val = SEAL_CALLOC(2, sizeof(char));
          cur_iter->string.val[0] = ited->string.val[index];
          cur_iter->string.val[1] = '\0';
          break;
        case FOR_LIST:
          cur_iter = ited->list.mems[index];
          break;
      }
      gc_track(visitor->gc, cur_iter);
      gc_release(iter_var->variable.val);
      gc_retain(cur_iter);
      iter_var->variable.val = cur_iter;

      scope_t local_scope;
      init_scope(&local_scope, &for_scope);
      ast_t* visited = visitor_visit(visitor, &local_scope, node->_for.comp);

      // TOOD free scope
      gc_free_scope(&local_scope);
      //gc_flush(visitor->gc);

      switch (visited->type) {
        case AST_RETURNED_VAL:
          gc_release(ited);
          gc_free_scope(&for_scope);
          gc_flush(visitor->gc);
          return visited;
        case AST_STOP:
          gc_release(ited);
          gc_free_scope(&for_scope);
          gc_flush(visitor->gc);
          return ast_null();
      }
      
      gc_flush(visitor->gc);
      if (++index < max_index) goto loop1;
    }

    gc_release(ited);
    gc_free_scope(&for_scope);
    gc_flush(visitor->gc);
  }
  
  return ast_null();
}
static ast_t* visitor_visit_func_def(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  list_push(&visitor->func_defs, node);
  if (node->func_def.is_extern)
    list_push(&visitor->ext_func_defs, node);
  return node;
}
static ast_t* visitor_visit_struct_def(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  list_push(&visitor->struct_defs, node);
  return node;
}
/* block control */
static ast_t* visitor_visit_return(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* ast = create_ast(AST_RETURNED_VAL);
  ast->returned_val.val = visitor_visit(visitor, scope, node->_return.expr);
  gc_retain(ast->returned_val.val);
  gc_track_ret(visitor->gc, ast);
  return ast;
}
/* operations */
static ast_t* visitor_visit_unary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* val = visitor_visit(visitor, scope, node->unary.expr);
  switch (node->unary.op_type) {
    case TOK_PLUS:
      kill_if_non_number(visitor, val, node);
      return val;
    case TOK_MINUS: {
      kill_if_non_number(visitor, val, node);
      ast_t* negated = create_ast(val->type);
      if (negated->type == AST_INT) {
        negated->integer.val = -val->integer.val;
      } else {
        negated->floating.val = -val->floating.val;
      }
      return negated;
      // TOOD free val
    }
    case TOK_NOT:
      kill_if_non_bool(visitor, val, node);
      return val->boolean.val ? ast_false() : ast_true();
  }
  return visitor_error(visitor, node, "VISITOR_VISIT_UNARY UNDEFINED");
}
static ast_t* visitor_visit_binary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* bin_left  = visitor_visit(visitor, scope, node->binary.left);
  ast_t* bin_right = visitor_visit(visitor, scope, node->binary.right);
  
  if (bin_left->type == AST_INT && bin_right->type == AST_INT) {
    Seal_int left  = bin_left->integer.val;
    Seal_int right = bin_right->integer.val;
    Seal_int result;

    switch (node->binary.op_type) {
      // first comparisons
      case TOK_EQ:
        return left == right ? ast_true() : ast_false();
      case TOK_NE:
        return left != right ? ast_true() : ast_false();
      case TOK_GT:
        return left > right ? ast_true() : ast_false();
      case TOK_GE:
        return left >= right ? ast_true() : ast_false();
      case TOK_LT:
        return left < right ? ast_true() : ast_false();
      case TOK_LE:
        return left <= right ? ast_true() : ast_false();
      case TOK_PLUS:
        result = left + right;
        break;
      case TOK_MINUS:
        result = left - right;
        break;
      case TOK_MUL:
        result = left * right;
        break;
      case TOK_DIV:
        result = left / right;
        break;
      case TOK_MOD:
        result = left % right;
        break;
      case TOK_POW:
        result = pow(left, right);
        break;
      default:
        binary_op_error(visitor, node, "integer", node->binary.op_type);
        break;
    }
    ast_t* res_node = create_ast(AST_INT);
    res_node->integer.val = result;
    return res_node;
  } else if (bin_left->type == AST_FLOAT && bin_right->type == AST_FLOAT) {
    Seal_float left  = bin_left->floating.val;
    Seal_float right = bin_right->floating.val;
    Seal_float result;

    switch (node->binary.op_type) {
      // first comparisons
      case TOK_EQ:
        return left == right ? ast_true() : ast_false();
      case TOK_NE:
        return left != right ? ast_true() : ast_false();
      case TOK_GT:
        return left > right ? ast_true() : ast_false();
      case TOK_GE:
        return left >= right ? ast_true() : ast_false();
      case TOK_LT:
        return left < right ? ast_true() : ast_false();
      case TOK_LE:
        return left <= right ? ast_true() : ast_false();
      case TOK_PLUS:
        result = left + right;
        break;
      case TOK_MINUS:
        result = left - right;
        break;
      case TOK_MUL:
        result = left * right;
        break;
      case TOK_DIV:
        result = left / right;
        break;
      case TOK_POW:
        result = pow(left, right);
        break;
      default:
        binary_op_error(visitor, node, "float", node->binary.op_type);
        break;
    }
    ast_t* res_node = create_ast(AST_FLOAT);
    res_node->floating.val = result;
    return res_node;
  } else if ((bin_left->type == AST_INT && bin_right->type == AST_FLOAT) ||
             (bin_left->type == AST_FLOAT && bin_right->type == AST_INT)) {
    Seal_float left  = bin_left->type == AST_INT ? bin_left->integer.val : bin_left->floating.val;
    Seal_float right = bin_right->type == AST_INT ? bin_right->integer.val : bin_right->floating.val;
    Seal_float result;

    switch (node->binary.op_type) {
      // first comparisons
      case TOK_EQ:
        return left == right ? ast_true() : ast_false();
      case TOK_NE:
        return left != right ? ast_true() : ast_false();
      case TOK_GT:
        return left > right ? ast_true() : ast_false();
      case TOK_GE:
        return left >= right ? ast_true() : ast_false();
      case TOK_LT:
        return left < right ? ast_true() : ast_false();
      case TOK_LE:
        return left <= right ? ast_true() : ast_false();
      case TOK_PLUS:
        result = left + right;
        break;
      case TOK_MINUS:
        result = left - right;
        break;
      case TOK_MUL:
        result = left * right;
        break;
      case TOK_DIV:
        result = left / right;
        break;
      case TOK_POW:
        result = pow(left, right);
        break;
      default:
        binary_op_error(visitor, node, "float", node->binary.op_type);
        break;
    }
    ast_t* res_node = create_ast(AST_FLOAT);
    res_node->floating.val = result;
    return res_node;
  } else if (bin_left->type == AST_STRING && bin_right->type == AST_STRING) {
    const char* left  = bin_left->string.val;
    const char* right = bin_right->string.val;
    char* result;
    size_t len;

    switch (node->binary.op_type) {
      // first comparisons
      case TOK_EQ:
        return strcmp(left, right) == 0 ? ast_true() : ast_false();
      case TOK_NE:
        return strcmp(left, right) != 0 ? ast_true() : ast_false();
      case TOK_PLUS:
        len = strlen(left) + strlen(right);
        result = SEAL_CALLOC(len + 1, sizeof(char));
        strcat(result, left);
        strcat(result, right);
        result[len] = '\0';
        break;
      default:
        binary_op_error(visitor, node, "string", node->binary.op_type);
        break;
    }
    ast_t* res_node = create_ast(AST_STRING);
    res_node->string.val = result;
    return res_node;
  } else if (bin_left->type == AST_BOOL && bin_right->type == AST_BOOL) {
    bool left  = bin_left->boolean.val;
    bool right = bin_right->boolean.val;

    switch (node->binary.op_type) {
      case TOK_EQ:
        return left == right ? ast_true() : ast_false();
      case TOK_NE:
        return left != right ? ast_true() : ast_false();
      default:
        binary_op_error(visitor, node, "bool", node->binary.op_type);
        break;
    }
  } else if (bin_left->type == AST_OBJECT && bin_right->type == AST_OBJECT) {
    switch (node->binary.op_type) {
      case TOK_EQ:
        return &bin_left->object == &bin_right->object ? ast_true() : ast_false();
      case TOK_NE:
        return &bin_left->object != &bin_right->object ? ast_true() : ast_false();
      default:
        binary_op_error(visitor, node, "object", node->binary.op_type);
        break;
    }
  } else if (bin_left->type == AST_NULL || bin_right->type == AST_NULL) {
    switch (node->binary.op_type) {
      case TOK_EQ:
        return bin_left->type == bin_right->type ? ast_true() : ast_false();
      case TOK_NE:
        return bin_left->type != bin_right->type ? ast_true() : ast_false();
      default:
        binary_op_error(visitor, node, "null", node->binary.op_type);
        break;
    }
  }

  char err[ERR_LEN];
  sprintf(err,
          "\'%s\' operator is not supported for \'%s\' and \'%s\'",
          htoken_type_name(node->binary.op_type),
          hast_type_name(bin_left->type),
          hast_type_name(bin_right->type));
  return visitor_error(visitor, node, err);
}
static ast_t* visitor_visit_binary_bool(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* left = visitor_visit(visitor, scope, node->binary_bool.left);
  kill_if_non_bool(visitor, left, node->binary_bool.left);
  ast_t* right;
  switch (node->binary_bool.op_type) {
    case TOK_AND:
      if (left->boolean.val == false) return ast_false();
      right = visitor_visit(visitor, scope, node->binary_bool.right);
      kill_if_non_bool(visitor, left, node->binary_bool.right);
      return right->boolean.val ? ast_true() : ast_false();
    case TOK_OR:
      if (left->boolean.val == true) return ast_true();
      right = visitor_visit(visitor, scope, node->binary_bool.right);
      kill_if_non_bool(visitor, left, node->binary_bool.right);
      return !right->boolean.val ? ast_false() : ast_true();
  }
  return visitor_error(visitor, node, "VISITOR_VISIT_BINARY_BOOL UNDEFINED");
}
static ast_t* visitor_visit_ternary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* cond = visitor_visit(visitor, scope, node->ternary.cond);
  kill_if_non_bool(visitor, cond, node->ternary.cond);
  return visitor_visit(visitor, scope, cond->boolean.val == true ? node->ternary.expr_true : node->ternary.expr_false);
}
static ast_t* visitor_visit_assign(visitor_t* visitor, scope_t* scope, ast_t* node) // consider constant
{
  ast_t* symbol = node->assign.var;
  ast_t* assigned = visitor_visit(visitor, scope, node->assign.expr);
  gc_retain(assigned);
  switch (symbol->type) {
    case AST_VAR_REF: {
      ast_t* var = scope_get_var(scope, symbol->var_ref.name, symbol->line);
      if (!var) {
        char err[ERR_LEN];
        sprintf(err, "\'%s\' is undefined", symbol->var_ref.name);
        return visitor_error(visitor, node, err);
      }
      if (var->variable.is_const) {
        char err[ERR_LEN];
        sprintf(err, "assignment to constant symbol \'%s\'", var->variable.name);
        return visitor_error(visitor, symbol, err);
      }
      // free unassigned symbol value
      gc_release(var->variable.val);
      var->variable.val = assigned;
      break;
    }
    case AST_SUBSCRIPT: {
      ast_t* main = visitor_visit(visitor, scope, symbol->subscript.main);
      kill_if_non_iterable(visitor, main, symbol->subscript.main);
      ast_t* index = visitor_visit(visitor, scope, symbol->subscript.index);
      kill_if_index_non_int(visitor, index, symbol->subscript.index);

      switch (main->type) {
        case AST_LIST:
          if (main->list.mem_size <= index->integer.val) {
            goto error;
          }
          // free unassigned member
          gc_release(main->list.mems[index->integer.val]);
          main->list.mems[index->integer.val] = assigned;
          break;
        case AST_STRING: {
          char err[ERR_LEN];
          sprintf(err, "strings are read-only");
          return visitor_error(visitor, symbol, err);
        }
        error:
          return visitor_error(visitor, node, "index out of bounds");
      }
      break;
    }
    case AST_MEMACC: {
      ast_t* main = visitor_visit(visitor, scope, symbol->memacc.main);
      ast_t* mem = symbol->memacc.mem;
      kill_if_non_object(visitor, main, symbol->memacc.main);
      bool found = false;
      for (int i = 0; i < main->object.field_size; i++) {
        if (strcmp(mem->var_ref.name, main->object.field_names[i]) == 0) {
          // free unassigned field
          found = true;
          gc_release(main->object.field_vals[i]);
          main->object.field_vals[i] = assigned;
          break;
        }
      }
      if (found) break;
      char err[ERR_LEN];
      sprintf(err,
              "%sobject has no field named \'%s\'",
              !main->object.is_lit ? main->object.def->struct_def.name : "",
              mem->var_ref.name);
      return visitor_error(visitor, mem, err);
    }
  }
  return assigned;
}
/* others */
static ast_t* visitor_visit_include(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  if (node->include.type == SEAL_INCLUDE_SRC) {
    if (visitor->state_size++ >= SEAL_MAX_STATE_SIZE)
      visitor_error(visitor, node, "maximum capacity of including seal files reached");
    state_t* state = SEAL_CALLOC(1, sizeof(state_t));
    init_state(state, visitor->gc, node->include.name);
    visitor->states[visitor->state_size - 1] = state;
  } else {
    for (int i = 0; i < visitor->libseal_size; i++) {
      if (strcmp(node->include.name, visitor->libseals[i]->name) == 0) {
        char err[ERR_LEN];
        sprintf(err, "library \'%s\' already included", node->include.name);
        return visitor_error(visitor, node, err);
      }
    }
    visitor->libseal_size++;
    visitor->libseals = realloc(visitor->libseals, visitor->libseal_size * sizeof(libseal_t*));
    visitor->libseals[visitor->libseal_size - 1] = create_libseal(node->include.name,
                                                                  node->include.has_alias,
                                                                  node->include.alias);
  }

  return ast_null();
}
