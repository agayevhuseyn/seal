#include <seal/src/libdef.h>
#include <seal/src/ast.h>
#include <seal/src/listseal.h>
#include <stdio.h>
#include <string.h>

static const char* libname = "list";

sealobj* _initlib(sealobj** args, size_t arg_size)
{
  init_const_asts();
  return ast_noop();
}

sealobj* _push(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_LIST, SEAL_DATA };
  seal_check_args(libname, "push", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  list_push(args[0], args[1]);

  return ast_noop();
}

sealobj* _pop(sealobj** args, size_t arg_size)
{
  seal_type expected_types[] = { SEAL_LIST };
  seal_check_args(libname, "pop", expected_types, sizeof(expected_types) / sizeof(expected_types[0]), args, arg_size);

  return list_pop(args[0]);
}
