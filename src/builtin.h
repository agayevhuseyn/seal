#ifndef SEAL_BUILTIN_H
#define SEAL_BUILTIN_H

#include "seal.h"

#include "ast.h"
#include "gc.h"

static inline ast_t* func_error(const char* name, ast_t* fcall, const char* err)
{
  fprintf(stderr, "seal: at line: %d\nerror: %s in function \'%s\'\n", fcall->line, err, name);
  exit(1);
  return ast_null();
}

static ast_t* builtin_writeln(ast_t* args[], size_t arg_size, bool is_main, bool is_single)
{
  for (int i = 0; i < arg_size; i++) {
    ast_t* arg = args[i];
    switch (arg->type) {
      case AST_NULL:
        printf("%s", "null");
        goto space;
      case AST_INT:
        printf("%d", arg->integer.val);
        goto space;
      case AST_FLOAT:
        printf("%f", arg->floating.val);
        goto space;
      case AST_STRING:
        if (is_single)
          printf("%s", arg->string.val);
        else
          printf("\'%s\'", arg->string.val);
        goto space;
      case AST_BOOL:
        printf("%s", arg->boolean.val ? "true" : "false");
        goto space;
      case AST_LIST:
        printf("[");
        for (int j = 0; j < arg->list.mem_size; j++) {
          builtin_writeln(&arg->list.mems[j], 1, false, false);
          if (j < arg->list.mem_size - 1) {
            printf(", ");
          }
        }
        printf("]");
        goto space;
      case AST_OBJECT:
        if (!arg->object.is_lit) {
          printf("%s: %p", arg->object.def->struct_def.name, &arg->object);
          goto space;
        }
        printf("{");
        for (int j = 0; j < arg->object.field_size; j++) {
          printf("%s = ", arg->object.field_names[j]);
          builtin_writeln(&arg->object.field_vals[j], 1, false, false);
          if (j < arg->object.field_size - 1) {
            printf(", ");
          }
        }
        printf("}");
        goto space;
      default:
        printf("UNRECOGNIZED TYPE: \'%s\'", hast_type_name(arg->type));
    }
    space:
    if (is_main) printf(" ");
  }
  if (is_main) printf("\n");
  return ast_null();
}

static ast_t* builtin_readln()
{
  ast_t* line = create_ast(AST_STRING);
  line->string.val = (char*)SEAL_CALLOC(1, sizeof(char));
  
  size_t len = 0;
  char c;
  while ((c = getchar()) != '\n' && c != EOF) {
    len++;
    line->string.val = (char*)SEAL_REALLOC(line->string.val, (len + 1) * sizeof(char));
    line->string.val[len - 1] = c;
  }
  line->string.val[len] = '\0';

  return line;
}

static inline ast_t* builtin_len(ast_t* arg)
{
  ast_t* len = create_ast(AST_INT);
  len->integer.val = arg->type == AST_STRING ? strlen(arg->string.val) : arg->list.mem_size;
  return len;
}

static inline ast_t* builtin_push(ast_t* args[2])
{
  args[0]->list.mem_size++;
  args[0]->list.mems = (ast_t**)SEAL_REALLOC(args[0]->list.mems, args[0]->list.mem_size * sizeof(ast_t*));
  args[0]->list.mems[args[0]->list.mem_size - 1] = args[1];
  gc_retain(args[1]);
  return ast_null();
}

static inline ast_t* builtin_pop(ast_t* arg, ast_t* fcall)
{
  if (arg->list.mem_size == 0) {
    func_error("pop", fcall, "pop from empty list");
  }
  arg->list.mem_size--;
  ast_t* popped = arg->list.mems[arg->list.mem_size];
  arg->list.mems = (ast_t**)SEAL_REALLOC(arg->list.mems, arg->list.mem_size * sizeof(ast_t*));
  return popped;
}

#endif
