#include "compiler.h"
#include "ast.h"
#include "sealtypes.h"

#define START_BYTECODE_CAP 1024

#define EMIT(cout, byte) do { \
    if (cout->bytecode_size >= cout->bytecode_cap) { \
      cout->bytecodes = SEAL_REALLOC(cout->bytecodes, sizeof(seal_byte) * (cout->bytecode_cap *= 2)); \
    } \
    cout->bytecodes[cout->bytecode_size++] = (seal_byte)(byte); \
  } while (0)

#define EMIT_DUMMY(cout, times) for (int i = 0; i < (times); i++) { \
    EMIT(cout, 0); \
  }

#define REPLACE_16BITS_INDEX(cout, addr, idx) do { \
    *(addr)++ = (seal_byte)((idx) >> 8); \
    *(addr)   = (seal_byte)(idx); \
  } while (0)

#define SET_16BITS_INDEX(cout, idx) do { \
    EMIT(cout, (seal_word)(idx) >> 8); \
    EMIT(cout, (seal_word)(idx)); \
  } while (0)

#define CUR_IDX(cout) (cout->bytecode_size) /* returns index of current empty byte */
#define CUR_ADDR(cout) (&(cout->bytecodes[CUR_IDX(cout)])) /* returns address of current empty byte */

#define PUSH_CONST(cout, val) ( \
    /* check bounds */ \
    *cout->const_pool_ptr++ = (val))

#define CONST_IDX(cout) ((seal_word)(cout->const_pool_ptr - cout->const_pool - 1)) /* WARNING!! only use this before after constant */

#define PUSH_LABEL(cout, addr) ( \
    /* check bounds */ \
    *cout->label_ptr++ = (addr))

#define LABEL_IDX(cout) ((seal_word)(cout->label_ptr - cout->labels - 1)) /* WARNING!! only use this after pushing label */

#define __compiler_error(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    exit(1); \
  } while (0)

#define AUG_ASSIGN_OP_TYPE(type) ( \
  (type) == TOK_PLUS_ASSIGN ? OP_ADD : \
  (type) == TOK_MINUS_ASSIGN ? OP_SUB : \
  (type) == TOK_MUL_ASSIGN ? OP_MUL : \
  (type) == TOK_DIV_ASSIGN ? OP_DIV : \
  (type) == TOK_MOD_ASSIGN ? OP_MOD : \
  (type) == TOK_BAND_ASSIGN ? OP_AND : \
  (type) == TOK_BOR_ASSIGN ? OP_OR : \
  (type) == TOK_XOR_ASSIGN ? OP_XOR : \
  (type) == TOK_SHL_ASSIGN ? OP_SHL : \
  (type) == TOK_SHR_ASSIGN ? OP_SHR : \
  -1)


