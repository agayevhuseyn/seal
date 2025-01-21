#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "visitor.h"
#include "ast.h"
#include "scope.h"
#include "io.h"
#include "gc.h"

visitor_t* init_visitor(parser_t* parser)
{
  visitor_t* visitor = calloc(1, sizeof(visitor_t));

  visitor->g_scope = init_scope();
  visitor->g_scope->is_global = true;
  visitor->func_defs = parser->func_defs;
  visitor->func_size = parser->func_size;
  visitor->obj_defs = parser->obj_defs;
  visitor->obj_size = parser->obj_size;
  visitor->libseals = (void*)0;
  visitor->libseal_size = 0;

  return visitor;
}

static inline ast_t* visitor_error(visitor_t* visitor, const char* msg)
{
  printf("Visitor-> Error: %s\n", msg);
  exit(1);
  return ast_noop();
}

static inline void visitor_check_index(visitor_t* visitor, ast_t* main, ast_t* ast_index)
{
  if (ast_index->type != AST_INT) {
    char msg[128];
    sprintf(msg, "indices must be integers, not %s", ast_name(ast_index));
    visitor_error(visitor, msg);
  }
  int index = ast_index->integer.val;
  if (index < 0) {
    char msg[128];
    sprintf(msg, "indices must be positive");
    visitor_error(visitor, msg);
  }
  switch (main->type) {
    case AST_STRING:
      if (index >= strlen(main->string.val)) {
        char msg[128];
        sprintf(msg, "\"%s\" index out of range: %d, max: %lu", main->string.val, index, strlen(main->string.val) - 1);
        visitor_error(visitor, msg);
      }
      break;
    case AST_LIST:
      if (index >= main->list.mem_size) {
        char msg[128];
        sprintf(msg, "list index out of range: %d, max: %lu", index, main->list.mem_size - 1);
        visitor_error(visitor, msg);
      }
      break;
    default: {
      char msg[128];
      sprintf(msg, "%s is not subscriptable", ast_name(main));
      visitor_error(visitor, msg);
    }
  }
}

ast_t* visitor_visit(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  switch (node->type) {
    case AST_NOOP: return node;
    case AST_COMP: return visitor_visit_comp(visitor, scope, node);
    case AST_FUNC_CALL: return visitor_visit_func_call(visitor, scope, node);
    case AST_INT: return visitor_visit_int(visitor, scope, node);
    case AST_STRING: return visitor_visit_string(visitor, scope, node);
    case AST_FLOAT: return visitor_visit_float(visitor, scope, node);
    case AST_BOOL: return visitor_visit_bool(visitor, scope, node);
    case AST_NULL: return visitor_visit_null(visitor, scope, node);
    case AST_SKIP: return visitor_visit_skip(visitor, scope, node);
    case AST_STOP: return visitor_visit_skip(visitor, scope, node);
    case AST_INCLUDE: return visitor_visit_include(visitor, scope, node);
    case AST_LIST: return visitor_visit_list(visitor, scope, node);
    case AST_BINARY: return visitor_visit_binary(visitor, scope, node);
    case AST_UNARY: return visitor_visit_unary(visitor, scope, node);
    case AST_TERNARY: return visitor_visit_ternary(visitor, scope, node);
    case AST_VARDEF: return visitor_visit_vardef(visitor, scope, node);
    case AST_VAR_REF: return visitor_visit_var_ref(visitor, scope, node);
    case AST_ASSIGN: return visitor_visit_assign(visitor, scope, node);
    case AST_MEM_ACC: return visitor_visit_mem_acc(visitor, scope, node);
    case AST_SUBSCRIPT: return visitor_visit_subscript(visitor, scope, node);
    case AST_LIBSEAL_FCALL: return visitor_visit_libseal_fcall(visitor, scope, node);
    case AST_IF: return visitor_visit_if(visitor, scope, node);
    case AST_ELSE: return visitor_visit_else(visitor, scope, node);
    case AST_WHILE: return visitor_visit_while(visitor, scope, node);
    case AST_FOR: return visitor_visit_for(visitor, scope, node);
    case AST_RETURN: return visitor_visit_return(visitor, scope, node);
    case AST_RETURN_VAL: return node->return_val.val;
    default: {
      char msg[128];
      sprintf(msg, "unexpected node type: \"%s\"", ast_name(node));
      return visitor_error(visitor, msg); 
    }
  }
}

