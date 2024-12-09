#include "ast.h"
#include <stdio.h>

ast_t* g_ast_noop  = (void*)0;
ast_t* g_ast_true  = (void*)0;
ast_t* g_ast_false = (void*)0;
ast_t* g_ast_null  = (void*)0;

void print_ast(ast_t* ast)
{
  switch (ast->type) {
    case AST_MODULE_FCALL:
      printf("%s, module:\n", ast_name(ast));
      print_ast(ast->module_fcall.module);
      printf("%s, fcall:\n", ast_name(ast));
      print_ast(ast->module_fcall.func_call);
      break;
    case AST_INCLUDE:
      printf("%s, name: %s\n", ast_name(ast), ast->include.module_name);
      break;
    case AST_WHILE:
      printf("%s\n", ast_name(ast));
      printf("\tcond:\n");
      print_ast(ast->__while.cond);
      printf("\tbody:\n");
      print_ast(ast->__while.body);
      break;
    case AST_ELSE:
      printf("%s\n", ast_name(ast));
      printf("\tbody:\n");
      print_ast(ast->__else.body);
      break;
    case AST_IF:
      printf("%s\n", ast_name(ast));
      printf("\tcond:\n");
      print_ast(ast->__if.cond);
      printf("\tbody:\n");
      print_ast(ast->__if.body);
      if (ast->__if.has_else) {
        printf("else part:\n");
        print_ast(ast->__if.else_part);
      }
      break;
    case AST_FUNC_DEF:
      printf("%s, func def name: %s, arg size: %lu\n", ast_name(ast), ast->func_def.fname, ast->func_def.arg_size);
      for (int i = 0; i < ast->func_def.arg_size; i++) {
        printf("arg: %d, name: %s\n", i, ast->func_def.args[i]);
      }
      printf("body: ");
      print_ast(ast->func_def.body);
      break;
    case AST_VARDEF:
      printf("%s, vardef size: %lu\n", ast_name(ast), ast->vardef.def_size);
      for (int i = 0; i < ast->vardef.def_size; i++) {
        printf("\t%s ", ast_name(ast));
        printf("name: %s", ast->vardef.names[i]);
        if (ast->vardef.is_defined_arr[i]) {
          printf("\n\t rightside: ");
          print_ast(ast->vardef.rights[i]);
        } else {
          printf("\n");
        }
      }
      break;
    case AST_OBJ_DEF:
      printf("%s, object name: %s, field size: %lu, init func:\n", ast_name(ast), ast->obj_def.oname, ast->obj_def.field_size);
      for (int i = 0; i < ast->obj_def.param_size; i++) {
        printf("%d arg name: %s\n", i, ast->obj_def.params[i]);
      }
      printf("fields:\n");
      for (int i = 0; i < ast->obj_def.field_size; i++) {
        printf("\t%s ", ast_name(ast));
        printf("name: %s", ast->obj_def.fields[i]->name);
        if (ast->obj_def.fields[i]->is_defined) {
          printf("\n\t rightside: ");
          print_ast(ast->obj_def.fields[i]->def);
        } else {
          printf("\n");
        }
      }
      break;
    case AST_LIST:
      printf("%s, mem size: %lu, mems:\n", ast_name(ast), ast->list.mem_size);
      for (int i = 0; i < ast->list.mem_size; i++) {
        printf("\t");
        print_ast(ast->list.mems[i]);
      }
      break;
    case AST_OBJECT:
      printf("%s, field size: %lu, fields:\n", ast_name(ast), ast->object.field_size);
      for (int i = 0; i < ast->object.field_size; i++) {
        printf("\tfield name: %s\n\t", ast->object.field_names[i]);
        print_ast(ast->object.field_vars[i]);
      }
      break;
    case AST_MEM_ACC:
      printf("%s, main:\n", ast_name(ast));
      print_ast(ast->mem_acc.main);
      printf("%s mem:\n", ast_name(ast));
      print_ast(ast->mem_acc.mem);
      break;
    case AST_SUBSCRIPT:
      printf("%s, main:\n", ast_name(ast));
      print_ast(ast->subscript.main);
      printf("%s index:\n", ast_name(ast));
      print_ast(ast->subscript.index);
      break;
    case AST_RETURN:
      printf("%s", ast_name(ast));
      if (!ast->__return.is_empty) {
        printf(" expr: ");
        print_ast(ast->__return.expr);
      } else {
        printf("\n");
      }
      break;
    case AST_ASSIGN:
      printf("%s\n", ast_name(ast));
      printf("%s var:\n", ast_name(ast));
      print_ast(ast->assign.var);
      printf("%s right:\n", ast_name(ast));
      print_ast(ast->assign.right);
      break;
    case AST_VAR_REF:
      printf("%s, var: %s\n", ast_name(ast), ast->var_ref.name);
      break;
    case AST_VARIABLE:
      printf("%s, var: %s\n", ast_name(ast), ast->variable.name);
      printf("\t val:");
      if (ast->variable.is_defined) {
        print_ast(ast->variable.val);
      } else {
        printf("undefined\n");
      }
      break;
    case AST_BINARY:
      printf("%s, op: %s\n", ast_name(ast), token_name(ast->binary.op));
      printf("%s left:\n", ast_name(ast));
      print_ast(ast->binary.left);
      printf("%s right:\n", ast_name(ast));
      print_ast(ast->binary.right);
      break;
    case AST_UNARY:
      printf("%s, op: %s\n", ast_name(ast), token_name(ast->unary.op));
      printf("%s expr:\n", ast_name(ast));
      print_ast(ast->unary.expr);
      break;
    case AST_COMP:
      printf("%s, size: %lu\n", ast_name(ast), ast->comp.stmt_size);
      for (int i = 0; i < ast->comp.stmt_size; i++) {
        print_ast(ast->comp.stmts[i]);
      }
      break;
    case AST_FUNC_CALL:
      printf("%s, name: %s, arg size: %lu\n", ast_name(ast), ast->func_call.fname, ast->func_call.arg_size);
      if (ast->func_call.arg_size <= 0) break;
      printf("args:\n");
      for (int i = 0; i < ast->func_call.arg_size; i++) {
        printf("\t");
        print_ast(ast->func_call.args[i]);
      }
      break;
    case AST_INT:
      printf("%s, value: %d\n", ast_name(ast), ast->integer.val);
      break;
    case AST_FLOAT:
      printf("%s, value: %f\n", ast_name(ast), ast->floating.val);
      break;
    case AST_STRING:
      printf("%s, value: %s\n", ast_name(ast), ast->string.val);
      break;
    case AST_BOOL:
      printf("%s, value: %s\n", ast_name(ast), ast->boolean.val ? "true" : "false");
      break;
    case AST_STOP:
      printf("%s\n", ast_name(ast));
      break;
    case AST_SKIP:
      printf("%s\n", ast_name(ast));
      break;
    case AST_NULL:
      printf("%s\n", ast_name(ast));
      break;
    case AST_NOOP:
      printf("%s\n", ast_name(ast));
      break;
  }
}

