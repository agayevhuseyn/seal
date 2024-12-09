#include "builtin.h"
#include <stdio.h>
#include <string.h>

ast_t* builtin_write(ast_t** args, size_t arg_size)
{
  for (int i = 0; i < arg_size; i++) {
    ast_t* arg = args[i];
    switch (arg->type) {
      case AST_INT:
        printf("%d ", arg->integer.val);
        break;
      case AST_STRING:
        printf("%s ", arg->string.val);
        break;
      case AST_FLOAT:
        printf("%f ", arg->floating.val);
        break;
      case AST_BOOL:
        printf("%s ", arg->boolean.val ? "true" : "false");
        break;
      case AST_LIST:  
        printf("[\n");
        for (int j = 0; j < arg->list.mem_size; j++) {
          builtin_write(&arg->list.mems[j], 1);
        }
        printf("\n]");
        break;
      case AST_NULL:  
        printf("%s ", "null");
        break;
      case AST_OBJECT:
        printf("object %s: {\n", arg->object.def->obj_def.oname);
        for (int j = 0; j < arg->object.field_size; j++) {
          printf("\t");
          printf("%s: ", arg->object.def->obj_def.fields[j]->name);
          builtin_write(&arg->object.field_vars[j], 1);
        }
        printf("}");
        break;
      default: {
        printf("function write: unexpected arg: \"%s\"\n", ast_name(arg));
        exit(1);
      }
    }
  }
  printf("\n");
  return ast_null();
}

ast_t* builtin_len(ast_t** args, size_t arg_size)
{
  if (arg_size != 1) {
    printf("function len: expected 1 arg, got %lu\n", arg_size);
    exit(1);
  }
  ast_t* arg = args[0];
  ast_t* ast = init_ast(AST_INT);
  switch (arg->type) {
    case AST_STRING:
      ast->integer.val = strlen(arg->string.val);
      break;
    case AST_LIST:
      ast->integer.val = arg->list.mem_size;
      break;
    default: {
      printf("function len: unexpected arg type: \"%s\"\n", ast_name(arg));
      exit(1);
    }
  }
  return ast;
}
