#include "../src/libdef.h"
#include "../src/ast.h"
#include "../src/listseal.h"
#include <stdio.h>
#include <string.h>

static const char* libname = "std";

sealobj* _initlib(sealobj** args, size_t arg_size)
{
  init_const_asts();
  return ast_noop();
}

static inline void write(sealobj* arg)
{
  switch (arg->type) {
    case SEAL_INT:
      printf("%d", arg->integer.val);
      break;
    case SEAL_FLOAT:
      printf("%f", arg->floating.val);
      break;
    case SEAL_STRING:
      printf("%s", arg->string.val);
      break;
    case SEAL_BOOL:
      printf("%s", arg->boolean.val ? "true" : "false");
      break;
    case SEAL_LIST:
      printf("[");
      for (int i = 0; i < arg->list.mem_size; i++) {
        write(arg->list.mems[i]);
        printf("%s", arg->list.mem_size - 1 == i ? "\0" : ", ");
      }
      printf("]");
      break;
    case SEAL_OBJECT:
      printf("%s: %p", arg->object.def->obj_def.oname, &(arg->object));
      break;
    default: {
      printf("function writeln: unexpected arg: \"%s\"\n", seal_type_name((seal_type)arg->type));
      exit(1);
    }
  }
}

sealobj* _writeln(sealobj** args, size_t arg_size)
{
  for (int i = 0; i < arg_size; i++) {
    sealobj* arg = args[i];
    write(arg);
    printf(" ");
  }
  printf("\n");
  return ast_noop();
}

sealobj* _read(sealobj** args, size_t arg_size)
{
  seal_type expected_types[1];
  size_t type_size;
  switch (arg_size) {
    case 0:
      type_size = 0;
      break;
    default:
      expected_types[0] = SEAL_STRING;
      type_size = 1;
      break;
  }
  seal_check_args(libname, "read", expected_types, type_size, args, arg_size);

  sealobj* sobj = init_sealobj(AST_STRING);
  sobj->string.val = calloc(1, sizeof(char));
  
  size_t len = 0;

  switch (type_size) {
    case 0:
      break;
    default:
      printf("%s", args[0]->string.val);
      break;
  }

  char c;
  while ((c = getchar()) != '\n' && c != EOF) {
    len++;
    sobj->string.val = realloc(sobj->string.val, (len + 1) * sizeof(char));
    sobj->string.val[len - 1] = c;
  }
  sobj->string.val[len] = '\0';

  return sobj;
}

sealobj* _len(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_ITERABLE };
  seal_check_args(libname, "len", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  sealobj* sobj = init_sealobj(AST_INT);
  sobj->integer.val = args[0]->type == AST_LIST ? args[0]->list.mem_size : strlen(args[0]->string.val);

  return sobj;
}

sealobj* _span(sealobj** args, size_t arg_size)
{
  seal_type expected_types[3];
  size_t type_size;
  switch (arg_size) {
    case 1:
      expected_types[0] = SEAL_INT;
      type_size = 1;
      break;
    case 2:
      expected_types[0] = SEAL_INT;
      expected_types[1] = SEAL_INT;
      type_size = 2;
      break;
    default:
      expected_types[0] = SEAL_INT;
      expected_types[1] = SEAL_INT;
      expected_types[2] = SEAL_INT;
      type_size = 3;
      break;
  }
  seal_check_args(libname, "span", expected_types, type_size, args, arg_size);

  sealobj* sobj = init_sealobj(AST_LIST);
  sobj->list.mem_size = 0;
  sobj->list.mems = (void*)0;

  int start, step, end;

  switch (type_size) {
    case 1:
      start = 0;
      end = args[0]->integer.val;
      step = 1;
      break;
    case 2:
      start = args[0]->integer.val;
      end = args[1]->integer.val;
      step = 1;
      break;
    default:
      start = args[0]->integer.val;
      end = args[1]->integer.val;
      step = args[2]->integer.val;
      break;
  }

  if (step == 0) return sobj;

  if (start < end && step > 0) {
    for (int i = start; i < end; i += step) {
      ast_t* mem = init_ast(AST_INT);
      mem->integer.val = i;
      list_push(sobj, mem);
    }
  } else if (start > end && step < 0) {
    for (int i = start; i > end; i += step) {
      ast_t* mem = init_ast(AST_INT);
      mem->integer.val = i;
      list_push(sobj, mem);
    }
  }

  return sobj;
}