ast_t* visitor_visit_comp(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  for (int i = 0; i < node->comp.stmt_size; i++) {
    ast_t* visited = visitor_visit(visitor, scope, node->comp.stmts[i]);
    switch (visited->type) {
      case AST_SKIP:
      case AST_STOP:
      case AST_RETURN_VAL:
        return visited;
      default:
        break;
    }
  }
  
  return ast_noop();
}

static ast_t* builtin_span(ast_t* count)
{
  ast_t* list = init_ast(AST_LIST);
  list->list.mem_size = count->integer.val;
  list->list.mems = calloc(count->integer.val, sizeof(ast_t*));
  for (int i = 0; i < count->integer.val; i++) {
    ast_t* mem = init_ast(AST_INT);
    mem->integer.val = i;
    list->list.mems[i] = mem;
    //list_push(list, mem);
  }

  for (int i = 0; i < count->integer.val; i++) {
    free(list->list.mems[i]);
  }
  printf("FREED ELEMENTS\n");
  getchar();
  free(list->list.mems);
  free(list);
  printf("FREED LIST\n");
  getchar();

  return list;
}

ast_t* visitor_visit_func_call(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  // user defined functions
  /*
  if (strcmp(node->func_call.fname, "span") == 0) {
    return builtin_span(node->func_call.args[0]);
  }
  */
  for (int i = 0; i < visitor->func_size; i++) {
    if (strcmp(node->func_call.fname, visitor->func_defs[i]->func_def.fname) == 0) {
      ast_t* fdef = visitor->func_defs[i];
      if (node->func_call.arg_size != fdef->func_def.arg_size) {
        char msg[128];
        sprintf(msg,
                "function \"%s\" requires %lu args, got %lu",
                node->func_call.fname,
                fdef->func_def.arg_size,
                node->func_call.arg_size);
        return visitor_error(visitor, msg);
      }
      // visit args
      size_t arg_size = node->func_call.arg_size;
      ast_t* args[arg_size];
      for (int j = 0; j < arg_size; j++) {
        args[j] = visitor_visit(visitor, scope, node->func_call.args[j]);
      }

      // add arg vals to local vars
      scope_t* loc_scope = init_scope();
      loc_scope->prev_scope = scope;
      for (int j = 0; j < arg_size; j++) {
        scope_add_var(loc_scope, init_var(fdef->func_def.args[j], args[j], true));
      }
      ast_t* return_val = visitor_visit(visitor, loc_scope, fdef->func_def.body);
      gc_free_scope(loc_scope); // free local scope after func call ends
      gc_free_visited_args(args, arg_size); // free visited args
      return visitor_visit(visitor, scope, return_val);
    }
  }

  // user defined objects
  for (int i = 0; i < visitor->obj_size; i++) {
    if (strcmp(node->func_call.fname, visitor->obj_defs[i]->obj_def.oname) == 0) {
      ast_t* odef = visitor->obj_defs[i];
      if (node->func_call.arg_size != odef->obj_def.param_size) {
        char msg[128];
        sprintf(msg,
                "object initialization \"%s\" requires %lu args, got %lu",
                node->func_call.fname,
                odef->obj_def.param_size,
                node->func_call.arg_size);
        return visitor_error(visitor, msg);
      }
      // visit args
      size_t arg_size = node->func_call.arg_size;
      ast_t* args[arg_size];
      for (int j = 0; j < arg_size; j++) {
        args[j] = visitor_visit(visitor, scope, node->func_call.args[j]);
      }

      // add arg vals to local vars
      scope_t* loc_scope = init_scope();
      for (int j = 0; j < arg_size; j++) {
        scope_add_var(loc_scope, init_var(odef->obj_def.params[j], args[j], true));
      }
      
      ast_t* obj = init_ast(AST_OBJECT);
      obj->object.def = odef;

      size_t field_size = odef->obj_def.field_size;
      obj->object.field_size = field_size;
      obj->object.field_vars  = calloc(field_size, sizeof(ast_t*));
      for (int j = 0; j < field_size; j++) {
        if (odef->obj_def.fields[j]->is_defined) {
          obj->object.field_vars[j] = visitor_visit(visitor, loc_scope, odef->obj_def.fields[j]->def);
          obj->object.field_vars[j]->ref_counter++;
        } else {
          obj->object.field_vars[j] = ast_null();
        }
      }
      gc_free_scope(loc_scope); // free local scope after object returns
      return obj;
    }
  }

  char msg[128];
  sprintf(msg, "call to undeclared function \"%s\"", node->func_call.fname);
  return visitor_error(visitor, msg);
}

