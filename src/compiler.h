#ifndef SEAL_COMPILER_H
#define SEAL_COMPILER_H

#include "seal.h"
#include "ast.h"
#include "sealtypes.h"
#include "bytecode.h"

typedef struct cout cout_t;

#define CONST_POOL_SIZE (0xFFFF + 1)    /* 65536 */
#define LABEL_SIZE      (0xFFFF + 1)    /* 65536 */
#define UNCOND_JMP_MAX_SIZE 1024

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

struct cout {
  uint8_t* bytecodes;     /* bytecode array */
  size_t   bytecode_size; /* bytecode size */
  size_t   bytecode_cap;  /* bytecode capacity */
  svalue_t  const_pool[CONST_POOL_SIZE]; /* pool for constant values */
  svalue_t* const_pool_ptr; /* pointer for tracking constant pool values */
  uint16_t labels[LABEL_SIZE]; /* array of labels to store */
  uint16_t* label_ptr; /* pointer for tracking labels */
  uint8_t* skip_addr_stack[UNCOND_JMP_MAX_SIZE]; /* stack for skip statements */
  uint8_t* stop_addr_stack[UNCOND_JMP_MAX_SIZE]; /* stack for stop statements */
  size_t skip_size; /* skip statements size */
  size_t stop_size; /* stop statements size */
};

void compile(cout_t*, ast_t*); /* init cout and compile root node into bytecode */
static void compile_node(cout_t*, ast_t*); /* compile any node into bytecode */
static void compile_if(cout_t*, ast_t*);
static void compile_while(cout_t*, ast_t*);
static void compile_dowhile(cout_t*, ast_t*);
static inline void compile_skip(cout_t*);
static inline void compile_stop(cout_t*);
static void compile_unary(cout_t*, ast_t*);
static void compile_binary(cout_t*, ast_t*);
static void compile_logical_binary(cout_t*, ast_t*);
static void compile_val(cout_t*, ast_t*);
static void compile_func_call(cout_t*, ast_t*);
static void compile_assign(cout_t*, ast_t*);
static void compile_var_ref(cout_t*, ast_t*);

#endif /* SEAL_COMPILER_H */
