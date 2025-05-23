#include "vm.h"
#include "builtins.h"

#define FETCH(vm) (*vm->ip++)
#define PUSH(vm, val) do { \
  if (vm->sp - vm->stack == STACK_SIZE) \
    ERROR("stack overflow"); \
  (*vm->sp++ = val); \
} while (0)
#define DUP(vm) do { \
  svalue_t top = *(vm->sp - 1); \
  PUSH(vm, top); \
} while (0)
#define POP(vm) (*(--(vm->sp)))
#define JUMP(vm, addr) (vm->ip = &vm->bytecodes[vm->label_ptr[addr]])
#define GET_CONST(vm, i) (vm->const_pool_ptr[i])
#define ERROR(...) (fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"), exit(EXIT_FAILURE))
#define ERROR_UNRY_OP(op, val) ERROR("\'%s\' unary operator is not supported for \'%s\'", #op, seal_type_name(val.type))
#define ERROR_BIN_OP(op, left, right) ERROR("\'%s\' operator is not supported for \'%s\' and \'%s\'", #op, seal_type_name(left.type), seal_type_name(right.type))

#define PUSH_NULL(vm)        PUSH(vm, (svalue_t) { .type = SEAL_NULL })
#define PUSH_INT(vm, val)    PUSH(vm, sval(SEAL_INT, _int, val))
#define PUSH_FLOAT(vm, val)  PUSH(vm, sval(SEAL_FLOAT, _float, val))
#define PUSH_STRING(vm, val) PUSH(vm, sval(SEAL_STRING, string, val))
#define PUSH_BOOL(vm, val)   PUSH(vm, sval(SEAL_BOOL, _bool, val))

#define AS_INT(val)    (val.as._int)
#define AS_FLOAT(val)  (val.as._float)
#define AS_STRING(val) (val.as.string)
#define AS_BOOL(val)   (val.as._bool)
#define AS_FUNC(val)   (val.as.func)

#define IS_NULL(val)   (val.type == SEAL_NULL)
#define IS_INT(val)    (val.type == SEAL_INT)
#define IS_FLOAT(val)  (val.type == SEAL_FLOAT)
#define IS_STRING(val) (val.type == SEAL_STRING)
#define IS_BOOL(val)   (val.type == SEAL_BOOL)
#define IS_FUNC(val)   (val.type == SEAL_FUNC)

#define TO_INT(val)
#define TO_FLOAT(val)
#define TO_STRING(val)
#define TO_BOOL(val) ( \
  IS_NULL(val) ? false : \
  IS_INT(val) ? AS_INT(val) != 0 : \
  IS_FLOAT(val) ? AS_FLOAT(val) != 0.0 : \
  IS_STRING(val) ? true : \
  IS_BOOL(val) ? AS_BOOL(val) : \
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
    PUSH_STRING(vm, seal_type_name((val).type)); \
    break; \
  case OP_NOT: \
    if (IS_BOOL(val)) \
      PUSH_BOOL(vm, !(AS_BOOL(val))); \
    else \
      ERROR_UNRY_OP(not, val); \
    break; \
  } \
} while (0)

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
  vm->sp = vm->stack;
  vm->ip = vm->bytecodes = cout->bytecodes;
  hashmap_init(&vm->globals, 255);
  REGISTER_BUILTIN_FUNC(&vm->globals, __seal_print, "print", 0, true);
}

void eval_vm(vm_t* vm)
{
  while (true) {
    seal_byte op = FETCH(vm);

    seal_word idx, addr;
    svalue_t  left, right;
    struct h_entry* entry;
    const char* sym;

    switch (op) {
      case OP_HALT:
        printf("Finish\n");
        return;
      case OP_PUSH_CONST:
        idx = FETCH(vm) << 8;
        idx |= FETCH(vm);
        PUSH(vm, GET_CONST(vm, idx));
        break;
      case OP_PUSH_INT:
        idx = FETCH(vm) << 8;
        idx |= FETCH(vm);
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
        POP(vm);
        break;
      case OP_DUP:
        DUP(vm);
        break;
      case OP_ADD:
        right = POP(vm);
        left  = POP(vm);
        BIN_OP(vm, left, right, +);
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
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        JUMP(vm, addr);
        break;
      case OP_JFALSE:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        left = POP(vm);
        if (!TO_BOOL(left))
          JUMP(vm, addr);
        break;
      case OP_JTRUE:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        left = POP(vm);
        if (TO_BOOL(left))
          JUMP(vm, addr);
        break;
      case OP_GET_GLOBAL:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        sym = AS_STRING(GET_CONST(vm, addr));
        entry = hashmap_search(&vm->globals, sym);
        if (entry->key) {
          PUSH(vm, entry->val);
        } else {
          ERROR("vm: %s is not defined", sym);
        }
        break;
      case OP_SET_GLOBAL:
        DUP(vm);
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        hashmap_insert(&vm->globals, AS_STRING(GET_CONST(vm, addr)), POP(vm));
        break;
      case OP_CALL: {
        seal_byte argc = FETCH(vm);
        svalue_t argv[argc];
        for (int i = argc - 1; i >= 0; i--)
          argv[i] = POP(vm);

        svalue_t callee = POP(vm);
        if (!IS_FUNC(callee))
          ERROR("calling non-function: \'%s\'", seal_type_name(callee.type));

        if (!IS_FUNC_VARARG(callee) && argc != FUNC_ARGC(callee) || IS_FUNC_VARARG(callee) && argc < FUNC_ARGC(callee))
          ERROR("\'%s\' function expected%s %d argument%s, got %d",
                FUNC_NAME(callee),
                IS_FUNC_VARARG(callee) ? " at least" : "",
                FUNC_ARGC(callee),
                FUNC_ARGC(callee) != 1 ? "s" : "",
                argc);

        svalue_t res;
        if (IS_BUILTIN_FUNC(callee)) {
          res = CALL_BUILTIN_FUNC(callee)(argc, argv);
        } else {
          ERROR("user defined functions not defined yet");
        }

        PUSH(vm, res);
      }
      break;
      default: fprintf(stderr, "unrecognized op type: %s\n", op_name(op)); return;
    }
  }
}
