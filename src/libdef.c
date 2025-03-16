#include "libdef.h"

static inline const char*
seal_type_name(seal_type type)
{
  switch (type) {
    case SEAL_NULL:     return "null";
    case SEAL_INT:      return "integer";
    case SEAL_FLOAT:    return "float";
    case SEAL_STRING:   return "string";
    case SEAL_BOOL:     return "bool";
    case SEAL_LIST:     return "list";
    case SEAL_OBJECT:   return "object";
    case SEAL_NUMBER:   return "number";
    case SEAL_ITERABLE: return "iterable";
    default:            return "SEAL TYPE NOT RECOGNIZED";
  }
}

void seal_func_err(const char* libname, const char* func_name, const char* err)
{
  fprintf(stderr,
          "seal: library \'%s\' error in \'%s\' function:\n%s\n",
          libname,
          func_name,
          err);
  exit(1);
}

void seal_check_args(const char* libname,
                     const char* func_name,
                     seal_type expected_types[],
                     size_t type_size,
                     sealobj* args[],
                     size_t arg_size)
{
  if (arg_size != type_size) {
    char err[ERR_LEN];
    sprintf(err, "expected %lu arg%s, got %lu", type_size, type_size > 1 ? "s" : "\0", arg_size);
    seal_func_err(libname, func_name, err);
  }
  for (int i = 0; i < type_size; i++) {
    sealobj* arg = args[i];
    switch (expected_types[i]) {
      case SEAL_NULL:
        if (IS_SEAL_NULL(arg)) continue;
        break;
      case SEAL_INT:
        if (IS_SEAL_INT(arg)) continue;
        break;
      case SEAL_FLOAT:
        if (IS_SEAL_FLOAT(arg)) continue;
        break;
      case SEAL_STRING:
        if (IS_SEAL_STRING(arg)) continue;
        break;
      case SEAL_BOOL:
        if (IS_SEAL_BOOL(arg)) continue;
        break;
      case SEAL_LIST:
        if (IS_SEAL_LIST(arg)) continue;
        break;
      case SEAL_OBJECT:
        if (IS_SEAL_OBJECT(arg)) continue;
        break;
      case SEAL_NUMBER:
        if (IS_SEAL_NUMBER(arg)) continue;
        break;
      case SEAL_ITERABLE:
        if (IS_SEAL_ITERABLE(arg)) continue;
        break;
    }

    char err[ERR_LEN];
    sprintf(err,
            "expected arg %d to be %s, got %s",
            i + 1,
            seal_type_name(expected_types[i]),
            seal_type_name(arg->type));
    seal_func_err(libname, func_name, err);
  }
}


sealobj* seal_get_obj_mem(sealobj* obj,
                          const char* mem_name,
                          seal_type type,
                          const char* libname,
                          const char* func_name)
{
  if (!IS_SEAL_OBJECT(obj)) {
    char err[ERR_LEN];
    sprintf(err, "required object, not \'%s\', by member \'%s\'\n", seal_type_name(obj->type), mem_name);
    seal_func_err(libname, func_name, err);
  } 
  for (int i = 0; i < obj->object.field_size; i++) {
    if (strcmp(mem_name, obj->object.field_names[i]) == 0) {
      sealobj* field = obj->object.field_vals[i];
      switch (type) {
        case SEAL_NULL:
          if (IS_SEAL_NULL(field)) break;
          goto error;
        case SEAL_INT:
          if (IS_SEAL_INT(field)) break;
          goto error;
        case SEAL_FLOAT:
          if (IS_SEAL_FLOAT(field)) break;
          goto error;
        case SEAL_STRING:
          if (IS_SEAL_STRING(field)) break;
          goto error;
        case SEAL_BOOL:
          if (IS_SEAL_BOOL(field)) break;
          goto error;
        case SEAL_LIST:
          if (IS_SEAL_LIST(field)) break;
          goto error;
        case SEAL_OBJECT:
          if (IS_SEAL_OBJECT(field)) break;
          goto error;
        case SEAL_NUMBER:
          if (IS_SEAL_NUMBER(field)) break;
          goto error;
        case SEAL_ITERABLE:
          if (IS_SEAL_ITERABLE(field)) break;
          goto error;
        error: {
          char err[ERR_LEN];
          sprintf(err,
                  "required member to be %s, got %s",
                  seal_type_name(type),
                  seal_type_name(field->type));
          seal_func_err(libname, func_name, err);
        }
      }
      return field;
    }
  }
  char err[ERR_LEN];
  sprintf(err,
          "object has no field named \'%s\'",
          mem_name);
  seal_func_err(libname, func_name, err);
  return seal_null();
}

sealobj* seal_create_object(const char* field_names[], sealobj* field_vals[], size_t field_size)
{
  sealobj* obj = create_ast(AST_OBJECT);
  obj->object.field_size = field_size;
  obj->object.field_vals = SEAL_CALLOC(field_size, sizeof(sealobj*));
  for (int i = 0; i < field_size; i++) {
    obj->object.field_vals[i] = field_vals[i];
  }
  obj->object.field_names = field_names;
  return obj;
}
