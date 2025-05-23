#include "builtins.h"

svalue_t __seal_print(seal_byte argc, svalue_t* argv)
{
  for (int i = 0; i < argc; i++) {
    svalue_t s = argv[i];

    switch (s.type) {
    case SEAL_INT:
      printf("%lld ", s.as._int);
      break;
    case SEAL_FLOAT:
      printf("%f ", s.as._float);
      break;
    case SEAL_STRING:
      printf("%s ", s.as.string);
      break;
    case SEAL_BOOL:
      printf("%s ", s.as._bool ? "true" : "false");
      break;
    case SEAL_NULL:
      printf("null ");
      break;
    case SEAL_FUNC:
      printf("function: %llx ", s.as.func.type == FUNC_BUILTIN ? (seal_int)&s.as.func.as.builtin.cfunc : (seal_int)&s.as.func.as.userdef.bytecode);
      break;
    default:
      printf("UNRECOGNIZED DATA TYPE TO PRINT\n");
    }
  }
  printf("\n");

  return SEAL_NULL_VALUE;
}
