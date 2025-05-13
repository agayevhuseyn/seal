#include "compiler.h"
#include "ast.h"
#include "sealtypes.h"

#define START_BYTECODE_CAP 1024

#define EMIT(cout, byte) do { \
    if (cout->bytecode_size >= cout->bytecode_cap) { \
      cout->bytecodes = SEAL_REALLOC(cout->bytecodes, sizeof(uint8_t) * (cout->bytecode_cap *= 2)); \
    } \
    cout->bytecodes[cout->bytecode_size++] = (uint8_t)(byte); \
  } while (0)

#define EMIT_DUMMY(cout, times) for (int i = 0; i < (times); i++) { \
    EMIT(cout, 0); \
  }

#define REPLACE_16BITS_INDEX(cout, addr, idx) do { \
    *(addr)++ = (uint8_t)((idx) >> 8); \
    *(addr)   = (uint8_t)(idx); \
  } while (0)

#define CUR_IDX(cout) (cout->bytecode_size) /* returns index of current empty byte */
#define CUR_ADDR(cout) (&(cout->bytecodes[CUR_IDX(cout)])) /* returns address of current empty byte */

#define PUSH_CONST(cout, val) ( \
    /* check bounds */ \
    *cout->const_pool_ptr++ = (val))

#define CONST_IDX(cout) ((uint16_t)(cout->const_pool_ptr - cout->const_pool - 1)) /* WARNING!! only use this before after constant */

#define PUSH_LABEL(cout, addr) ( \
    /* check bounds */ \
    *cout->label_ptr++ = (addr))

#define LABEL_IDX(cout) ((uint16_t)(cout->label_ptr - cout->labels - 1)) /* WARNING!! only use this after pushing label */

