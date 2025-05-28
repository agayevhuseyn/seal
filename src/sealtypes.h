#ifndef SEAL_TYPES_H
#define SEAL_TYPES_H

#include "seal.h"

#define SEAL_NULL        (1 << 0)    /* 00000001 */
#define SEAL_INT         (1 << 1)    /* 00000010 */
#define SEAL_FLOAT       (1 << 2)    /* 00000100 */
#define SEAL_STRING      (1 << 3)    /* 00001000 */
#define SEAL_BOOL        (1 << 4)    /* 00010000 */
#define SEAL_LIST        (1 << 5)    /* 00100000 */
#define SEAL_MAP         (1 << 6)    /* 01000000 */
#define SEAL_FUNC        (1 << 7)    /* 10000000 */
#define SEAL_NUMBER      (SEAL_INT | SEAL_FLOAT)    /* 00000110 */
#define SEAL_ITERABLE    (SEAL_STRING | SEAL_LIST)  /* 00000110 */
#define SEAL_ANY         (SEAL_NULL | SEAL_INT | SEAL_FLOAT | SEAL_STRING | \
                          SEAL_BOOL | SEAL_LIST | SEAL_MAP)  /* 11111111 */

typedef int seal_type;
typedef struct svalue svalue_t;

struct seal_func {
  enum {
    FUNC_BUILTIN,
    FUNC_USERDEF
  } type;
  union {
    struct {
      seal_byte* bytecode;
      seal_byte  argc;
      seal_byte  local_size;
    } userdef;
    struct {
      svalue_t (*cfunc)(seal_byte argc, svalue_t* argv);
      seal_byte argc;
    } builtin;
  } as;
  const char* name;
  bool is_vararg;
};

struct svalue {
  seal_type type;
  union {
    seal_int    _int;
    seal_float  _float;
    const char* string;
    bool        _bool;
    struct seal_func func;
    /*  TODO
    as_list;
    as_map;
    */
  } as;
};

#define sval(t, mem, val) (svalue_t) { .type = t, .as.mem = val}
#define SEAL_VALUE_NULL   (svalue_t) { .type = SEAL_NULL }
#define SEAL_VALUE_TRUE   sval(SEAL_BOOL, _bool, true)
#define SEAL_VALUE_FALSE  sval(SEAL_BOOL, _bool, false)
#define SEAL_VALUE_INT(val)    sval(SEAL_INT, _int, val)
#define SEAL_VALUE_STRING(val) sval(SEAL_STRING, string, val)

static inline const char*
seal_type_name(int type)
{
  switch (type) {
    case SEAL_NULL    : return "null";
    case SEAL_INT     : return "int";
    case SEAL_FLOAT   : return "float";
    case SEAL_STRING  : return "string";
    case SEAL_BOOL    : return "bool";
    case SEAL_LIST    : return "list";
    case SEAL_MAP     : return "map";
    case SEAL_FUNC    : return "function";
    case SEAL_NUMBER  : return "number";
    case SEAL_ITERABLE: return "iterable";
    case SEAL_ANY     : return "any";
    default           : return "SEAL TYPE NOT RECOGNIZED";
  }
}

#endif /* SEAL_TYPES_H */
