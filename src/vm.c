#include "vm.h"
#include "builtins.h"
#include "gc.h"

#define FETCH(lf) (*lf->ip++)
#define PUSH(vm, val) do { \
  if (vm->sp - vm->stack == STACK_SIZE) \
    ERROR("stack overflow"); \
  svalue_t loc_val = val; \
  *vm->sp++ = loc_val; \
  gc_incref(loc_val); \
} while (0)
#define DUP(vm) do { \
  svalue_t top = *(vm->sp - 1); \
  PUSH(vm, top); \
} while (0)
#define POP(vm) (*(--(vm->sp)))
#define JUMP(vm, lf, addr) (lf->ip = &lf->bytecodes[vm->label_ptr[addr]])
#define GET_CONST(vm, i) (vm->const_pool_ptr[i])
#define ERROR(...) (fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"), exit(EXIT_FAILURE))
#define ERROR_UNRY_OP(op, val) ERROR("\'%s\' unary operator is not supported for \'%s\'", #op, seal_type_name(val.type))
#define ERROR_BIN_OP(op, left, right) ERROR("\'%s\' operator is not supported for \'%s\' and \'%s\'", #op, seal_type_name(left.type), seal_type_name(right.type))

#define PUSH_NULL(vm)        PUSH(vm, (svalue_t) { .type = SEAL_NULL })
#define PUSH_INT(vm, val)    PUSH(vm, sval(SEAL_INT, _int, val))
#define PUSH_FLOAT(vm, val)  PUSH(vm, sval(SEAL_FLOAT, _float, val))
#define PUSH_STRING(vm, val) PUSH(vm, sval(SEAL_STRING, string, val))
#define PUSH_BOOL(vm, val)   PUSH(vm, sval(SEAL_BOOL, _bool, val))

#define TO_INT(val)
#define TO_FLOAT(val)
#define TO_STRING(val)
#define TO_BOOL(val) ( \
  IS_NULL(val) ? false : \
  IS_INT(val) ? AS_INT(val) != 0 : \
  IS_FLOAT(val) ? AS_FLOAT(val) != 0.0 : \
  IS_STRING(val) ? true : \
  IS_BOOL(val) ? AS_BOOL(val) : \
  IS_LIST(val) ? AS_LIST(val)->size > 0 : \
  (ERROR("cannot convert to bool type"), -1))

/* macros for seal functions */
#define IS_FUNC_VARARG(val)    (AS_FUNC(val).is_vararg)
#define IS_BUILTIN_FUNC(val)   (AS_FUNC(val).type == FUNC_BUILTIN)
#define IS_USERDEF_FUNC(val)   (AS_FUNC(val).type == FUNC_USERDEF)
#define AS_BUILTIN_FUNC(val)   (AS_FUNC(val).as.builtin)
#define AS_USERDEF_FUNC(val)   (AS_FUNC(val).as.userdef)
#define FUNC_ARGC(val)         (IS_BUILTIN_FUNC(val) ? AS_BUILTIN_FUNC(val).argc : AS_USERDEF_FUNC(val).argc)
#define FUNC_NAME(val)         (AS_FUNC(val).name)
#define CALL_BUILTIN_FUNC(val) (AS_BUILTIN_FUNC(val).cfunc)

/* string manipulation */
struct seal_string* str_concat(const char* l, const char* r)
{
  int len = strlen(l) + strlen(r) + 1;
  char* res = SEAL_CALLOC(len, sizeof(char));
  strcpy(res, l);
  strcat(res, r);
  res[len - 1] = '\0';
  struct seal_string *str = SEAL_CALLOC(1, sizeof(struct seal_string));
  str->size = len - 1;
  str->is_static = false;
  str->ref_count = 0;
  str->val = res;
  return str;
}

/* arithmetic */
#define BIN_OP_INT(vm, left, right, op)   PUSH_INT(vm, AS_INT(left) op AS_INT(right))
#define BIN_OP_FLOAT(vm, left, right, op) PUSH_FLOAT(vm, AS_FLOAT(left) op AS_FLOAT(right))
#define BIN_OP_INT_AND_FLOAT(vm, left, right, op) \
  PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) op (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)))

