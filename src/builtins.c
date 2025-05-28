#include "builtins.h"

#define BUILTIN_ERROR(...) do { \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  exit(1); \
} while (0)

#define BUILTIN_FUNC_ARG_ERR(name, arg_i, given, expected) do { \
  BUILTIN_ERROR("in function \'%s\': expected arg %d to be \'%s\', got \'%s\'", \
                name, arg_i, seal_type_name(given), seal_type_name(expected)); \
} while (0)

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
      printf("function: %p ", s.as.func.type == FUNC_BUILTIN ? (void*)&s.as.func.as.builtin.cfunc : (void*)&s.as.func.as.userdef.bytecode);
      break;
    default:
      printf("UNRECOGNIZED DATA TYPE TO PRINT\n");
    }
  }
  printf("\n");

  return SEAL_VALUE_NULL;
}
svalue_t __seal_scan(seal_byte argc, svalue_t* argv)
{
  static char buf[BUFSIZ];
  char *c = buf;
  while ((*c = getchar()) != EOF && *c != '\n')
    c++;

  *c = '\0';
  return SEAL_VALUE_STRING(buf);
}
svalue_t __seal_exit(seal_byte argc, svalue_t* argv)
{
   if (!IS_INT(argv[0]))
    BUILTIN_FUNC_ARG_ERR("exit", 0, VAL_TYPE(argv[0]), SEAL_INT); 

   exit(AS_INT(argv[0]));
   return SEAL_VALUE_NULL;
}
svalue_t __seal_len(seal_byte argc, svalue_t* argv)
{
  if (!IS_STRING(argv[0]))
    BUILTIN_FUNC_ARG_ERR("len", 0, VAL_TYPE(argv[0]), SEAL_STRING);

  return SEAL_VALUE_INT(strlen(AS_STRING(argv[0])));
}
svalue_t __seal_int(seal_byte argc, svalue_t* argv)
{
  if (!IS_STRING(argv[0]))
    BUILTIN_FUNC_ARG_ERR("int", 0, VAL_TYPE(argv[0]), SEAL_STRING);

  return SEAL_VALUE_INT(atoi(AS_STRING(argv[0])));
}