const char* ast_name(ast_t* ast)
{
  switch (ast->type) {
    case AST_NOOP: return "AST_NOOP";
    case AST_NULL: return "AST_NULL";
    case AST_COMP: return "AST_COMP";
    case AST_FUNC_CALL: return "AST_FUNC_CALL";
    case AST_INT: return "AST_INT";
    case AST_STRING: return "AST_STRING";
    case AST_BINARY: return "AST_BINARY";
    case AST_UNARY: return "AST_UNARY";
    case AST_MEM_ACC: return "AST_MEM_ACC";
    case AST_SUBSCRIPT: return "AST_SUBSCRIPT";
    case AST_FLOAT: return "AST_FLOAT";
    case AST_VARIABLE: return "AST_VARIABLE";
    case AST_ASSIGN: return "AST_ASSIGN";
    case AST_LIST: return "AST_LIST";
    case AST_VARDEF: return "AST_VARDEF";
    case AST_FUNC_DEF: return "AST_FUNC_DEF";
    case AST_BOOL: return "AST_BOOL";
    case AST_RETURN: return "AST_RETURN";
    case AST_IF: return "AST_IF";
    case AST_ELSE: return "AST_ELSE";
    case AST_WHILE: return "AST_WHILE";
    case AST_SKIP: return "AST_SKIP";
    case AST_STOP: return "AST_STOP";
    case AST_OBJ_DEF: return "AST_OBJ_DEF";
    case AST_INCLUDE: return "AST_INCLUDE";
    case AST_MODULE_FCALL: return "AST_MODULE_FCALL";
    case AST_VAR_REF: return "AST_VAR_REF";
    case AST_RETURN_VAL: return "AST_RETURN_VAL";
    case AST_OBJECT: return "AST_OBJECT";
  }
}

ast_t* init_ast(AST_Type type)
{
  ast_t* ast = calloc(1, sizeof(ast_t));

  ast->type = type;

  return ast;
}

ast_t* ast_noop()
{
  if (!g_ast_noop) {
    g_ast_noop = init_ast(AST_NOOP);
  }
  return g_ast_noop;
}

ast_t* ast_true()
{
  if (!g_ast_true) {
    g_ast_true = init_ast(AST_BOOL);
    g_ast_true->boolean.val = true;
  }
  return g_ast_true;
}

ast_t* ast_false()
{
  if (!g_ast_false) {
    g_ast_false = init_ast(AST_BOOL);
    g_ast_false->boolean.val = false;
  }
  return g_ast_false;
}

ast_t* ast_null()
{
  if (!g_ast_null) {
    g_ast_null = init_ast(AST_NULL);
  }
  return g_ast_null;
}