#define __compiler_error(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    exit(1); \
  } while (0)

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

  EMIT(cout, OP_HALT); /* push halt opcode for termination */
}
static void compile_node(cout_t* cout, ast_t* node)
{
  switch (node->type) {
  case AST_COMP:
    for (int i = 0; i < node->comp.stmt_size; i++) {
      /*
       * DO NOT COMPILE DUMMY STATEMENTS (like 0, 1, 'a')
       * ONLY COMPILE FUNCTIONS AND POP THEIR VALUE 
       */
      switch (node->comp.stmts[i]->type) {
        case AST_NULL:
        case AST_INT:
        case AST_FLOAT:
        case AST_STRING:
        case AST_BOOL:
          break;
        case AST_UNARY:
        case AST_BINARY:
        case AST_BINARY_BOOL:
        case AST_TERNARY:
          compile_node(cout, node->comp.stmts[i]);
          EMIT(cout, OP_POP);
          break;
        default:
          compile_node(cout, node->comp.stmts[i]);
          break;
      }
    }
  break;
  case AST_NULL: case AST_INT: case AST_FLOAT: case AST_STRING: case AST_BOOL:
    compile_val(cout, node);
    break;
  case AST_NOP:
    break;
  case AST_IF: compile_if(cout, node); break;
  case AST_WHILE: compile_while(cout, node); break;
  case AST_DOWHILE: compile_dowhile(cout, node); break;
  case AST_BINARY: compile_binary(cout, node); break;
  case AST_FUNC_CALL: compile_func_call(cout, node); break;
  case AST_ASSIGN: compile_assign(cout, node); break;
  case AST_VAR_REF: compile_var_ref(cout, node); break;
  default:
    printf("%s is not implemented yet\n", hast_type_name(ast_type(node)));
    exit(1);
  }
}
static void compile_if(cout_t* cout, ast_t* node)
{
  int jmp_size = 0;
  ast_t* temp_node = node;
  while (temp_node->type == AST_IF && temp_node->_if.has_else) {
    temp_node = temp_node->_if._else;
    jmp_size++;
  }
  uint8_t *end_addrs[jmp_size], **end_addr = end_addrs, *next_addr = NULL;

  compile_node(cout, node->_if.cond);

  EMIT(cout, OP_JFALSE);
  next_addr = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2); /* push 2 dummy values that will be changed later */

  compile_node(cout, node->_if.comp);

  if (jmp_size == 0) {
    PUSH_LABEL(cout, CUR_IDX(cout));
    REPLACE_16BITS_INDEX(cout, next_addr, LABEL_IDX(cout));
    return;
  }
  
  EMIT(cout, OP_JUMP);
  *end_addr++ = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2);

  PUSH_LABEL(cout, CUR_IDX(cout));
  REPLACE_16BITS_INDEX(cout, next_addr, LABEL_IDX(cout));

  do {
    node = node->_if._else;

    if (node->type == AST_IF) {
      compile_node(cout, node->_if.cond);

      EMIT(cout, OP_JFALSE);
      next_addr = CUR_ADDR(cout);
      EMIT_DUMMY(cout, 2);

      compile_node(cout, node->_if.comp);

      if (end_addr - end_addrs != jmp_size) {
        if (node->_if.has_else) {
          EMIT(cout, OP_JUMP);
          *end_addr++ = CUR_ADDR(cout);
          EMIT_DUMMY(cout, 2);
        }
      }

      PUSH_LABEL(cout, CUR_IDX(cout));
      REPLACE_16BITS_INDEX(cout, next_addr, LABEL_IDX(cout));
    } else {
      compile_node(cout, node->_else.comp);
    }
  } while (node->_if.has_else);

  PUSH_LABEL(cout, CUR_IDX(cout));

  for (int i = 0; i < jmp_size; i++) {
    REPLACE_16BITS_INDEX(cout, end_addrs[i], LABEL_IDX(cout));
  }
}
static void compile_while(cout_t* cout, ast_t* node)
{
  PUSH_LABEL(cout, CUR_IDX(cout));
  uint16_t start = LABEL_IDX(cout);
  compile_node(cout, node->_while.cond);

  EMIT(cout, OP_JFALSE);
  uint8_t* end_addr = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2);

  compile_node(cout, node->_while.comp);

  EMIT(cout, OP_JUMP);
  EMIT(cout, start << start);
  EMIT(cout, start);
  PUSH_LABEL(cout, CUR_IDX(cout));
  REPLACE_16BITS_INDEX(cout, end_addr, LABEL_IDX(cout));
}
static void compile_dowhile(cout_t* cout, ast_t* node)
{
  PUSH_LABEL(cout, CUR_IDX(cout));
  uint16_t start = LABEL_IDX(cout);

  compile_node(cout, node->_while.comp);
  compile_node(cout, node->_while.cond);

  EMIT(cout, OP_JTRUE);
  EMIT(cout, start >> 8);
  EMIT(cout, start);
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

  EMIT(cout, opcode);
}
static void compile_val(cout_t* cout, ast_t* node)
{
  EMIT(cout, OP_PUSH); /* push opcode */

  svalue_t val;
  switch (node->type) {
    case AST_INT   : val.type = SEAL_INT;    val.as._int   = node->integer.val; break;
    case AST_FLOAT : val.type = SEAL_FLOAT;  val.as._float = node->floating.val; break;
    case AST_STRING: val.type = SEAL_STRING; val.as.string = node->string.val; break;
    case AST_BOOL:
      val.type = SEAL_BOOL;
      val.as._bool  = node->boolean.val;
      EMIT(cout, 0);
      EMIT(cout, val.as._bool ? TRUE_IDX : FALSE_IDX);
      return;
    case AST_NULL:
      val.type = SEAL_NULL;
      EMIT(cout, 0);
      EMIT(cout, NULL_IDX);
      return;
  }
  PUSH_CONST(cout, val); /* push constant into pool */
  uint16_t idx = CONST_IDX(cout);
  EMIT(cout, (uint8_t)(idx >> 8));
  EMIT(cout, (uint8_t)idx);
}
static void compile_func_call(cout_t* cout, ast_t* node)
{
  const char* name = node->func_call.name;

  for (int i = 0; i < node->func_call.arg_size; i++)
    compile_node(cout, node->func_call.args[i]);

  if (strcmp(name, "print") == 0)
    EMIT(cout, OP_PRINT);
  else if (strcmp(name, "scan") == 0)
    EMIT(cout, OP_SCAN);
  else
    __compiler_error("%s function is not defined\n", name);

  EMIT(cout, (uint8_t)node->func_call.arg_size);
}
static void compile_assign(cout_t* cout, ast_t* node)
{
  compile_node(cout, node->assign.expr);

  int lval_type = node->assign.var->type;
  svalue_t sym;
  switch (lval_type) {
  case AST_VAR_REF:
    EMIT(cout, OP_SET_GLOBAL);
    sym.type = SEAL_INT;
    sym.as.string = node->assign.var->var_ref.name;
    PUSH_CONST(cout, sym);
    EMIT(cout, CONST_IDX(cout) >> 8);
    EMIT(cout, CONST_IDX(cout));
    break;
  default:
    __compiler_error("assigning to %s is not implemented yet", hast_type_name(lval_type));
    break;
  }
}
static void compile_var_ref(cout_t* cout, ast_t* node)
{
  EMIT(cout, OP_GET_GLOBAL);
  svalue_t sym = { .type = SEAL_STRING, .as.string = node->var_ref.name };
  PUSH_CONST(cout, sym);
  EMIT(cout, CONST_IDX(cout) >> 8);
  EMIT(cout, CONST_IDX(cout));
}