#define BIN_OP(vm, left, right, op) do { \
  if (IS_INT(left) && IS_INT(right)) \
    BIN_OP_INT(vm, left, right, op); \
  else if (IS_FLOAT(left) && IS_FLOAT(right)) \
    BIN_OP_FLOAT(vm, left, right, op); \
  else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right)) \
    BIN_OP_INT_AND_FLOAT(vm, left, right, op); \
  else if (IS_STRING(left) && IS_STRING(right)) \
    PUSH_STRING(vm, str_concat(AS_STRING(left), AS_STRING(right))); \
  else \
    ERROR_BIN_OP(op, left, right); \
} while (0)

#define MOD_OP(vm, left, right) do { \
  if (IS_INT(left) && IS_INT(right)) \
    PUSH_INT(vm, AS_INT(left) % AS_INT(right)); \
  else \
    ERROR_BIN_OP(%, left, right); \
} while (0)

/* bitwise binary */
#define BITWISE_OP(vm, left, right, op) do { \
  if (IS_INT(left) && IS_INT(right)) \
    PUSH_INT(vm, AS_INT(left) op AS_INT(right)); \
  else \
    ERROR_BIN_OP(op, left, right); \
} while (0)

/* equality */
#define EQUAL_OP_STRING(vm, left, right, op) PUSH_BOOL(vm, strcmp(AS_STRING(left), AS_STRING(right)) op 0)
#define EQUAL_OP_BOOL(vm, left, right, op)   PUSH_BOOL(vm, AS_BOOL(left) op AS_BOOL(right))
#define EQUAL_OP_INT(vm, left, right, op)    PUSH_BOOL(vm, AS_INT(left) op AS_INT(right))
#define EQUAL_OP_FLOAT(vm, left, right, op)  PUSH_BOOL(vm, AS_FLOAT(left) op AS_FLOAT(right))
#define EQUAL_OP_INT_AND_FLOAT(vm, left, right, op) \
  PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) op (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)))

#define EQUAL_OP(vm, left, right, op) do { \
  if (IS_INT(left) && IS_INT(right)) \
    EQUAL_OP_INT(vm, left, right, op); \
  else if (IS_FLOAT(left) && IS_FLOAT(right)) \
    EQUAL_OP_FLOAT(vm, left, right, op); \
  else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right)) \
    EQUAL_OP_INT_AND_FLOAT(vm, left, right, op); \
  else if (IS_STRING(left) && IS_STRING(right)) \
    EQUAL_OP_STRING(vm, left, right, op); \
  else if (IS_BOOL(left) && IS_BOOL(right)) \
    EQUAL_OP_BOOL(vm, left, right, op); \
  else \
    ERROR_BIN_OP(op, left, right); \
} while (0)

/* comparison */
#define CMP_OP_INT(vm, left, right, op)    PUSH_BOOL(vm, AS_INT(left) op AS_INT(right))
#define CMP_OP_FLOAT(vm, left, right, op)  PUSH_BOOL(vm, AS_FLOAT(left) op AS_FLOAT(right))
#define CMP_OP_INT_AND_FLOAT(vm, left, right, op) \
  PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) op (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)))

#define CMP_OP(vm, left, right, op) do { \
  if (IS_INT(left) && IS_INT(right)) \
    CMP_OP_INT(vm, left, right, op); \
  else if (IS_FLOAT(left) && IS_FLOAT(right)) \
    CMP_OP_FLOAT(vm, left, right, op); \
  else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right)) \
    CMP_OP_INT_AND_FLOAT(vm, left, right, op); \
  else \
    ERROR_BIN_OP(op, left, right); \
} while (0)

