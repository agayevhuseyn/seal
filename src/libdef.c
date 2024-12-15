#include "libdef.h"
#include <stdio.h>

const char* seal_type_name(seal_type type)
{
  switch (type) {
    case SEAL_NUMBER: return "SEAL_NUMBER";
    case SEAL_ITERABLE: return "SEAL_ITERABLE";
    case SEAL_INT: return "SEAL_INT";
    case SEAL_FLOAT: return "SEAL_FLOAT";
    case SEAL_STRING: return "SEAL_STRING";
    case SEAL_BOOL: return "SEAL_BOOL";
    case SEAL_LIST: return "SEAL_LIST";
    case SEAL_OBJECT: return "SEAL_OBJECT";
    case SEAL_NULL: return "SEAL_NULL";
    case SEAL_NOOP: return "SEAL_NOOP";
  }
}

static inline void liberror(const char* libname, const char* msg)
{
  printf("Library %s-> %s\n", libname, msg);
  exit(1);
}

void seal_check_args(const char* libname,
                     const char* func_name,
                     seal_type* expected_types,
                     size_t type_size,
                     sealobj** args,
                     size_t arg_size)
{
  if (arg_size != type_size) {
    char msg[256];
    sprintf(msg, "\'%s\' function expected %lu arg%s, got %lu", func_name, type_size, type_size > 1 ? "s" : "\0", arg_size);
    liberror(libname, msg);
  }
  for (int i = 0; i < type_size; i++) {
    if ((expected_types[i] == SEAL_NUMBER && ((seal_type)args[i]->type != SEAL_INT && (seal_type)args[i]->type != SEAL_FLOAT)) ||
    (expected_types[i] == SEAL_ITERABLE && ((seal_type)args[i]->type != SEAL_LIST && (seal_type)args[i]->type != SEAL_STRING)) ||
    (expected_types[i] != SEAL_NUMBER && expected_types[i] != SEAL_ITERABLE && (seal_type)args[i]->type != expected_types[i])) {
      char msg[256];
      sprintf(msg, "\'%s\' function expected arg %i to be %s, got %s",
             func_name, i, seal_type_name(expected_types[i]), seal_type_name((seal_type)args[i]->type));
      liberror(libname, msg);
    }
  }
}
