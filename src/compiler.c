#include "compiler.h"
#include "ast.h"
#include "hashmap.h"
#include "sealtypes.h"

#define START_BYTECODE_CAP 8

#define EMIT(bc, byte) do { \
    if ((bc)->size + 1 >= (bc)->cap) { \
      (bc)->bytecodes = SEAL_REALLOC((bc)->bytecodes, sizeof(seal_byte) * ((bc)->cap *= 2)); \
    } \
    (bc)->bytecodes[(bc)->size++] = (seal_byte)(byte); \
  } while (0)

#define EMIT_DUMMY(bc, times) for (int i = 0; i < (times); i++) { \
    EMIT(bc, 0); \
  }

#define REPLACE_16BITS_INDEX(addr, idx) do { \
    *(addr) = (seal_byte)((idx) >> 8); \
    *(addr + 1)   = (seal_byte)(idx); \
  } while (0)

#define SET_16BITS_INDEX(bc, idx) do { \
    EMIT(bc, (seal_word)(idx) >> 8); \
    EMIT(bc, (seal_word)(idx)); \
  } while (0)

#define CUR_IDX(bc) ((bc)->size) /* returns index of current empty byte */
#define CUR_ADDR(bc) (&((bc)->bytecodes[CUR_IDX(bc)])) /* returns address of current empty byte */
#define CUR_ADDR_OFFSET(bc) (&((bc)->bytecodes[CUR_IDX(bc)]) - (bc)->bytecodes) /* returns address of current empty byte */

#define PUSH_CONST(cout, val) ( \
    /* check bounds */ \
    *cout->const_pool_ptr++ = (val))

#define CONST_IDX(cout) ((seal_word)(cout->const_pool_ptr - cout->const_pool - 1)) /* WARNING!! only use this after pushing constant */

#define PUSH_LABEL(cout, addr) ( \
    /* check bounds */ \
    *cout->label_ptr++ = (addr))

#define LABEL_IDX(cout) ((seal_word)(cout->label_ptr - cout->labels - 1)) /* WARNING!! only use this after pushing label */