/* unary */
#define UNRY_OP(vm, val, op) do { \
  switch (op) { \
  case OP_NEG: \
    if (IS_INT(val)) \
      PUSH_INT(vm, -(AS_INT(val))); \
    else \
      ERROR_UNRY_OP(-, val); \
    break; \
  case OP_BNOT: \
    if (IS_INT(val)) \
      PUSH_INT(vm, ~(AS_INT(val))); \
    else \
      ERROR_UNRY_OP(~, val); \
    break; \
  case OP_TYPOF: \
    PUSH(vm, SEAL_VALUE_STRING(seal_type_name((val).type))); \
    break; \
  case OP_NOT: \
    if (IS_BOOL(val)) \
      PUSH_BOOL(vm, !(AS_BOOL(val))); \
    else \
      ERROR_UNRY_OP(not, val); \
    break; \
  } \
} while (0)

/* local variables */
#define GET_LOCAL(lf, slot) (lf->locals[slot])
#define SET_LOCAL(lf, slot, val) (lf->locals[slot] = val)

/* initialization */
#define REGISTER_BUILTIN_FUNC(map, _name, str, _argc, _is_vararg) do { \
  svalue_t func = { \
    .type = SEAL_FUNC, \
    .as.func = { \
      .type = FUNC_BUILTIN, \
      .name = str, \
      .is_vararg = _is_vararg, \
      .as.builtin = { \
        .cfunc = _name, \
        .argc = _argc \
      } \
    } \
  }; \
  hashmap_insert(map, str, func); \
} while (0)

void init_vm(vm_t* vm, cout_t* cout)
{
  vm->const_pool_ptr = cout->const_pool;
  vm->label_ptr = cout->labels;

  svalue_t *stack = SEAL_CALLOC(STACK_SIZE, sizeof(svalue_t));
  vm->stack = stack;

  vm->sp = vm->stack;
  vm->bytecodes = cout->bc.bytecodes;
  // vm->lf = SEAL_CALLOC(FRAME_MAX, sizeof(struct local_frame));
  // vm->lf[0] = (struct local_frame) { .ip = vm->bytecodes, .caller = NULL };
  hashmap_init(&vm->globals, 256);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_print, "print", 0, true);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_scan, "scan", 0, false);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_exit, "exit", 1, false);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_len, "len", 1, false);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_int, "int", 1, false);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_push, "push", 2, false);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_pop, "pop", 1, false);
}