ast_t* visitor_visit_int(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_string(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_float(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_bool(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_null(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_skip(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_stop(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  return node;
}

ast_t* visitor_visit_include(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  if (node->include.type == INC_SRC_FILE) {
    // read content from file
    const char* included_src = read_file(node->include.source_file.file_name);
    if (!included_src) {
      char msg[128];
      sprintf(msg, "\"%s\" could not be included", node->include.source_file.file_name);
      return visitor_error(visitor, msg);
    }

    // tokenize content
    lexer_t* included_lexer = init_lexer(included_src);
    lexer_collect_tokens(included_lexer);

    // generate ast
    parser_t* included_parser = init_parser(included_lexer);
    parser_parse(included_parser);

    // merge arrays
    visitor->func_defs = realloc(visitor->func_defs, (visitor->func_size + included_parser->func_size) * sizeof(ast_t*));
    for (int i = 0; i < included_parser->func_size; i++) {
      visitor->func_defs[visitor->func_size + i] = included_parser->func_defs[i];
    }
    visitor->func_size += included_parser->func_size;
    visitor->obj_defs = realloc(visitor->obj_defs, (visitor->obj_size + included_parser->obj_size) * sizeof(ast_t*));
    for (int i = 0; i < included_parser->obj_size; i++) {
      visitor->obj_defs[visitor->obj_size + i] = included_parser->obj_defs[i];
    }
    visitor->obj_size += included_parser->obj_size;

  } else if (node->include.type == INC_LIBSEAL) {
    for (int i = 0; i < visitor->libseal_size; i++) {
      if (strcmp(node->include.libseal.name, visitor->libseals[i]->name) == 0) {
        char msg[128];
        sprintf(msg, "libseal \"%s\" has already been included", node->include.libseal.name);
        return visitor_error(visitor, msg);
      }
    }
    visitor->libseal_size++;
    visitor->libseals = realloc(visitor->libseals, visitor->libseal_size * sizeof(libseal_t*));
    visitor->libseals[visitor->libseal_size - 1] = init_libseal(node->include.libseal.name,
                                                             node->include.libseal.has_alias,
                                                             node->include.libseal.alias_name);
  }

  return ast_noop();
}

ast_t* visitor_visit_list(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* ast = init_ast(AST_LIST);
  ast->list.mem_size = node->list.mem_size;
  ast->list.mems = calloc(ast->list.mem_size, sizeof(ast_t*));
  for (int i = 0; i < node->list.mem_size; i++) {
    ast->list.mems[i] = visitor_visit(visitor, scope, node->list.mems[i]);
  }
//  ast->is_static = true;

  return ast;
}

ast_t* visitor_visit_mem_acc(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* main = visitor_visit(visitor, scope, node->mem_acc.main);
  if (main->type != AST_OBJECT) {
    char msg[128];
    sprintf(msg, "unexpected type at member access main: \"%s\" %s", ast_name(main), node->mem_acc.main->var_ref.name);
    return visitor_error(visitor, msg);
  }
  if (node->mem_acc.mem->type != AST_VAR_REF) {
    char msg[128];
    sprintf(msg, "unexpected type at member access member: \"%s\"", ast_name(node->mem_acc.mem));
    return visitor_error(visitor, msg);
  }

  for (int i = 0; i < main->object.field_size; i++) {
    if (strcmp(node->mem_acc.mem->var_ref.name, main->object.def->obj_def.fields[i]->name) == 0) {
      return main->object.field_vars[i];
    }
  }

  char msg[128];
  sprintf(msg, "no such member in \"%s\" type: \"%s\"", main->object.def->obj_def.oname, node->mem_acc.mem->var_ref.name);
  return visitor_error(visitor, msg);
}

ast_t* visitor_visit_subscript(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* main = visitor_visit(visitor, scope, node->subscript.main);
  ast_t* ast_index = visitor_visit(visitor, scope, node->subscript.index);
  visitor_check_index(visitor, main, ast_index);
  int index = ast_index->integer.val;
  ast_t* result = (void*)0;

  switch (main->type) {
    case AST_STRING:
      result = init_ast(AST_STRING);
      result->string.val = calloc(2, sizeof(char));
      result->string.val[0] = main->string.val[index];
      result->string.val[1] = '\0';
      break;
    case AST_LIST:
      result = main->list.mems[index];
      break;
    default: {
      char msg[128];
      sprintf(msg, "%s is not subscriptable", ast_name(main));
      return visitor_error(visitor, msg);
    }
  }

  return result;
}

ast_t* visitor_visit_libseal_fcall(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* libseal = node->libseal_fcall.libseal;
  if (libseal->type != AST_VAR_REF) {
    char msg[128];
    sprintf(msg, "unexpected type at libseal function call libseal: \"%s\"", ast_name(libseal));
    return visitor_error(visitor, msg);
  }
  if (node->libseal_fcall.func_call->type != AST_FUNC_CALL) {
    char msg[128];
    sprintf(msg, "unexpected type at libseal function call function: \"%s\"", ast_name(node->libseal_fcall.func_call));
    return visitor_error(visitor, msg);
  }

  for (int i = 0; i < visitor->libseal_size; i++) {
    if (strcmp(libseal->var_ref.name, visitor->libseals[i]->name) == 0) {
      size_t arg_size = node->libseal_fcall.func_call->func_call.arg_size;
      ast_t* args[arg_size];
      for (int i = 0; i < arg_size; i++) {
        args[i] = visitor_visit(visitor, scope, node->libseal_fcall.func_call->func_call.args[i]);
      }
      ast_t* res = libseal_function_call(visitor->libseals[i], node->libseal_fcall.func_call->func_call.fname, args, arg_size);
      gc_free_visited_args(args, arg_size);
     /* for (int j = 0; j < arg_size; j++) {
        //free(args[j]);
      }
      free(args);
      args = (void*)0; */
      //printf("%d\n", args);
      
      /*
      printf("%s: %zu", ast_name(res), res->list.mem_size);
      for (int i = 0; i < res->list.mem_size; i++) {
        free(res->list.mems[i]);
        res->list.mems[i] = NULL;
      }
      free(res->list.mems);
      res->list.mems = NULL;
      free(res);
      res = NULL;
      getchar();
      */
      return res;
    }
  }

  char msg[128];
  sprintf(msg, "libseal \"%s\" is not included", libseal->var_ref.name);
  return visitor_error(visitor, msg);
}

static ast_t* binary_op_error(visitor_t* visitor, const char* type_name, Token_Type op)
{
  char msg[128];
  sprintf(msg, "%s operator cannot be applied to %s", token_name(op), type_name);
  return visitor_error(visitor, msg);
}

ast_t* visitor_visit_binary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  if (node->binary.op == TOK_AND || node->binary.op == TOK_OR) {
    ast_t* bin_left = visitor_visit(visitor, scope, node->binary.left);
    if (bin_left->type != AST_BOOL) {
      char msg[128];
      sprintf(msg, "%s operator expected left to be %s instead of \"%s\"",
              token_name(node->binary.op), "bool", ast_name(bin_left));
      return visitor_error(visitor, msg);
    }
    bool left = bin_left->boolean.val;
    if (node->binary.op == TOK_AND) {
      if (!left) {
        return ast_false();
      }
    } else { // or
      if (left) {
        return ast_true();
      }
    }
    ast_t* bin_right = visitor_visit(visitor, scope, node->binary.right);
    if (bin_right->type != AST_BOOL) {
      char msg[128];
      sprintf(msg, "%s operator expected right to be %s instead of \"%s\"",
              token_name(node->binary.op), "bool", ast_name(bin_right));
      return visitor_error(visitor, msg);
    }
    bool right = bin_right->boolean.val;

    if (node->binary.op == TOK_AND) {
      return left && right ? ast_true() : ast_false();
    } else { // or
      return left || right ? ast_true() : ast_false();
    }
  }

  ast_t* bin_left  = visitor_visit(visitor, scope, node->binary.left);
  ast_t* bin_right = visitor_visit(visitor, scope, node->binary.right);

  if (bin_left->type == AST_INT && bin_right->type == AST_INT) {
    int left  = bin_left->integer.val;
    int right = bin_right->integer.val;
    int result;

    switch (node->binary.op) {
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
      default: {
        binary_op_error(visitor, "int", node->binary.op);
      }
    }
    ast_t* ast = init_ast(AST_INT);
    ast->integer.val = result;
    return ast;
  } else if (bin_left->type == AST_FLOAT && bin_right->type == AST_FLOAT) {
    float left  = bin_left->floating.val;
    float right = bin_right->floating.val;
    float result;

    switch (node->binary.op) {
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
      default: {
        binary_op_error(visitor, "float", node->binary.op);
      }
    }
    ast_t* ast = init_ast(AST_FLOAT);
    ast->floating.val = result;
    return ast;
  } else if ((bin_left->type == AST_INT   && bin_right->type == AST_FLOAT) ||
             (bin_left->type == AST_FLOAT && bin_right->type == AST_INT)) {
    float left  = bin_left->type == AST_INT ? (float)bin_left->integer.val : bin_left->floating.val;
    float right = bin_right->type == AST_INT ? (float)bin_right->integer.val : bin_right->floating.val;
    float result;

    switch (node->binary.op) {
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
      default: {
        binary_op_error(visitor, "float", node->binary.op);
      }
    }
    ast_t* ast = init_ast(AST_FLOAT);
    ast->floating.val = result;
    return ast;
  } else if (bin_left->type == AST_STRING && bin_right->type == AST_STRING) {
    char* left  = bin_left->string.val;
    char* right = bin_right->string.val;
    char* result;

    switch (node->binary.op) {
      case TOK_PLUS: {
        int len = strlen(left) + strlen(right);
        result = calloc(len + 1, sizeof(char));
        strcat(result, left);
        strcat(result, right);
        result[len] = '\0';
        break;
      }
      case TOK_EQ:
        return strcmp(left, right) == 0 ? ast_true() : ast_false();
      case TOK_NE:
        return strcmp(left, right) != 0 ? ast_true() : ast_false();
      default: {
        binary_op_error(visitor, "string", node->binary.op);
      }
    }
    ast_t* ast = init_ast(AST_STRING);
    ast->string.val = result;
    return ast;
  } else if (bin_left->type == AST_BOOL && bin_right->type == AST_BOOL) {
    bool left  = bin_left->boolean.val;
    bool right = bin_right->boolean.val;

    switch (node->binary.op) {
      case TOK_EQ:
        return left == right ? ast_true() : ast_false();
      case TOK_NE:
        return left != right ? ast_true() : ast_false();
      default: {
        binary_op_error(visitor, "bool", node->binary.op);
      }
    }
  } else if (bin_left->type == AST_OBJECT && bin_right->type == AST_OBJECT) {
    switch (node->binary.op) {
      case TOK_EQ:
        return &bin_left->object == &bin_right->object ? ast_true() : ast_false();
      case TOK_NE:
        return &bin_left->object != &bin_right->object ? ast_true() : ast_false();
      default: {
        binary_op_error(visitor, "object", node->binary.op);
      }
    }
  } else if (bin_left->type == AST_NULL || bin_right->type == AST_NULL) {
    switch (node->binary.op) {
      case TOK_EQ:
        return bin_left->type == bin_right->type ? ast_true() : ast_false();
      case TOK_NE:
        return bin_left->type != bin_right->type ? ast_true() : ast_false();
      default: {
        binary_op_error(visitor, "null", node->binary.op);
      }
    }
  }

  char msg[128];
  sprintf(msg, "unexpected types in binary: left: %s, right: %s", ast_name(bin_left), ast_name(bin_right));
  return visitor_error(visitor, msg);
}

ast_t* visitor_visit_unary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* expr = visitor_visit(visitor, scope, node->unary.expr);
  ast_t* ast = (void*)0;

  switch (node->unary.op) {
    case TOK_MINUS:
      switch (expr->type) {
        case AST_INT:
          ast = init_ast(AST_INT);
          ast->integer.val = -expr->integer.val;
          break;
        case AST_FLOAT:
          ast = init_ast(AST_FLOAT);
          ast->floating.val = -expr->floating.val;
          break;
        default: {
          char msg[128];
          sprintf(msg, "unexpected type for unary \"-\" operator: %s", ast_name(expr));
          return visitor_error(visitor, msg);
        }
      }
      break;
    case TOK_NOT:
      switch (expr->type) {
        case AST_BOOL:
          ast = expr->boolean.val ? ast_false() : ast_true();
          break;
        default: {
          char msg[128];
          sprintf(msg, "unexpected type for unary \"!(not)\" operator: %s", ast_name(expr));
          return visitor_error(visitor, msg);
        }
      }
      break;
    default: {
      char msg[128];
      sprintf(msg, "unexpected unary operator: %s", token_name(node->unary.op));
      return visitor_error(visitor, msg);
    }
  }

  return ast;
}

ast_t* visitor_visit_ternary(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* cond = visitor_visit(visitor, scope, node->ternary.cond);
  bool should_ev;
  switch (cond->type) {
    case AST_BOOL:
      should_ev = cond->boolean.val == true;
      break;
    case AST_INT:
      should_ev = cond->integer.val != 0;
      break;
    case AST_FLOAT:
      should_ev = cond->floating.val != 0;
      break;
    default: {
      char msg[128];
      sprintf(msg, "unexpected type at ternary condition: \"%s\"", ast_name(cond));
      return visitor_error(visitor, msg);
    }
  }

  return visitor_visit(visitor, scope, should_ev ? node->ternary.true_branch : node->ternary.false_branch);
}

ast_t* visitor_visit_vardef(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  for (int i = 0; i < node->vardef.def_size; i++) {
    // check if object is defined before
    for (int j = 0 ; j < visitor->obj_size; j++) {
      if (strcmp(node->vardef.names[i], visitor->obj_defs[j]->obj_def.oname) == 0) {
        char msg[128];
        sprintf(msg, "redefinition of object \"%s\"", node->vardef.names[i]);
        return visitor_error(visitor, msg);
      }
    }
    for (int j = 0 ; j < visitor->func_size; j++) {
      if (strcmp(node->vardef.names[i], visitor->func_defs[j]->func_def.fname) == 0) {
        char msg[128];
        sprintf(msg, "redefinition of function \"%s\"", node->vardef.names[i]);
        return visitor_error(visitor, msg);
      }
    }
    scope_add_var(scope,
                  init_var(node->vardef.names[i],
                           node->vardef.is_defined_arr[i] ? visitor_visit(visitor, scope, node->vardef.rights[i]) : (void*)0,
                           node->vardef.is_defined_arr[i])
                 );
  }

  return ast_noop();
}

ast_t* visitor_visit_var_ref(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* var = scope_get_var(scope, node->var_ref.name);
  return var->variable.val;
}

ast_t* visitor_visit_assign(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* var;
  ast_t* assign_expr = visitor_visit(visitor, scope, node->assign.right);
  assign_expr->ref_counter++; // increment reference counter
  switch (node->assign.var->type) {
    case AST_VAR_REF:
      var = scope_get_var(scope, node->assign.var->var_ref.name);
      var->variable.val->ref_counter--;
      var->variable.val = assign_expr;
      break;
    case AST_SUBSCRIPT: {
      var = visitor_visit(visitor, scope, node->assign.var->subscript.main);
      ast_t* ast_index = visitor_visit(visitor, scope, node->assign.var->subscript.index);
      visitor_check_index(visitor, var, ast_index);
      if (var->type != AST_LIST) {
        char msg[128];
        sprintf(msg, "unexpected var type at visit assign: \"%s\"", ast_name(var));
        return visitor_error(visitor, msg);
      }
      var->list.mems[ast_index->integer.val]->ref_counter--;
      var->list.mems[ast_index->integer.val] = assign_expr;
      break;
    }
    case AST_MEM_ACC: {
      var = visitor_visit(visitor, scope, node->assign.var->mem_acc.main);
      ast_t* mem = node->assign.var->mem_acc.mem;
      if (var->type != AST_OBJECT) {
        char msg[128];
        sprintf(msg, "unexpected var type at visit assign member access main: \"%s\"", ast_name(var));
        return visitor_error(visitor, msg);
      }
      if (mem->type != AST_VAR_REF) {
        char msg[128];
        sprintf(msg, "unexpected var type at visit assign member access member: \"%s\"", ast_name(mem));
        return visitor_error(visitor, msg);
      }
      bool is_found = false;
      for (int i = 0; i < var->object.field_size; i++) {
        if (strcmp(mem->var_ref.name, var->object.def->obj_def.fields[i]->name) == 0) {
          var->object.field_vars[i]->ref_counter--;
          var->object.field_vars[i] = assign_expr;
          is_found = true;
          break;
        }
      }
      if (is_found) break;
      char msg[128];
      sprintf(msg, "no such member in \"%s\" type: \"%s\"", var->object.def->obj_def.oname, mem->var_ref.name);
      return visitor_error(visitor, msg);
      break;
    }
    default: {
      char msg[128];
      sprintf(msg, "unexpected var type at visit assign: \"%s\"", ast_name(node->assign.var));
      return visitor_error(visitor, msg);
    }
  }
  
  return assign_expr;
}

ast_t* visitor_visit_if(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* cond = visitor_visit(visitor, scope, node->__if.cond);
  bool should_ev;
  switch (cond->type) {
    case AST_BOOL:
      should_ev = cond->boolean.val == true;
      break;
    case AST_INT:
      should_ev = cond->integer.val != 0;
      break;
    case AST_FLOAT:
      should_ev = cond->floating.val != 0;
      break;
    default: {
      char msg[128];
      sprintf(msg, "unexpected type at if condition: \"%s\"", ast_name(cond));
      return visitor_error(visitor, msg);
    }
  }
  if (should_ev) {
    scope_t* loc_scope = init_scope();
    loc_scope->prev_scope = scope;
    ast_t* res = visitor_visit(visitor, loc_scope, node->__if.body);
    gc_free_scope(loc_scope); // free local scope after if block ends
    return res;
  } else if (node->__if.has_else) {
    return visitor_visit(visitor, scope, node->__if.else_part);
  }

  return ast_noop();
}

ast_t* visitor_visit_else(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  scope_t* loc_scope = init_scope();
  loc_scope->prev_scope = scope;
  ast_t* res = visitor_visit(visitor, loc_scope, node->__else.body);
  gc_free_scope(loc_scope); // free local scope after else block ends
  return res;
}

ast_t* visitor_visit_while(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  loop: {
    ast_t* cond = visitor_visit(visitor, scope, node->__while.cond);
    bool should_ev = false;
    switch (cond->type) {
      case AST_BOOL:
        should_ev = cond->boolean.val == true;
        break;
      case AST_INT:
        should_ev = cond->integer.val != 0;
        break;
      case AST_FLOAT:
        should_ev = cond->floating.val != 0;
        break;
      default: {
        char msg[128];
        sprintf(msg, "unexpected type at while condition: \"%s\"", ast_name(cond));
        return visitor_error(visitor, msg);
      }
      gc_free_node(cond);
    }
    if (should_ev) {
      scope_t* loc_scope = init_scope();
      loc_scope->prev_scope = scope;
      ast_t* visited = visitor_visit(visitor, loc_scope, node->__while.body);
      gc_free_scope(loc_scope); // free local scope after while block ends
      switch (visited->type) {
        case AST_RETURN_VAL:
          return visited;
        case AST_STOP:
          return ast_noop();
        default:
          break;
      }
      goto loop;
    }
  }

  return ast_noop();
}

ast_t* visitor_visit_for(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* iterated = visitor_visit(visitor, scope, node->__for.iterated);
  int max_index;
  int index = 0;

  switch (iterated->type) {
    case AST_STRING:
      max_index = strlen(iterated->string.val);
      break;
    case AST_LIST:
      max_index = iterated->list.mem_size;
      break;
    default: {
      char msg[128];
      sprintf(msg, "\"%s\" is not iterable", ast_name(iterated));
      return visitor_error(visitor, msg);
    }
  }

  if (max_index == 0) return ast_noop();

  scope_t* for_scope = init_scope();
  for_scope->prev_scope = scope;
  scope = for_scope;
  ast_t* iterator_var = init_var(node->__for.name_iterator, (void*)0, false);
  scope_add_var(scope, iterator_var);
  ast_t* cur_iter = (void*)0;
  loop: {
    switch (iterated->type) {
      case AST_STRING:
        cur_iter = init_ast(AST_STRING);
        cur_iter->string.val = calloc(2, sizeof(char));
        cur_iter->string.val[0] = iterated->string.val[index];
        cur_iter->string.val[1] = '\0';
        break;
      case AST_LIST:
        cur_iter = iterated->list.mems[index];
        break;
      default:
        break;
    }
    cur_iter->ref_counter++; // increment iterator val
    iterator_var->variable.val = cur_iter;

    scope_t* loc_scope = init_scope();
    loc_scope->prev_scope = scope;
    ast_t* visited = visitor_visit(visitor, loc_scope, node->__for.body);

    cur_iter->ref_counter--; // decrement iterator val
    switch (visited->type) {
      case AST_RETURN_VAL:
        return visited;
      case AST_STOP:
        return ast_noop();
      default:
        break;
    }
    gc_free_scope(loc_scope);
    if (++index < max_index) goto loop;
  }
  iterator_var->variable.val = ast_null();
  gc_free_scope(for_scope);
  gc_free_node(iterated);
  return ast_noop();
}

ast_t* visitor_visit_return(visitor_t* visitor, scope_t* scope, ast_t* node)
{
  ast_t* ast = init_ast(AST_RETURN_VAL);
  if (node->__return.is_empty) {
    ast->return_val.val = ast_null();
    return ast;
  } 
  ast->return_val.val = visitor_visit(visitor, scope, node->__return.expr);
  return ast;
}
