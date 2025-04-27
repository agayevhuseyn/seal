#ifndef SEAL_COMPILER_H
#define SEAL_COMPILER_H

#include "seal.h"
#include "ast.h"
#include "sealtypes.h"
#include "bytecode.h"

typedef struct cout cout_t;

#define CONST_POOL_SIZE (0xFFFF + 1)    /* 65536 */

struct cout {
  uint8_t* bytecodes;     /* bytecode array */
  size_t   bytecode_size; /* bytecode size */
  size_t   bytecode_cap;  /* bytecode capacity */
  svalue_t  const_pool[CONST_POOL_SIZE]; /* pool for constant values */
  svalue_t* const_pool_idx; /* index for tracking constant pool values */
};

void compile(cout_t*, ast_t*); /* init cout and compile root node into bytecode */
static void compile_node(cout_t*, ast_t*); /* compile any node into bytecode */
static void compile_binary(cout_t*, ast_t*);
static void compile_val(cout_t*, ast_t*);

#endif /* SEAL_COMPILER_H */
