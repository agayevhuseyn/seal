#include "libdef.h"
#include "ast.h"
#include <stdio.h>
#include <string.h>

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
    case SEAL_DATA: return "SEAL_DATA";
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
    (expected_types[i] != SEAL_NUMBER &&
      expected_types[i] != SEAL_ITERABLE &&
      expected_types[i] != SEAL_DATA &&
      (seal_type)args[i]->type != expected_types[i]) ||
    (expected_types[i] == SEAL_DATA && (seal_type)args[i]->type == SEAL_NOOP)) {
      char msg[256];
      sprintf(msg, "\'%s\' function expected arg %i to be %s, got %s",
             func_name, i + 1, seal_type_name(expected_types[i]), seal_type_name((seal_type)args[i]->type));
      liberror(libname, msg);
    }
  }
}

sealobj* get_obj_mem(ast_t* obj, const char* mem_name, seal_type type)
{
  if (obj->type != (AST_Type)SEAL_OBJECT) {
    printf("Required object, not \"%s\", by %s\n", seal_type_name((seal_type)obj->type), mem_name);
    exit(1);
  }
  for (int i = 0; i < obj->object.field_size; i++) {
    if (strcmp(mem_name, obj->object.def->obj_def.fields[i]->name) == 0) {
      sealobj* mem = obj->object.field_vars[i];
      
      if ((type == SEAL_NUMBER && ((seal_type)mem->type != SEAL_INT && (seal_type)mem->type != SEAL_FLOAT)) ||
      (type == SEAL_ITERABLE && ((seal_type)mem->type != SEAL_LIST && (seal_type)mem->type != SEAL_STRING)) ||
      (type != SEAL_NUMBER &&
        type != SEAL_ITERABLE &&
        type != SEAL_DATA &&
        (seal_type)mem->type != type) ||
      (type == SEAL_DATA && (seal_type)mem->type == SEAL_NOOP)) {
        printf("Object \'%s\' member \'%s\' is not type \'%s\', but \'%s\'\n",
               obj->object.def->obj_def.oname, mem_name, seal_type_name(type), seal_type_name((seal_type)mem->type));
        exit(1);
      }

      return mem; // successful return
    }
  }
 
  printf("no member named %s in %s\n", mem_name, obj->object.def->obj_def.oname);
  exit(1);
  return (void*)0;
}

void seal_func_err(const char* libname, const char* func_name, const char* msg)
{
  // no need newline
  char fmsg[512];
  sprintf(fmsg, "function %s: %s", func_name, msg);
  liberror(libname, fmsg);
}
