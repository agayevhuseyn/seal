#ifndef SEAL_BYTECODE_H
#define SEAL_BYTECODE_H

#include "seal.h"

/* essential */
#define OP_HALT       0x00
#define OP_PUSH       0x01
#define OP_POP        0x02
#define OP_DUP        0x03
#define OP_JUMP       0x04
#define OP_JTRUE      0x05
#define OP_JFALSE     0x06
/* built-in functions */
#define OP_PRINT      0x07
#define OP_SCAN       0x08
/* variable */
#define OP_GET_GLOBAL 0x09
#define OP_SET_GLOBAL 0x0a
#define OP_GET_LOCAL  0x0b
#define OP_SET_LOCAL  0x0c
/* arithmetic */
#define OP_ADD        0x0d
#define OP_SUB        0x0e
#define OP_MUL        0x0f
#define OP_DIV        0x10
#define OP_MOD        0x11
/* bitwise binary */
#define OP_BAND       0x12
#define OP_BOR        0x13
#define OP_XOR        0x14
#define OP_SHL        0x15
#define OP_SHR        0x16
/* comparison */
#define OP_GT         0x19
#define OP_GE         0x1a
#define OP_LT         0x1b
#define OP_LE         0x1c
/* equality */
#define OP_EQ         0x17
#define OP_NE         0x18
/* logical binary */
#define OP_AND        0x1d
#define OP_OR         0x1e
/* unary */
#define OP_NOT        0x1f
#define OP_NEG        0x20
#define OP_TYPOF      0x21
#define OP_BNOT       0x22


#define PRINT_BYTE(bytecodes, size) for(int i = 0; i < size; i++) { \
    printf("%d ", bytecodes[i]); \
    if (i == size - 1) printf("\n"); \
  }

static inline const char* op_name(int op)
{
  switch (op) {
  /* essential */
  case OP_HALT      :  return "OP_HALT";
  case OP_PUSH      :  return "OP_PUSH";
  case OP_POP       :  return "OP_POP";
  case OP_DUP       :  return "OP_DUP";
  case OP_JUMP      :  return "OP_JUMP";
  case OP_JTRUE     :  return "OP_JTRUE";
  case OP_JFALSE    :  return "OP_JFALSE";
  /* builtin-in functions */
  case OP_PRINT     :  return "OP_PRINT";
  case OP_SCAN      :  return "OP_SCAN";
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
  case OP_BAND      :  return "OP_BAND";
  case OP_BOR       :  return "OP_BOR";
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
  case OP_AND       :  return "OP_AND";
  case OP_OR        :  return "OP_OR";
  case OP_NOT       :  return "OP_NOT";
  /* typeof */
  case OP_TYPOF     :  return "OP_TYPOF";
  default           :  return "OP NOT RECOGNIZED";
  }
}

static inline void print_op(uint8_t* bytes, size_t byte_size, uint16_t* labels, size_t label_size)
{
  for (int i = 0; i < byte_size;) {
    for (int j = 0; j < label_size; j++) {
      if (i == labels[j]) {
        printf("LABEL%d: %d\n", j, labels[j]);
      }
    }
    uint8_t op = bytes[i++];
    printf("%s ", op_name(op)); 
    switch (op) { /* check if opcode requires byte(s) */
    case OP_PUSH: {
      uint8_t left  = bytes[i++];
      uint8_t right = bytes[i++];
      uint16_t idx  = (left << 8) | right;
      switch (idx) {
        case 0: printf("NULL");  break;
        case 1: printf("TRUE");  break;
        case 2: printf("FALSE"); break;
        default:
          printf("%d", idx);
          break;
      }
    }
    break;
    case OP_JUMP: case OP_JFALSE: case OP_JTRUE: case OP_GET_GLOBAL: case OP_SET_GLOBAL: {
      uint8_t left  = bytes[i++];
      uint8_t right = bytes[i++];
      uint16_t idx  = (left << 8) | right;
      printf("%d", idx);
    }
    break;
    case OP_PRINT:
      printf("%d", bytes[i++]);   
      break;
    }
    printf("\n");
  }
}

#endif /* SEAL_BYTECODE_H */
