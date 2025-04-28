#include "compiler.h"
#include "sealtypes.h"

#define START_BYTECODE_CAP 1024

#define PUSH(cout, byte) do { \
    if (cout->bytecode_size >= cout->bytecode_cap) { \
      cout->bytecodes = SEAL_REALLOC(cout->bytecodes, sizeof(uint8_t) * (cout->bytecode_cap *= 2)); \
    } \
    cout->bytecodes[cout->bytecode_size++] = byte; \
  } while (0)

#define CUR_IDX(cout) (cout->bytecode_size) /* returns index of current empty byte */
#define CUR_ADDR(cout) (&(cout->bytecodes[CUR_IDX(cout)])) /* returns address of current empty byte */

#define PUSH_CONST(cout, val) ( \
    /* check bounds */ \
    *cout->const_pool_ptr++ = val)

#define CONST_IDX(cout) ((uint16_t)(cout->const_pool_ptr - cout->const_pool))

#define PUSH_LABEL(cout, addr) ( \
    /* check bounds */ \
    *cout->label_ptr++ = addr)

#define LABEL_IDX(cout) ((uint8_t)(cout->label_ptr - cout->labels - 1)) /* WARNING!! only use this after pushing label */ \

void compile(cout_t* cout, ast_t* node)
{
  cout->bytecode_size = 0;
  cout->bytecode_cap  = START_BYTECODE_CAP;
  cout->const_pool_ptr = cout->const_pool;
  cout->label_ptr = cout->labels;
  cout->bytecodes = SEAL_CALLOC(START_BYTECODE_CAP, sizeof(uint8_t));

  PUSH_CONST(cout, (svalue_t) { .type = SEAL_NULL }); /* push constant null */
  PUSH_CONST(cout, sval(SEAL_BOOL, _bool, true)); /* push constant true */
  PUSH_CONST(cout, sval(SEAL_BOOL, _bool, false)); /* push constant false */

  compile_node(cout, node);

  PUSH(cout, OP_PRINT);
  PUSH(cout, OP_HALT); /* push halt opcode for termination */
}
static void compile_node(cout_t* cout, ast_t* node)
{
  switch (node->type) {
    case AST_COMP: for (int i = 0; i < node->comp.stmt_size - 1; i++) { compile_node(cout, node->comp.stmts[i]); } break;
    case AST_IF: compile_if(cout, node);
    case AST_BINARY: compile_binary(cout, node); break;
    case AST_NULL:
    case AST_INT:
    case AST_FLOAT:
    case AST_STRING:
    case AST_BOOL:
      compile_val(cout, node);
      break;
  }
}
static void compile_if(cout_t* cout, ast_t* node)
{
  PUSH(cout, OP_JZ);
  uint8_t* addr = CUR_ADDR(cout);
  PUSH(cout, 0); /* just push 0 for now */

  compile_node(cout, node->_if.comp);
  PUSH_LABEL(cout, CUR_IDX(cout));
  *addr = LABEL_IDX(cout);
}
static void compile_binary(cout_t* cout, ast_t* node)
{
  compile_node(cout, node->binary.left);
  compile_node(cout, node->binary.right);
  
  uint8_t opcode;
  switch (node->binary.op_type) {
    case TOK_PLUS : opcode = OP_ADD; break;
    case TOK_MINUS: opcode = OP_SUB; break;
    case TOK_MUL  : opcode = OP_MUL; break;
    case TOK_DIV  : opcode = OP_DIV; break;
    case TOK_MOD  : opcode = OP_MOD; break;
    case TOK_EQ   : opcode = OP_EQ;  break;
    case TOK_NE   : opcode = OP_NE;  break;
    case TOK_GT   : opcode = OP_GT;  break;
    case TOK_GE   : opcode = OP_GE;  break;
    case TOK_LT   : opcode = OP_LT;  break;
    case TOK_LE   : opcode = OP_LE;  break;
  }

  PUSH(cout, opcode);
}
static void compile_val(cout_t* cout, ast_t* node)
{
  PUSH(cout, OP_PUSH); /* push opcode */

  svalue_t val;
  switch (node->type) {
    case AST_INT   : val.type = SEAL_INT;    val.as._int   = node->integer.val; break;
    case AST_FLOAT : val.type = SEAL_FLOAT;  val.as._float = node->floating.val; break;
    case AST_STRING: val.type = SEAL_STRING; val.as.string = node->string.val; break;
    case AST_BOOL:
      val.type = SEAL_BOOL;
      val.as._bool  = node->boolean.val;
      PUSH(cout, 0);
      PUSH(cout, val.as._bool ? TRUE_IDX : FALSE_IDX);
      return;
    case AST_NULL:
      val.type = SEAL_NULL;
      PUSH(cout, 0);
      PUSH(cout, NULL_IDX);
      return;
  }
  uint16_t idx = CONST_IDX(cout);
  PUSH(cout, (uint8_t)idx >> 8);
  PUSH(cout, (uint8_t)idx);
  PUSH_CONST(cout, val); /* push constant into pool */
}
