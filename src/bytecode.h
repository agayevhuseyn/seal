#ifndef SEAL_BYTECODE_H
#define SEAL_BYTECODE_H

#include "sealconf.h"

/* essential */
#define OP_HALT       0x00
#define OP_PUSH_INT   0x02
#define OP_PUSH_NULL  0x03
#define OP_PUSH_TRUE  0x04
#define OP_PUSH_FALSE 0x05
#define OP_PUSH_CONST 0x06
#define OP_POP        0x07
#define OP_DUP        0x08
#define OP_SWAP       0x09
#define OP_JUMP       0x0a
#define OP_JTRUE      0x0b
#define OP_JFALSE     0x0c
#define OP_CALL       0x0d
/* variable */
#define OP_GET_GLOBAL 0x0e
#define OP_SET_GLOBAL 0x0f
#define OP_GET_LOCAL  0x10
#define OP_SET_LOCAL  0x11
/* arithmetic */
#define OP_ADD        0x12
#define OP_SUB        0x13
#define OP_MUL        0x14
#define OP_DIV        0x15
#define OP_MOD        0x16
/* bitwise binary */
#define OP_AND        0x17
#define OP_OR         0x18
#define OP_XOR        0x19
#define OP_SHL        0x1a
#define OP_SHR        0x1b
/* comparison */
#define OP_GT         0x1c
#define OP_GE         0x1d
#define OP_LT         0x1e
#define OP_LE         0x1f
/* equality */
#define OP_EQ         0x20
#define OP_NE         0x21
/* unary */
#define OP_NOT        0x22
#define OP_NEG        0x23
#define OP_TYPOF      0x24
#define OP_BNOT       0x25
/* list */
#define OP_GEN_LIST   0x26
/* iterable */
#define OP_GET_FIELD  0x27
#define OP_SET_FIELD  0x28
/* membership */
#define OP_IN         0x29
/* map */
#define OP_GEN_MAP    0x2a
/* include */
#define OP_INCLUDE    0x2b

#define PRINT_BYTE(bytecodes, size) for(int i = 0; i < size; i++) { \
    printf("%d ", bytecodes[i]); \
    if (i == size - 1) printf("\n"); \
  }

static inline const char* op_name(int op)
{
  switch (op) {
  /* essential */
  case OP_HALT      :  return "OP_HALT";
  case OP_PUSH_CONST:  return "OP_PUSH_CONST";
  case OP_PUSH_INT  :  return "OP_PUSH_INT";
  case OP_PUSH_NULL :  return "OP_PUSH_NULL";
  case OP_PUSH_TRUE :  return "OP_PUSH_TRUE";
  case OP_PUSH_FALSE:  return "OP_PUSH_FALSE";
  case OP_POP       :  return "OP_POP";
  case OP_DUP       :  return "OP_DUP";
  case OP_SWAP      :  return "OP_SWAP";
  case OP_JUMP      :  return "OP_JUMP";
  case OP_JTRUE     :  return "OP_JTRUE";
  case OP_JFALSE    :  return "OP_JFALSE";
  case OP_CALL      :  return "OP_CALL";
  /* variable */
  case OP_GET_GLOBAL:  return "OP_GET_GLOBAL";
  case OP_SET_GLOBAL:  return "OP_SET_GLOBAL";
  case OP_GET_LOCAL :  return "OP_GET_LOCAL";
  case OP_SET_LOCAL :  return "OP_SET_LOCAL";
  /* arithmetic */
  case OP_ADD       :  return "OP_ADD";
  case OP_SUB       :  return "OP_SUB";
  case OP_MUL       :  return "OP_MUL";
  case OP_DIV       :  return "OP_DIV";
  case OP_MOD       :  return "OP_MOD";
  /* bitwise */
  case OP_AND       :  return "OP_AND";
  case OP_OR        :  return "OP_OR";
  case OP_XOR       :  return "OP_XOR";
  case OP_SHL       :  return "OP_SHL";
  case OP_SHR       :  return "OP_SHR";
  case OP_BNOT      :  return "OP_BNOT";
  /* comparison */
  case OP_EQ        :  return "OP_EQ";
  case OP_NE        :  return "OP_NE";
  case OP_GT        :  return "OP_GT";
  case OP_GE        :  return "OP_GE";
  case OP_LT        :  return "OP_LT";
  case OP_LE        :  return "OP_LE";
  /* logical */
  case OP_NOT       :  return "OP_NOT";
  /* typeof */
  case OP_TYPOF     :  return "OP_TYPOF";
  case OP_GEN_LIST  :  return "OP_GEN_LIST";
  case OP_GET_FIELD :  return "OP_GET_FIELD";
  case OP_GEN_MAP   :  return "OP_GEN_MAP";
  case OP_INCLUDE   :  return "OP_INCLUDE";
  default           :  return "OP NOT RECOGNIZED";
  }
}

static inline void print_op(seal_byte* bytes, size_t byte_size, seal_word* labels, size_t label_size)
{
  for (int i = 0; i < byte_size;) {
    for (int j = 0; j < label_size; j++) {
      if (i == labels[j]) {
        printf("LABEL%d: %d\n", j, labels[j]);
      }
    }
    seal_byte op = bytes[i++];
    printf("%s ", op_name(op)); 
    switch (op) { /* check if opcode requires byte(s) */
    case OP_PUSH_CONST: case OP_PUSH_INT: {
      seal_byte left  = bytes[i++];
      seal_byte right = bytes[i++];
      seal_word idx  = (left << 8) | right;
      printf("%d", idx);
    }
    break;
    case OP_JUMP: case OP_JFALSE: case OP_JTRUE: case OP_GET_GLOBAL: case OP_SET_GLOBAL: {
      seal_byte left  = bytes[i++];
      seal_byte right = bytes[i++];
      seal_word idx  = (left << 8) | right;
      printf("%d", idx);
      break;
    }
    case OP_GET_LOCAL: case OP_SET_LOCAL: case OP_GEN_LIST: case OP_GEN_MAP: {
      seal_byte slot = bytes[i++];
      printf("%d", slot);
      break;
    }
    case OP_CALL:
      printf("%d", bytes[i++]);   
      break;
    }
    printf("\n");
  }
}

#endif /* SEAL_BYTECODE_H */
