#include <seal/src/libdef.h>
#include <seal/src/ast.h>
#include <stdlib.h>
#include <time.h>

static const char* libname = "random";

sealobj* _initlib(sealobj** args, size_t arg_size)
{
  init_const_asts();
  srand(time((void*)0));
  return ast_noop();
}

sealobj* _int(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_INT, SEAL_INT };
  seal_check_args(libname, "int", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  sealobj* sobj = init_sealobj(AST_INT);
  int min = args[0]->integer.val, max = args[1]->integer.val;
  int result = rand() % (max - min + 1) + min;
  sobj->integer.val = result;

  return sobj;
}
