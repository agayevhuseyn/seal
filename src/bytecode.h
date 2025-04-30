#ifndef SEAL_BYTECODE_H
#define SEAL_BYTECODE_H

#include "seal.h"

#define OP_HALT   0x00
#define OP_PUSH   0x01
#define OP_POP    0x02
#define OP_ADD    0x03
#define OP_SUB    0x04
#define OP_MUL    0x05
#define OP_DIV    0x06
#define OP_MOD    0x07
#define OP_EQ     0x08
#define OP_NE     0x09
#define OP_GT     0x0A
#define OP_GE     0x0B
#define OP_LT     0x0C
#define OP_LE     0x0D
#define OP_JMP    0x0E
#define OP_JZ     0x0F
#define OP_JNZ    0x10
#define OP_PRINT  0x11

#define PRINT_BYTE(bytecodes, size) for(int i = 0; i < size; i++) { \
    printf("%d ", bytecodes[i]); \
    if (i == size - 1) printf("\n"); \
  }

static inline const char* op_name(int op)
{
  switch (op) {
    case OP_HALT : return "HALT";
    case OP_PUSH : return "PUSH";
    case OP_POP  : return "POP";
    case OP_ADD  : return "ADD";
    case OP_SUB  : return "SUB";
    case OP_MUL  : return "MUL";
    case OP_DIV  : return "DIV";
    case OP_MOD  : return "MOD";
    case OP_EQ   : return "EQ";
    case OP_NE   : return "NE";
    case OP_GT   : return "GT";
    case OP_GE   : return "GE";
    case OP_LT   : return "LT";
    case OP_LE   : return "LE";
    case OP_JMP  : return "JMP";
    case OP_JZ   : return "JZ";
    case OP_JNZ  : return "JNZ";
    case OP_PRINT: return "PRINT";
    default      : return "OP NOT RECOGNIZED";
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
      case OP_JMP:
      case OP_JZ:
      case OP_JNZ:
        printf("%d", bytes[i++]);
        break;
    }
    printf("\n");
  }
}

#endif /* SEAL_BYTECODE_H */
