#ifndef SEAL_VM_H
#define SEAL_VM_H

#include "seal.h"
#include "compiler.h"
#include "sealtypes.h"

#define STACK_SIZE (0xFFFF + 1)

typedef struct vm vm_t;

struct vm {
 svalue_t* const_pool_ptr; /* pointer to pool for constant values */
 uint16_t* label_ptr; /* pointer to array of labels */
 uint8_t*  bytecodes;  /* bytecode array (do not increment this) */ 
 uint8_t*  ip;    /* instruction pointer */
 svalue_t  stack[STACK_SIZE]; /* stack */
 svalue_t* sp;    /* stack pointer */
 svalue_t* bp;    /* base pointer */
};

void init_vm(vm_t* vm, cout_t* cout);
void eval_vm(vm_t* vm);

static void print_stack(vm_t* vm)
{
  svalue_t* sp = vm->sp;
  while (sp > vm->stack) {
    svalue_t val = *(--sp);
    switch (val.type) {
      case SEAL_NULL:
        printf("null\n");
        break;
      case SEAL_INT:
        printf("%lld\n", val.as._int);
        break;
      case SEAL_FLOAT:
        printf("%f\n", val.as._float);
        break;
      case SEAL_STRING:
        printf("%s\n", val.as.string);
        break;
      case SEAL_BOOL:
        printf("%s\n", val.as._bool ? "true" : "false");
        break;
      default:
        printf("STACK TYPE UNRECOGNIZED: %d\n", val.type);
        break;
    }
  }
}

#endif /* SEAL_VM_H */
