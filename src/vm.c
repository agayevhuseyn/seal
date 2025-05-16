#include "vm.h"

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
#define ERROR(...) (fprintf(stderr, __VA_ARGS__), exit(EXIT_FAILURE))
#define ERROR_OP(op, left, right) ERROR("\'%s\' operator is not supported for \'%s\' and \'%s\'\n", #op, seal_type_name(left.type), seal_type_name(right.type))

#define PUSH_NULL(vm)        PUSH(vm, GET_CONST(vm, NULL_IDX))
#define PUSH_INT(vm, val)    PUSH(vm, sval(SEAL_INT, _int, val))
#define PUSH_FLOAT(vm, val)  PUSH(vm, sval(SEAL_FLOAT, _float, val))
#define PUSH_STRING(vm, val) PUSH(vm, sval(SEAL_STRING, string, val))
#define PUSH_BOOL(vm, val)   PUSH(vm, GET_CONST(vm, val ? TRUE_IDX : FALSE_IDX))

#define AS_INT(val)    (val.as._int)
#define AS_FLOAT(val)  (val.as._float)
#define AS_STRING(val) (val.as.string)
#define AS_BOOL(val)   (val.as._bool)

#define IS_NULL(val)   (val.type == SEAL_NULL)
#define IS_INT(val)    (val.type == SEAL_INT)
#define IS_FLOAT(val)  (val.type == SEAL_FLOAT)
#define IS_STRING(val) (val.type == SEAL_STRING)
#define IS_BOOL(val)   (val.type == SEAL_BOOL)

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
    ERROR_OP(op, left, right); \
} while (0)

#define MOD_OP(vm, left, right) do { \
  if (IS_INT(left) && IS_INT(right)) \
    PUSH_INT(vm, AS_INT(left) % AS_INT(right)); \
  else \
    ERROR_OP(%, left, right); \
} while (0)

#define BITWISE_OP(vm, left, right, op) do { \
  if (IS_INT(left) && IS_INT(right)) \
    PUSH_INT(vm, AS_INT(left) op AS_INT(right)); \
  else \
    ERROR_OP(op, left, right); \
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
    ERROR_OP(op, left, right); \
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
    ERROR_OP(op, left, right); \
} while (0)


void init_vm(vm_t* vm, cout_t* cout)
{
  vm->const_pool_ptr = cout->const_pool;
  vm->label_ptr = cout->labels;
  vm->sp = vm->stack;
  vm->ip = vm->bytecodes = cout->bytecodes;
  hashmap_init(&vm->globals, 255);
}

void eval_vm(vm_t* vm)
{
  while (true) {
    uint8_t op = FETCH(vm);

    uint16_t idx, addr;
    svalue_t right, left;
    struct h_entry* entry;
    const char* sym;

    switch (op) {
      case OP_HALT:
        printf("Finish\n");
        return;
      case OP_PUSH:
        idx = FETCH(vm) << 8;
        idx |= FETCH(vm);
        PUSH(vm, GET_CONST(vm, idx));
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
      case OP_BAND:
        right = POP(vm);
        left  = POP(vm);
        BITWISE_OP(vm, left, right, &);
        break;
      case OP_BOR:
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
      case OP_JUMP:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        JUMP(vm, addr);
        break;
      case OP_JFALSE:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        if (!AS_BOOL(POP(vm))) {
          JUMP(vm, addr);
        }
        break;
      case OP_JTRUE:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        if (AS_BOOL(POP(vm))) {
          JUMP(vm, addr);
        }
        break;
      case OP_GET_GLOBAL:
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        sym = AS_STRING(GET_CONST(vm, addr));
        entry = hashmap_search(&vm->globals, sym);
        if (entry->key) {
          PUSH(vm, entry->val);
        } else {
          ERROR("vm: %s is not defined\n", sym);
        }
        break;
      case OP_SET_GLOBAL:
        DUP(vm);
        addr = FETCH(vm) << 8;
        addr |= FETCH(vm);
        hashmap_insert(&vm->globals, AS_STRING(GET_CONST(vm, addr)), POP(vm));
        break;
      case OP_PRINT: {
        size_t argc = FETCH(vm);
        svalue_t args[argc];
        for (int i = argc - 1; i >= 0; i--)
          args[i] = POP(vm);

        for (int i = 0; i < argc; i++) {
          svalue_t s = args[i];

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
            default:
              printf("UNRECOGNIZED DATA TYPE TO PRINT\n");
          }
        }
        printf("\n");
      }
      break;
      default: fprintf(stderr, "unrecognized op type: %s\n", op_name(op)); return;
    }
  }
}
