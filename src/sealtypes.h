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
                          SEAL_BOOL | SEAL_LIST | SEAL_MAP | SEAL_FUNC)  /* 11111111 */


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
      //size_t bytecode_size;
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

struct seal_string {
  const char* val;
  int size;
  int ref_count;
  bool is_static;
};

struct seal_list {
  svalue_t *mems;
  size_t size;
  size_t cap;
  int ref_count;
};

#define LIST_PUSH(s, e) do { \
  struct seal_list *l = AS_LIST(s); \
  if (l->size >= l->cap) { \
    l->mems = SEAL_REALLOC(l->mems, sizeof(svalue_t) * (l->cap *= 2)); \
  } \
  l->mems[l->size++] = e; \
} while (0)


struct svalue {
  seal_type type;
  union {
    seal_int    _int;
    seal_float  _float;
    struct seal_string *string;
    bool        _bool;
    struct seal_func func;
    struct seal_list *list;
    /*
    as_map;
    */
  } as;
};


#define AS_INT(val)    ((val).as._int)
#define AS_FLOAT(val)  ((val).as._float)
#define AS_STRING(_val) ((_val).as.string->val)
#define AS_BOOL(val)   ((val).as._bool)
#define AS_FUNC(val)   ((val).as.func)
#define AS_LIST(val)   ((val).as.list)

#define VAL_TYPE(val)  ((val).type)
#define IS_NULL(val)   (VAL_TYPE(val) == SEAL_NULL)
#define IS_INT(val)    (VAL_TYPE(val) == SEAL_INT)
#define IS_FLOAT(val)  (VAL_TYPE(val) == SEAL_FLOAT)
#define IS_STRING(val) (VAL_TYPE(val) == SEAL_STRING)
#define IS_BOOL(val)   (VAL_TYPE(val) == SEAL_BOOL)
#define IS_FUNC(val)   (VAL_TYPE(val) == SEAL_FUNC)
#define IS_LIST(val)   (VAL_TYPE(val) == SEAL_LIST)


#define sval(t, mem, val) (svalue_t) { .type = t, .as.mem = val }
#define SEAL_VALUE_NULL   (svalue_t) { .type = SEAL_NULL }
#define SEAL_VALUE_TRUE   sval(SEAL_BOOL, _bool, true)
#define SEAL_VALUE_FALSE  sval(SEAL_BOOL, _bool, false)
#define SEAL_VALUE_INT(val)    sval(SEAL_INT, _int, val)
#define SEAL_VALUE_FLOAT(val)  sval(SEAL_FLOAT, _float, val)


static inline svalue_t SEAL_VALUE_STRING(const char* val)
{
  svalue_t res = {
    .type = SEAL_STRING,
    .as.string = SEAL_CALLOC(1, sizeof(struct seal_string))
  };
  res.as.string->val = val;
  res.as.string->size = strlen(val);
  res.as.string->is_static = false;
  res.as.string->ref_count = 0;
  return res;
}

static inline svalue_t SEAL_VALUE_STRING_STATIC(const char* val)
{
  svalue_t res = SEAL_VALUE_STRING(val);
  res.as.string->is_static = true;
  return res;
}

static inline svalue_t SEAL_VALUE_LIST()
{
  svalue_t res = {
    .type = SEAL_LIST,
    .as.list = SEAL_CALLOC(1, sizeof(struct seal_list))
  };
  AS_LIST(res)->ref_count = 0;
  AS_LIST(res)->cap = 2;
  AS_LIST(res)->size = 0;
  AS_LIST(res)->mems = SEAL_CALLOC(AS_LIST(res)->cap, sizeof(svalue_t));
  return res;
}

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
