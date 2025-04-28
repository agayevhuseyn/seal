#ifndef SEAL_COMPILER_H
#define SEAL_COMPILER_H

#include "seal.h"
#include "ast.h"
#include "sealtypes.h"
#include "bytecode.h"

typedef struct cout cout_t;

#define CONST_POOL_SIZE (0xFFFF + 1)    /* 65536 */

#define NULL_IDX  0
#define TRUE_IDX  1
#define FALSE_IDX 2

#define PRINT_CONST_POOL(cout) for (int i = 0; i < cout.const_pool_ptr - cout.const_pool; i++) { \
    if (i == 0) printf("CONST POOL START-------\n"); \
    svalue_t s = cout.const_pool[i]; \
    switch (s.type) { \
      case SEAL_INT: \
        printf("%lld\n", s.as._int); \
        break; \
      case SEAL_FLOAT: \
        printf("%f\n", s.as._float); \
        break; \
      case SEAL_STRING: \
        printf("\'%s\'\n", s.as.string); \
        break; \
      case SEAL_BOOL: \
        printf("%s\n", s.as._bool ? "true" : "false"); \
        break; \
      case SEAL_NULL: \
        printf("null\n"); \
        break; \
      default: \
        printf("UNRECOGNIZED DATA TYPE TO PRINT\n"); \
    } \
    if (i - 1 == cout.const_pool_ptr - cout.const_pool) printf("CONST POOL END-------\n"); \
  }

#define LABEL_SIZE (0xFF + 1)

struct cout {
  uint8_t* bytecodes;     /* bytecode array */
  size_t   bytecode_size; /* bytecode size */
  size_t   bytecode_cap;  /* bytecode capacity */
  svalue_t  const_pool[CONST_POOL_SIZE]; /* pool for constant values */
  svalue_t* const_pool_ptr; /* pointer for tracking constant pool values */
  uint16_t labels[LABEL_SIZE]; /* array of labels to store */
  uint16_t* label_ptr; /* pointer for tracking labels */
};

void compile(cout_t*, ast_t*); /* init cout and compile root node into bytecode */
static void compile_node(cout_t*, ast_t*); /* compile any node into bytecode */
static void compile_if(cout_t*, ast_t*);
static void compile_binary(cout_t*, ast_t*);
static void compile_val(cout_t*, ast_t*);

#endif /* SEAL_COMPILER_H */
