#include "vm.h"

#define FETCH(vm) (*vm->ip++)
#define PUSH(vm, val) (*vm->sp++ = val)
#define POP(vm) (*(--(vm->sp)))
#define JUMP(vm, addr) (vm->ip = &vm->bytecodes[vm->label_ptr[addr]])
#define GET_CONST(vm, i) (vm->const_pool_ptr[i])
#define ERROR(err) (fprintf(stderr, "seal vm: %s\n", err), exit(EXIT_FAILURE))
#define ERROR_OP(op, left, right) do { \
    char err[ERR_LEN]; \
    sprintf(err, "\'%s\' operator is not supported for \'%s\' and \'%s\'", #op, seal_type_name(left.type), seal_type_name(right.type)); \
    ERROR(err); \
  } while (0)

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

void init_vm(vm_t* vm, cout_t* cout)
{
  vm->const_pool_ptr = cout->const_pool;
  vm->label_ptr = cout->labels;
  vm->sp = vm->stack;
  vm->ip = vm->bytecodes = cout->bytecodes;
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
          PUSH(vm, GET_CONST(vm, idx));
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
        else if (IS_BOOL(left) && IS_BOOL(right))
          PUSH_BOOL(vm, AS_BOOL(left) == AS_BOOL(right));
        else if (IS_STRING(left) && IS_STRING(right))
          PUSH_BOOL(vm, strcmp(AS_STRING(left), AS_STRING(right)) == 0);
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
        else if (IS_BOOL(left) && IS_BOOL(right))
          PUSH_BOOL(vm, AS_BOOL(left) != AS_BOOL(right));
        else if (IS_STRING(left) && IS_STRING(right))
          PUSH_BOOL(vm, strcmp(AS_STRING(left), AS_STRING(right)) != 0);
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
        JUMP(vm, FETCH(vm));
        break;
      case OP_JZ: {
        uint16_t addr = FETCH(vm);
        if (!AS_BOOL(POP(vm))) {
          JUMP(vm, addr);
        }
      }
      break;
      case OP_JNZ:
        break;
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
