#include "vm.h"

#define FETCH(vm) (*vm->ip++)
#define PUSH(vm, val) (*vm->sp++ = val)
#define POP(vm) (*(--(vm->sp)))
#define ERROR(err) (fprintf(stderr, "seal vm: %s\n", err), exit(EXIT_FAILURE))
#define ERROR_OP(op, left, right) do { \
    char err[ERR_LEN]; \
    sprintf(err, "\'%s\' operator is not supported for \'%s\' and \'%s\'", #op, seal_type_name(left.type), seal_type_name(right.type)); \
    ERROR(err); \
  } while (0)

#define sval(t, mem, val) (svalue_t) { .type = t, .as.mem = val}

#define PUSH_INT(vm, val)    PUSH(vm, sval(SEAL_INT, _int, val))
#define PUSH_FLOAT(vm, val)  PUSH(vm, sval(SEAL_FLOAT, _float, val))
#define PUSH_STRING(vm, val) PUSH(vm, sval(SEAL_STRING, string, val))
#define PUSH_BOOL(vm, val)   PUSH(vm, sval(SEAL_BOOL, _bool, val))

#define AS_INT(val)    (val.as._int)
#define AS_FLOAT(val)  (val.as._float)
#define AS_STRING(val) (val.as.string)
#define AS_BOOL(val)   (val.as._bool)

#define IS_NULL(val)   (val.type == SEAL_NULL)
#define IS_INT(val)    (val.type == SEAL_INT)
#define IS_FLOAT(val)  (val.type == SEAL_FLOAT)
#define IS_STRING(val) (val.type == SEAL_STRING)
#define IS_BOOL(val)   (val.type == SEAL_BOOL)

static const svalue_t sval_true  = sval(SEAL_BOOL, _bool, true);
static const svalue_t sval_false = sval(SEAL_BOOL, _bool, false);
static const svalue_t sval_null  = (svalue_t) { .type = SEAL_NULL };

void init_vm(vm_t* vm, cout_t* cout)
{
  vm->const_pool_ptr = cout->const_pool;
  vm->sp = vm->stack;
  vm->ip = cout->bytecodes;
}

void eval_vm(vm_t* vm)
{
  while (true) {
    uint8_t op = FETCH(vm);

    switch (op) {
      case OP_HALT: printf("Finish\n"); return;
      case OP_PUSH: {
          uint8_t left  = FETCH(vm);
          uint8_t right = FETCH(vm);
          uint16_t idx = left ? (left << 8) | right : right;
          PUSH(vm, vm->const_pool_ptr[idx]);
        }
        break;
      case OP_POP: POP(vm); break;
      case OP_ADD: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) + AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) + AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) + (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(+, left, right);
      }
      break;
      case OP_SUB: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) - AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) - AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) - (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(-, left, right);
      }
      break;
      case OP_MUL: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) * AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) * AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) * (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(*, left, right);
      }
      break;
      case OP_DIV: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) / AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) / AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) / (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(*, left, right);
      }
      break;
      case OP_MOD: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) % AS_INT(right));
        else
          ERROR_OP(*, left, right);
      }
      break;
      case OP_EQ: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) == AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) == AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) == (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(==, left, right);
      }
      break;
      case OP_NE: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) != AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) != AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) != (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(!=, left, right);
      }
      break;
      case OP_GT: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) > AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) > AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) > (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(>, left, right);
      }
      break;
      case OP_GE: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) >= AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) >= AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) >= (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(>=, left, right);
      }
      break;
      case OP_LT: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) < AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) < AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) < (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(<, left, right);
      }
      break;
      case OP_LE: {
        svalue_t right = POP(vm);
        svalue_t left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) <= AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) <= AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) <= (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(<=, left, right);
      }
      break;
      case OP_JMP:
      case OP_JZ:
      case OP_JNZ:
      case OP_PRINT: {
        svalue_t s = POP(vm);
        switch (s.type) {
          case SEAL_INT:
            printf("%lld\n", s.as._int);
            break;
          case SEAL_FLOAT:
            printf("%f\n", s.as._float);
            break;
          case SEAL_STRING:
            printf("%s\n", s.as.string);
            break;
          case SEAL_BOOL:
            printf("%s\n", s.as._bool ? "true" : "false");
            break;
          case SEAL_NULL:
            printf("null\n");
            break;
          default:
            printf("UNRECOGNIZED DATA TYPE TO PRINT\n");
        }
      }
      break;
      default: fprintf(stderr, "unrecognized op type: %d\n", op); return;
    }
  }
}
