#include "vm.h"

#define FETCH(vm) (*vm->ip++)
#define PUSH(vm, val) do { \
    if (vm->sp - vm->stack == STACK_SIZE) \
      ERROR("stack overflow"); \
    (*vm->sp++ = val); \
  } while (0)
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

    uint16_t idx, addr;
    svalue_t right, left;

    switch (op) {
      case OP_HALT: printf("Finish\n"); return;
      case OP_PUSH:
        idx = FETCH(vm) << 8;
        idx |= FETCH(vm);
        PUSH(vm, GET_CONST(vm, idx));
        break;
      case OP_POP: POP(vm); break;
      case OP_ADD:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) + AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) + AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) + (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(+, left, right);
        break;
      case OP_SUB:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) - AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) - AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) - (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(-, left, right);
        break;
      case OP_MUL:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) * AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) * AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) * (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(*, left, right);
        break;
      case OP_DIV:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) / AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_FLOAT(vm, AS_FLOAT(left) / AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_FLOAT(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) / (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(*, left, right);
        break;
      case OP_MOD:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_INT(vm, AS_INT(left) % AS_INT(right));
        else
          ERROR_OP(*, left, right);
        break;
      case OP_EQ:
        right = POP(vm);
        left  = POP(vm);
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
        break;
      case OP_NE:
        right = POP(vm);
        left  = POP(vm);
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
        break;
      case OP_GT:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) > AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) > AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) > (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(>, left, right);
        break;
      case OP_GE:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) >= AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) >= AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) >= (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(>=, left, right);
        break;
      case OP_LT:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) < AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) < AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) < (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(<, left, right);
        break;
      case OP_LE:
        right = POP(vm);
        left  = POP(vm);
        if (IS_INT(left) && IS_INT(right))
          PUSH_BOOL(vm, AS_INT(left) <= AS_INT(right));
        else if (IS_FLOAT(left) && IS_FLOAT(right))
          PUSH_BOOL(vm, AS_FLOAT(left) <= AS_FLOAT(right));
        else if (IS_INT(left) && IS_FLOAT(right) || IS_FLOAT(left) && IS_INT(right))
          PUSH_BOOL(vm, (IS_INT(left) ? AS_INT(left) : AS_FLOAT(left)) <= (IS_INT(right) ? AS_INT(right) : AS_FLOAT(right)));
        else
          ERROR_OP(<=, left, right);
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
      default: fprintf(stderr, "unrecognized op type: %d\n", op); return;
    }
  }
}
