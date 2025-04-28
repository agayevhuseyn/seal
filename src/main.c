#include "ast.h"
#include "seal.h"

#include "lexer.h"
#include "parser.h"
#include "gc.h"
#include "vm.h"

static inline void usage(const char* program_name)
{
  fprintf(stderr, "seal: usage: %s filename.seal\n", program_name);
}

static inline void flags()
{
  fprintf(stderr, "seal: flags: -pt (print tokens), -pa (print AST), -po (print opcodes), -pc (print constant pool)\n");
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  /* FLAGS */
  bool PRINT_TOKS = false;
  bool PRINT_AST  = false;
  bool PRINT_OP   = false;
  bool PRINT_CONST_POOL = false;

  for (int i = 2; i < argc; i++) {
    if (strcmp("-pt", argv[i]) == 0) {
      PRINT_TOKS = true;
    } else if (strcmp("-pa", argv[i]) == 0) {
      PRINT_AST = true;
    } else if (strcmp("-po", argv[i]) == 0) {
      PRINT_OP  = true;
    } else if (strcmp("-pc", argv[i]) == 0) {
      PRINT_CONST_POOL = true;
    } else {
      flags();
      return EXIT_FAILURE;
    }
  }
  const char* file_path = argv[1];
  /* lexing */
  lexer_t lexer;
  init_lexer(&lexer, file_path);
  lexer_get_tokens(&lexer);
  if (PRINT_TOKS)
    print_tokens(lexer.toks, lexer.tok_size);
  /* parsing and generating abstract syntax tree (AST) */
  
  create_const_asts(); // allocate constant ASTs
  parser_t parser;
  init_parser(&parser, &lexer);
  ast_t* root = parser_parse(&parser);
  if (PRINT_AST)
    print_ast(root);

  cout_t cout;
  compile(&cout, root);
  if (PRINT_OP)
    print_op(cout.bytecodes, cout.bytecode_size);
  if (PRINT_CONST_POOL)
    PRINT_CONST_POOL(cout);

  vm_t vm;
  init_vm(&vm, &cout);
  eval_vm(&vm);
  //print_stack(&vm);

  // FREE EVERYTHING AFTER BEING USED
  
  return EXIT_SUCCESS;
}
