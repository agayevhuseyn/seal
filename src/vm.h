#ifndef SEAL_VM_H
#define SEAL_VM_H

#include "seal.h"
#include "op.h"
#include "compiler.h"
#include "sealtypes.h"

#define STACK_SIZE (0xFFFF + 1)

typedef struct vm vm_t;

struct vm {
 svalue_t* const_pool_ptr; /* pointer to pool for constant values */
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
  while (sp >= vm->stack) {
    printf("%lld\n", (*sp--).as._int);
  }
}

#endif /* SEAL_VM_H */