void compile(cout_t* cout, ast_t* node)
{
  cout->bytecode_size = 0;
  cout->bytecode_cap  = START_BYTECODE_CAP;
  cout->const_pool_ptr = cout->const_pool;
  cout->label_ptr = cout->labels;
  cout->bytecodes = SEAL_CALLOC(START_BYTECODE_CAP, sizeof(seal_byte));

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
        case AST_ASSIGN:
        case AST_VAR_REF:
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
  case AST_SKIP: compile_skip(cout); break;
  case AST_STOP: compile_stop(cout); break;
  case AST_UNARY: compile_unary(cout, node); break;
  case AST_BINARY: compile_binary(cout, node); break;
  case AST_BINARY_BOOL: compile_logical_binary(cout, node); break;
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
  seal_byte *end_addrs[jmp_size], **end_addr = end_addrs, *next_addr = NULL;

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
  size_t skip_start_size = cout->skip_size;
  size_t stop_start_size = cout->stop_size;

  PUSH_LABEL(cout, CUR_IDX(cout));
  seal_word start = LABEL_IDX(cout);
  compile_node(cout, node->_while.cond);

  EMIT(cout, OP_JFALSE);
  seal_byte* end_addr = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2);

  compile_node(cout, node->_while.comp);

  EMIT(cout, OP_JUMP);
  SET_16BITS_INDEX(cout, start);
  PUSH_LABEL(cout, CUR_IDX(cout));
  REPLACE_16BITS_INDEX(cout, end_addr, LABEL_IDX(cout));

  if (skip_start_size < cout->skip_size) {
    for (int i = skip_start_size; i < cout->skip_size; i++) {
      REPLACE_16BITS_INDEX(cout, cout->skip_addr_stack[i], start);
    }
  }
  cout->skip_size = skip_start_size;
  if (stop_start_size < cout->stop_size) {
    for (int i = stop_start_size; i < cout->stop_size; i++) {
      REPLACE_16BITS_INDEX(cout, cout->stop_addr_stack[i], LABEL_IDX(cout));
    }
  }
  cout->stop_size = stop_start_size;
}
static void compile_dowhile(cout_t* cout, ast_t* node)
{
  PUSH_LABEL(cout, CUR_IDX(cout));
  seal_word start = LABEL_IDX(cout);

  compile_node(cout, node->_while.comp);
  compile_node(cout, node->_while.cond);

  EMIT(cout, OP_JTRUE);
  SET_16BITS_INDEX(cout, start);
}
static inline void compile_skip(cout_t* cout)
{
  if (cout->skip_size >= UNCOND_JMP_MAX_SIZE)
    __compiler_error("maximum number of skip has exceeded");
  EMIT(cout, OP_JUMP);
  cout->skip_addr_stack[cout->skip_size++] = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2);
}
static inline void compile_stop(cout_t* cout)
{
  if (cout->stop_size >= UNCOND_JMP_MAX_SIZE)
    __compiler_error("maximum number of stop has exceeded");
  EMIT(cout, OP_JUMP);
  cout->stop_addr_stack[cout->stop_size++] = CUR_ADDR(cout);
  EMIT_DUMMY(cout, 2);
}
static void compile_unary(cout_t* cout, ast_t* node)
{
  compile_node(cout, node->unary.expr);

  seal_byte opcode;
  switch (node->unary.op_type) {
  case TOK_NOT   : opcode = OP_NOT; break;
  case TOK_MINUS : opcode = OP_NEG; break;
  case TOK_PLUS  : return;
  case TOK_TYPEOF: opcode = OP_TYPOF; break;
  case TOK_BNOT  : opcode = OP_BNOT; break;
  }

  EMIT(cout, opcode);
}
static void compile_binary(cout_t* cout, ast_t* node)
{
  compile_node(cout, node->binary.left);
  compile_node(cout, node->binary.right);
  
  seal_byte opcode;
  switch (node->binary.op_type) {
  case TOK_PLUS : opcode = OP_ADD; break;
  case TOK_MINUS: opcode = OP_SUB; break;
  case TOK_MUL  : opcode = OP_MUL; break;
  case TOK_DIV  : opcode = OP_DIV; break;
  case TOK_MOD  : opcode = OP_MOD; break;
  case TOK_BAND : opcode = OP_AND; break;
  case TOK_BOR  : opcode = OP_OR;  break;
  case TOK_XOR  : opcode = OP_XOR; break;
  case TOK_SHL  : opcode = OP_SHL; break;
  case TOK_SHR  : opcode = OP_SHR; break;
  case TOK_EQ   : opcode = OP_EQ;  break;
  case TOK_NE   : opcode = OP_NE;  break;
  case TOK_GT   : opcode = OP_GT;  break;
  case TOK_GE   : opcode = OP_GE;  break;
  case TOK_LT   : opcode = OP_LT;  break;
  case TOK_LE   : opcode = OP_LE;  break;
  }

  EMIT(cout, opcode);
}
static void compile_logical_binary(cout_t* cout, ast_t* node)
{
  compile_node(cout, node->binary.left);

  EMIT(cout, OP_DUP); /* duplicate to compare for jumping */

  if (node->binary.op_type == TOK_AND) {
    EMIT(cout, OP_JFALSE);
  } else {
    EMIT(cout, OP_JTRUE);
  }
  seal_byte* end_addr = CUR_ADDR(cout); /* store end address */
  EMIT_DUMMY(cout, 2); /* push zero bytes */
  EMIT(cout, OP_POP); /* pop first value if no jump */

  compile_node(cout, node->binary.right); /* compile right side */

  PUSH_LABEL(cout, CUR_IDX(cout)); /* push end label */
  REPLACE_16BITS_INDEX(cout, end_addr, LABEL_IDX(cout)); /* replace zero bytes with end label index */
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
  SET_16BITS_INDEX(cout, CONST_IDX(cout));
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

  EMIT(cout, (seal_byte)node->func_call.arg_size);
}
static void compile_assign(cout_t* cout, ast_t* node)
{
  int lval_type = node->assign.var->type;
  svalue_t sym;

  if (node->assign.op_type == TOK_ASSIGN) {
    compile_node(cout, node->assign.expr);

    switch (lval_type) {
    case AST_VAR_REF:
      EMIT(cout, OP_SET_GLOBAL);
      sym = sval(SEAL_STRING, string, node->assign.var->var_ref.name);
      PUSH_CONST(cout, sym);
      SET_16BITS_INDEX(cout, CONST_IDX(cout));
      break;
    default:
      __compiler_error("assigning to %s is not implemented yet", hast_type_name(lval_type));
      break;
    }
  }
  else {
    seal_word sym_idx;
    int aug_type = node->assign.op_type;
    switch (lval_type) {
    case AST_VAR_REF:
      EMIT(cout, OP_GET_GLOBAL);
      sym = sval(SEAL_STRING, string, node->assign.var->var_ref.name);
      PUSH_CONST(cout, sym);
      sym_idx = CONST_IDX(cout);
      SET_16BITS_INDEX(cout, sym_idx);
      compile_node(cout, node->assign.expr);
      EMIT(cout, AUG_ASSIGN_OP_TYPE(aug_type));
      EMIT(cout, OP_SET_GLOBAL);
      SET_16BITS_INDEX(cout, sym_idx);
      break;
    default:
      __compiler_error("assigning to %s is not implemented yet", hast_type_name(lval_type));
      break;
    }
  }
}
static void compile_var_ref(cout_t* cout, ast_t* node)
{
  EMIT(cout, OP_GET_GLOBAL);
  PUSH_CONST(cout, sval(SEAL_STRING, string, node->var_ref.name));
  SET_16BITS_INDEX(cout, CONST_IDX(cout));
}