#define __compiler_error(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
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
  /* constant values' pool */
  static svalue_t const_pool[CONST_POOL_SIZE];
  memset(const_pool, 0, sizeof(const_pool));
  cout->const_pool_ptr = cout->const_pool = const_pool;

  /* label array */
  static seal_word labels[LABEL_SIZE];
  memset(labels, 0, sizeof(labels));
  cout->label_ptr = cout->labels = labels;

  cout->bc.bytecodes = SEAL_CALLOC(START_BYTECODE_CAP, sizeof(seal_byte));

  /* skip address offset stack */
  static size_t skip_addr_offset_stack[UNCOND_JMP_MAX_SIZE];
  memset(skip_addr_offset_stack, 0, sizeof(skip_addr_offset_stack));
  cout->skip_addr_offset_stack = skip_addr_offset_stack;

  /* stop address offset stack */
  static size_t stop_addr_offset_stack[UNCOND_JMP_MAX_SIZE];
  memset(stop_addr_offset_stack, 0, sizeof(stop_addr_offset_stack));
  cout->stop_addr_offset_stack = stop_addr_offset_stack;

  cout->skip_size = 0;
  cout->stop_size = 0;
  cout->bc.size = 0;
  cout->bc.cap  = START_BYTECODE_CAP;

  hashmap_t main_scope;
  struct h_entry entries[LOCAL_MAX];
  hashmap_init_static(&main_scope, entries, LOCAL_MAX);
  compile_node(cout, node, &main_scope, &cout->bc);

  cout->main_scope_local_size = main_scope.filled;

  EMIT(&cout->bc, OP_HALT); /* push halt opcode for termination */
}
static void compile_node(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
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
        case AST_FUNC_CALL:
          compile_node(cout, node->comp.stmts[i], scope, bc);
          EMIT(bc, OP_POP);
          break;
        default:
          compile_node(cout, node->comp.stmts[i], scope, bc);
          break;
      }
    }
  break;
  case AST_NULL: case AST_INT: case AST_FLOAT: case AST_STRING: case AST_BOOL:
    compile_val(cout, node, scope, bc);
    break;
  case AST_NOP:
    break;
  case AST_IF: compile_if(cout, node, scope, bc); break;
  case AST_WHILE: compile_while(cout, node, scope, bc); break;
  case AST_DOWHILE: compile_dowhile(cout, node, scope, bc); break;
  case AST_SKIP: compile_skip(cout, bc); break;
  case AST_STOP: compile_stop(cout, bc); break;
  case AST_UNARY: compile_unary(cout, node, scope, bc); break;
  case AST_BINARY: compile_binary(cout, node, scope, bc); break;
  case AST_BINARY_BOOL: compile_logical_binary(cout, node, scope, bc); break;
  case AST_FUNC_CALL: compile_func_call(cout, node, scope, bc); break;
  case AST_ASSIGN: compile_assign(cout, node, scope, bc); break;
  case AST_VAR_REF: compile_var_ref(cout, node, scope, bc); break;
  case AST_FUNC_DEF: compile_func_def(cout, node, scope); break;
  case AST_RETURN: compile_return(cout, node, scope, bc); break;
  case AST_LIST: compile_list(cout, node, scope, bc); break;
  default:
    printf("%s is not implemented yet\n", hast_type_name(ast_type(node)));
    exit(1);
  }
}
static void compile_if(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  int jmp_size = 0;
  ast_t* temp_node = node;
  while (temp_node->type == AST_IF && temp_node->_if.has_else) {
    temp_node = temp_node->_if._else;
    jmp_size++;
  }
  size_t end_addr_offsets[jmp_size], *end_addr_offset = end_addr_offsets, next_addr_offset;

  compile_node(cout, node->_if.cond, scope, bc);

  EMIT(bc, OP_JFALSE);
  next_addr_offset = CUR_ADDR_OFFSET(bc);
  EMIT_DUMMY(bc, 2); /* push 2 dummy values that will be changed later */

  compile_node(cout, node->_if.comp, scope, bc);

  if (jmp_size == 0) {
    PUSH_LABEL(cout, CUR_IDX(bc));
    REPLACE_16BITS_INDEX(bc->bytecodes + next_addr_offset, LABEL_IDX(cout));
    return;
  }
  
  EMIT(bc, OP_JUMP);
  *end_addr_offset++ = CUR_ADDR_OFFSET(bc);
  EMIT_DUMMY(bc, 2);

  PUSH_LABEL(cout, CUR_IDX(bc));
  REPLACE_16BITS_INDEX(bc->bytecodes + next_addr_offset, LABEL_IDX(cout));

  do {
    node = node->_if._else;

    if (node->type == AST_IF) {
      compile_node(cout, node->_if.cond, scope, bc);

      EMIT(bc, OP_JFALSE);
      next_addr_offset = CUR_ADDR_OFFSET(bc);
      EMIT_DUMMY(bc, 2);

      compile_node(cout, node->_if.comp, scope, bc);

      if (end_addr_offset - end_addr_offsets != jmp_size) {
        if (node->_if.has_else) {
          EMIT(bc, OP_JUMP);
          *end_addr_offset++ = CUR_ADDR_OFFSET(bc);
          EMIT_DUMMY(bc, 2);
        }
      }

      PUSH_LABEL(cout, CUR_IDX(bc));
      REPLACE_16BITS_INDEX(bc->bytecodes + next_addr_offset, LABEL_IDX(cout));
    } else {
      compile_node(cout, node->_else.comp, scope, bc);
    }
  } while (node->_if.has_else);

  PUSH_LABEL(cout, CUR_IDX(bc));

  for (int i = 0; i < jmp_size; i++) {
    REPLACE_16BITS_INDEX(bc->bytecodes + end_addr_offsets[i], LABEL_IDX(cout));
  }
}
static void compile_while(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  size_t skip_start_size = cout->skip_size;
  size_t stop_start_size = cout->stop_size;

  PUSH_LABEL(cout, CUR_IDX(bc));
  seal_word start = LABEL_IDX(cout);
  compile_node(cout, node->_while.cond, scope, bc);

  EMIT(bc, OP_JFALSE);
  size_t end_addr_offs = CUR_ADDR_OFFSET(bc);
  EMIT_DUMMY(bc, 2);

  compile_node(cout, node->_while.comp, scope, bc);

  EMIT(bc, OP_JUMP);
  SET_16BITS_INDEX(bc, start);
  PUSH_LABEL(cout, CUR_IDX(bc));
  REPLACE_16BITS_INDEX(bc->bytecodes + end_addr_offs, LABEL_IDX(cout));

  if (skip_start_size < cout->skip_size) {
    for (int i = skip_start_size; i < cout->skip_size; i++) {
      REPLACE_16BITS_INDEX(bc->bytecodes + cout->skip_addr_offset_stack[i], start);
    }
  }
  cout->skip_size = skip_start_size;
  if (stop_start_size < cout->stop_size) {
    for (int i = stop_start_size; i < cout->stop_size; i++) {
      REPLACE_16BITS_INDEX(bc->bytecodes + cout->stop_addr_offset_stack[i], LABEL_IDX(cout));
    }
  }
  cout->stop_size = stop_start_size;
}
static void compile_dowhile(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  PUSH_LABEL(cout, CUR_IDX(bc));
  seal_word start = LABEL_IDX(cout);

  compile_node(cout, node->_while.comp, scope, bc);
  compile_node(cout, node->_while.cond, scope, bc);

  EMIT(bc, OP_JTRUE);
  SET_16BITS_INDEX(bc, start);
}
static inline void compile_skip(cout_t* cout, struct bytechunk* bc)
{
  if (cout->skip_size >= UNCOND_JMP_MAX_SIZE)
    __compiler_error("maximum number of skip has exceeded");
  EMIT(bc, OP_JUMP);
  cout->skip_addr_offset_stack[cout->skip_size++] = CUR_ADDR_OFFSET(bc);
  EMIT_DUMMY(bc, 2);
}
static inline void compile_stop(cout_t* cout, struct bytechunk* bc)
{
  if (cout->stop_size >= UNCOND_JMP_MAX_SIZE)
    __compiler_error("maximum number of stop has exceeded");
  EMIT(bc, OP_JUMP);
  cout->stop_addr_offset_stack[cout->stop_size++] = CUR_ADDR_OFFSET(bc);
  EMIT_DUMMY(bc, 2);
}
static void compile_unary(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  compile_node(cout, node->unary.expr, scope, bc);

  seal_byte opcode;
  switch (node->unary.op_type) {
  case TOK_NOT   : opcode = OP_NOT; break;
  case TOK_MINUS : opcode = OP_NEG; break;
  case TOK_TYPEOF: opcode = OP_TYPOF; break;
  case TOK_BNOT  : opcode = OP_BNOT; break;
  case TOK_PLUS  : return;
  }

  EMIT(bc, opcode);
}
static void compile_binary(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  compile_node(cout, node->binary.left, scope, bc);
  compile_node(cout, node->binary.right, scope, bc);
  
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

  EMIT(bc, opcode);
}
static void compile_logical_binary(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  compile_node(cout, node->binary.left, scope, bc);

  EMIT(bc, OP_DUP); /* duplicate to compare for jumping */

  if (node->binary.op_type == TOK_AND) {
    EMIT(bc, OP_JFALSE);
  } else {
    EMIT(bc, OP_JTRUE);
  }
  size_t end_addr_offset = CUR_ADDR_OFFSET(bc); /* store end address offset */
  EMIT_DUMMY(bc, 2); /* push zero bytes */
  EMIT(bc, OP_POP); /* pop first value if no jump */

  compile_node(cout, node->binary.right, scope, bc); /* compile right side */

  PUSH_LABEL(cout, CUR_IDX(bc)); /* push end label */
  REPLACE_16BITS_INDEX(bc->bytecodes + end_addr_offset, LABEL_IDX(cout)); /* replace zero bytes with end label index */
}
static void compile_val(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  svalue_t val;
  switch (node->type) {
  case AST_BOOL:
    EMIT(bc, node->boolean.val ? OP_PUSH_TRUE : OP_PUSH_FALSE);
    return;
  case AST_NULL:
    EMIT(bc, OP_PUSH_NULL);
    return;
  case AST_INT:
    if (node->integer.val <= 0xFFFF) {
      EMIT(bc, OP_PUSH_INT);
      SET_16BITS_INDEX(bc, node->integer.val);
      return;
    }
    val = sval(SEAL_INT, _int, node->integer.val);
    break;
  case AST_FLOAT:
    val = sval(SEAL_FLOAT, _float, node->floating.val);
    break;
  case AST_STRING:
    val = SEAL_VALUE_STRING_STATIC(node->string.val);
    break;
  }

  EMIT(bc, OP_PUSH_CONST); /* push opcode */
  PUSH_CONST(cout, val); /* push constant into pool */
  SET_16BITS_INDEX(bc, CONST_IDX(cout));
}
static void compile_func_call(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  compile_node(cout, node->func_call.main, scope, bc);

  for (int i = 0; i < node->func_call.arg_size; i++)
    compile_node(cout, node->func_call.args[i], scope, bc);

  EMIT(bc, OP_CALL);
  EMIT(bc, node->func_call.arg_size);
}
static void compile_assign(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  int lval_type = node->assign.var->type;
  svalue_t sym;
  const char* name;
  struct h_entry* e;

  if (node->assign.op_type == TOK_ASSIGN) {
    compile_node(cout, node->assign.expr, scope, bc);

    switch (lval_type) {
    case AST_VAR_REF:
      if (node->assign.var->var_ref.is_global) {
        EMIT(bc, OP_SET_GLOBAL);
        sym = SEAL_VALUE_STRING_STATIC(node->assign.var->var_ref.name);
        PUSH_CONST(cout, sym);
        SET_16BITS_INDEX(bc, CONST_IDX(cout));
      } else {
        EMIT(bc, OP_SET_LOCAL);
        name = node->assign.var->var_ref.name;
        e = hashmap_search(scope, name);
        if (e == NULL)
          __compiler_error("maximum number of locals is %zu", scope->cap);

        if (e->key == NULL)
          hashmap_insert_e(scope, e, name, SEAL_VALUE_INT(scope->filled));

        EMIT(bc, e->val.as._int); /* push slot index of local table */
      }
      break;
    default:
      __compiler_error("assigning to %s is not implemented yet", hast_type_name(lval_type));
      break;
    }
  } else {
    seal_word sym_idx;
    int aug_type = node->assign.op_type;
    switch (lval_type) {
    case AST_VAR_REF:
      if (node->assign.var->var_ref.is_global) {
        EMIT(bc, OP_GET_GLOBAL);
        sym = SEAL_VALUE_STRING_STATIC(node->assign.var->var_ref.name);
        PUSH_CONST(cout, sym);
        sym_idx = CONST_IDX(cout);
        SET_16BITS_INDEX(bc, sym_idx);
        compile_node(cout, node->assign.expr, scope, bc);
        EMIT(bc, AUG_ASSIGN_OP_TYPE(aug_type));
        EMIT(bc, OP_SET_GLOBAL);
        SET_16BITS_INDEX(bc, sym_idx);
      } else {
        EMIT(bc, OP_GET_LOCAL);
        name = node->assign.var->var_ref.name;
        e = hashmap_search(scope, name);
        if (e == NULL || e->key == NULL)
          __compiler_error("\'%s\' is not defined", name);

        EMIT(bc, e->val.as._int); /* push slot index of local table */
        compile_node(cout, node->assign.expr, scope, bc);
        EMIT(bc, AUG_ASSIGN_OP_TYPE(aug_type));
        EMIT(bc, OP_SET_LOCAL);
        EMIT(bc, e->val.as._int);
      }
      break;
    default:
      __compiler_error("assigning to %s is not implemented yet", hast_type_name(lval_type));
      break;
    }
  }
}
static void compile_var_ref(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  if (!node->var_ref.is_global) {
    struct h_entry* e = hashmap_search(scope, node->var_ref.name);
    if (e == NULL || e->key == NULL)
      goto global;

    EMIT(bc, OP_GET_LOCAL);
    EMIT(bc, e->val.as._int);
  } else {
global:
    EMIT(bc, OP_GET_GLOBAL);
    PUSH_CONST(cout, SEAL_VALUE_STRING_STATIC(node->var_ref.name));
    SET_16BITS_INDEX(bc, CONST_IDX(cout));
  }
}
static void compile_func_def(cout_t* cout, ast_t* node, hashmap_t* scope)
{
  struct bytechunk bc;
  bc.cap = START_BYTECODE_CAP;
  bc.size = 0;
  bc.bytecodes = SEAL_CALLOC(START_BYTECODE_CAP, sizeof(seal_byte));

  hashmap_t loc_scope;
  struct h_entry entries[LOCAL_MAX];
  hashmap_init_static(&loc_scope, entries, LOCAL_MAX);
  for (int i = 0; i < node->func_def.param_size; i++) {
    hashmap_insert(&loc_scope, node->func_def.param_names[i], SEAL_VALUE_INT(i));
  }

  svalue_t func_obj = {
    .type = SEAL_FUNC,
    .as.func = {
      .type = FUNC_USERDEF,
      .is_vararg = false,
      .name = node->func_def.name,
      .as.userdef = {
        .argc = node->func_def.param_size,
        //.bytecode = bc.bytecodes
      }
    }
  };

  compile_node(cout, node->func_def.comp, &loc_scope, &bc);
  func_obj.as.func.as.userdef.bytecode = bc.bytecodes;

  func_obj.as.func.as.userdef.local_size = loc_scope.filled; /* assign size of locals */
  
  EMIT(&bc, OP_PUSH_NULL);
  EMIT(&bc, OP_HALT);

  EMIT(&cout->bc, OP_PUSH_CONST); /* push function object to constant pool */
  PUSH_CONST(cout, func_obj);
  SET_16BITS_INDEX(&cout->bc, CONST_IDX(cout));

  EMIT(&cout->bc, OP_SET_GLOBAL); /* set function object to global */
  PUSH_CONST(cout, SEAL_VALUE_STRING_STATIC(node->func_def.name));
  SET_16BITS_INDEX((&cout->bc), CONST_IDX(cout));
  EMIT(&cout->bc, OP_POP);
}
static void compile_return(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  compile_node(cout, node->_return.expr, scope, bc);
  EMIT(bc, OP_HALT);
}
static void compile_list(cout_t* cout, ast_t* node, hashmap_t* scope, struct bytechunk* bc)
{
  if (node->list.mem_size > 255)
    __compiler_error("maximum number of elements in a list initializer is 255");
  for (int i = 0; i < node->list.mem_size; i++)
    compile_node(cout, node->list.mems[i], scope, bc);

  EMIT(bc, OP_GEN_LIST);
  EMIT(bc, node->list.mem_size);
}