void eval_vm(vm_t* vm, struct local_frame* lf)
{
  while (true) {
    seal_byte op = FETCH(lf);

    seal_word idx, addr;
    svalue_t  left, right;
    struct h_entry* entry;
    const char* sym;

    switch (op) {
    case OP_HALT:
      return;
    case OP_PUSH_CONST:
      idx = FETCH(lf) << 8;
      idx |= FETCH(lf);
      PUSH(vm, GET_CONST(vm, idx));
      left = GET_CONST(vm, idx);
      break;
    case OP_PUSH_INT:
      idx = FETCH(lf) << 8;
      idx |= FETCH(lf);
      PUSH_INT(vm, idx);
      break;
    case OP_PUSH_NULL:
      PUSH_NULL(vm);
      break;
    case OP_PUSH_TRUE:
      PUSH_BOOL(vm, true);
      break;
    case OP_PUSH_FALSE:
      PUSH_BOOL(vm, false);
      break;
    case OP_POP:
      left = POP(vm);
      gc_decref(left);
      break;
    case OP_DUP:
      DUP(vm);
      break;
    case OP_ADD:
      right = POP(vm);
      left  = POP(vm);
      BIN_OP(vm, left, right, +);
      gc_decref(left);
      gc_decref(right);
      break;
    case OP_SUB:
      right = POP(vm);
      left  = POP(vm);
      BIN_OP(vm, left, right, -);
      break;
    case OP_MUL:
      right = POP(vm);
      left  = POP(vm);
      BIN_OP(vm, left, right, *);
      break;
    case OP_DIV:
      right = POP(vm);
      left  = POP(vm);
      BIN_OP(vm, left, right, /);
      break;
    case OP_MOD:
      right = POP(vm);
      left  = POP(vm);
      MOD_OP(vm, left, right);
      break;
    case OP_AND:
      right = POP(vm);
      left  = POP(vm);
      BITWISE_OP(vm, left, right, &);
      break;
    case OP_OR:
      right = POP(vm);
      left  = POP(vm);
      BITWISE_OP(vm, left, right, |);
      break;
    case OP_XOR:
      right = POP(vm);
      left  = POP(vm);
      BITWISE_OP(vm, left, right, ^);
      break;
    case OP_SHL:
      right = POP(vm);
      left  = POP(vm);
      BITWISE_OP(vm, left, right, <<);
      break;
    case OP_SHR:
      right = POP(vm);
      left  = POP(vm);
      BITWISE_OP(vm, left, right, >>);
      break;
    case OP_EQ:
      right = POP(vm);
      left  = POP(vm);
      EQUAL_OP(vm, left, right, ==);
      break;
    case OP_NE:
      right = POP(vm);
      left  = POP(vm);
      EQUAL_OP(vm, left, right, !=);
      break;
    case OP_GT:
      right = POP(vm);
      left  = POP(vm);
      CMP_OP(vm, left, right, >);
      break;
    case OP_GE:
      right = POP(vm);
      left  = POP(vm);
      CMP_OP(vm, left, right, >=);
      break;
    case OP_LT:
      right = POP(vm);
      left  = POP(vm);
      CMP_OP(vm, left, right, <);
      break;
    case OP_LE:
      right = POP(vm);
      left  = POP(vm);
      CMP_OP(vm, left, right, <=);
      break;
    case OP_NOT:
    case OP_NEG:
    case OP_TYPOF:
    case OP_BNOT:
      left = POP(vm);
      UNRY_OP(vm, left, op);
      break;
    case OP_JUMP:
      addr = FETCH(lf) << 8;
      addr |= FETCH(lf);
      JUMP(vm, lf, addr);
      break;
    case OP_JFALSE:
      addr = FETCH(lf) << 8;
      addr |= FETCH(lf);
      left = POP(vm);
      if (!TO_BOOL(left))
        JUMP(vm, lf, addr);
      break;
    case OP_JTRUE:
      addr = FETCH(lf) << 8;
      addr |= FETCH(lf);
      left = POP(vm);
      if (TO_BOOL(left))
        JUMP(vm, lf, addr);
      break;
    case OP_GET_GLOBAL:
      addr = FETCH(lf) << 8;
      addr |= FETCH(lf);
      sym = AS_STRING(GET_CONST(vm, addr));
      entry = hashmap_search(&vm->globals, sym);
      if (entry && entry->key) {
        PUSH(vm, entry->val);
      } else {
        ERROR("vm: %s is not defined", sym);
      }
      break;
    case OP_SET_GLOBAL:
      DUP(vm);
      addr = FETCH(lf) << 8;
      addr |= FETCH(lf);
      hashmap_insert(&vm->globals, AS_STRING(GET_CONST(vm, addr)), POP(vm));
      break;
    case OP_GET_LOCAL:
      addr = FETCH(lf);
      PUSH(vm, GET_LOCAL(lf, addr));
      break;
    case OP_SET_LOCAL:
      DUP(vm);
      addr = FETCH(lf);
      left = POP(vm);
      gc_decref(GET_LOCAL(lf, addr));
      SET_LOCAL(lf, addr, left);
      break;
    case OP_CALL: {
      seal_byte argc = FETCH(lf);
      svalue_t *argv = vm->sp - argc;
      vm->sp -= argc;

      svalue_t func = POP(vm);
      if (!IS_FUNC(func))
        ERROR("calling non-function: \'%s\'", seal_type_name(func.type));

      if (!IS_FUNC_VARARG(func) && argc != FUNC_ARGC(func) || IS_FUNC_VARARG(func) && argc < FUNC_ARGC(func))
        ERROR("\'%s\' function expected%s %d argument%s, got %d",
              FUNC_NAME(func),
              IS_FUNC_VARARG(func) ? " at least" : "",
              FUNC_ARGC(func),
              FUNC_ARGC(func) != 1 ? "s" : "",
              argc);

      if (IS_BUILTIN_FUNC(func)) {
        PUSH(vm, CALL_BUILTIN_FUNC(func)(argc, argv)); /* push function result to stack */
        for (int i = 0; i < argc; i++) {
          gc_decref(argv[i]);
        }
      } else {
        svalue_t locals[func.as.func.as.userdef.local_size];
        memset(locals, 0, sizeof(locals));
        struct local_frame func_lf = { .locals = locals, .ip = AS_USERDEF_FUNC(func).bytecode, .bytecodes = AS_USERDEF_FUNC(func).bytecode };
        for (int i = 0; i < argc; i++) {
          SET_LOCAL((&func_lf), i, argv[i]);
        }
        eval_vm(vm, &func_lf);
        for (int i = 0; i < func.as.func.as.userdef.local_size; i++) {
          gc_decref(locals[i]);
        }
      }
      break;
    }
    case OP_GEN_LIST: {
      seal_byte size = FETCH(lf);
      left = SEAL_VALUE_LIST();
      svalue_t sorted[size];
      for (int i = 0; i < size; i++) {
        sorted[i] = POP(vm);
      }
      for (int i = size - 1; i >= 0; i--) {
        LIST_PUSH(left, sorted[i]);
      }
      PUSH(vm, left);
      break;
    }
    case OP_GET_FIELD:
      right = POP(vm);
      left  = POP(vm);
      if (IS_MAP(left)) {
        if (IS_STRING(right)) {
          struct sh_entry *e = shashmap_search(AS_MAP(left)->map, AS_STRING(right));
          if (e == NULL || e->key == NULL)
            ERROR("\'%s\' key is not found", AS_STRING(right));

          PUSH(vm, e->val);
          gc_decref(left);
          gc_decref(right);
          break;
        }
      }

      if (!IS_LIST(left) && !IS_STRING(left))
        ERROR("subscript requires list or string as base");
      if (!IS_INT(right))
        ERROR("subscript requires int as field");
      if (AS_INT(right) < 0)
        ERROR("cannot index negative");
      if (IS_STRING(left)) {
        if (AS_INT(right) >= left.as.string->size)
          ERROR("index exceed size");
        char *c = SEAL_CALLOC(2, sizeof(char));
        c[0] = AS_STRING(left)[AS_INT(right)];
        c[1] = '\0';
        PUSH(vm, SEAL_VALUE_STRING(c));
      } else {
        if (AS_INT(right) >= AS_LIST(left)->size)
          ERROR("index exceed size");
        PUSH(vm, AS_LIST(left)->mems[AS_INT(right)]);
      }
      gc_decref(left);
      gc_decref(right);
      break;
    case OP_SET_FIELD:
      right = POP(vm);
      left  = POP(vm);
      if (!IS_LIST(left))
        ERROR("subscript assign requires only list as base");
      if (!IS_INT(right))
        ERROR("subscript requires int as field");
      if (AS_INT(right) < 0)
        ERROR("cannot index negative");
      if (AS_INT(right) >= AS_LIST(left)->size)
        ERROR("index exceed size");
      gc_decref(AS_LIST(left)->mems[AS_INT(right)]);
      PUSH(vm, AS_LIST(left)->mems[AS_INT(right)] = POP(vm));
      gc_decref(left);
      gc_decref(right);
      break;
    case OP_IN:
      right = POP(vm);
      left  = POP(vm);
      if (!IS_STRING(right))
        ERROR("in operator requires string");
      if (!IS_STRING(left))
        ERROR("leftside must be string when rightside is string");

      PUSH_BOOL(vm, strstr(AS_STRING(right), AS_STRING(left)) != NULL);
      gc_decref(left);
      gc_decref(right);
      break;
    case OP_GEN_MAP: {
      seal_byte size = FETCH(lf);
      left = SEAL_VALUE_MAP();
      for (int i = 0; i < size; i++) {
        const char *key = AS_STRING(POP(vm));
        MAP_INSERT(left, key, POP(vm)); 
      }
      PUSH(vm, left);
      break;
    }
    default:
      fprintf(stderr, "unrecognized op type: %d\n", op);
      return;
    }
  }
}
