#include "ast.h"
#include "seal.h"

#include "lexer.h"
#include "parser.h"
#include "vm.h"

#define USAGE(prog_name) (fprintf(stderr, "seal: usage: %s filename.seal\n", prog_name))
#define PRINT_FLAGS() (fprintf(stderr, "seal: flags: -pt (print tokens), -pa (print AST), -po (print opcodes), -pb (print bytes), -pc (print constant pool)\n"))

int main(int argc, char** argv)
{
  if (argc < 2) {
    USAGE(argv[0]);
    return EXIT_FAILURE;
  }
  /* FLAGS */
  bool PRINT_TOKS = false;
  bool PRINT_AST  = false;
  bool PRINT_OP   = false;
  bool PRINT_BYTE = false;
  bool PRINT_CONST_POOL = false;
  bool PRINT_STACK = false;

  for (int i = 2; i < argc; i++) {
    if (strcmp("-pt", argv[i]) == 0) {
      PRINT_TOKS = true;
    } else if (strcmp("-pa", argv[i]) == 0) {
      PRINT_AST = true;
    } else if (strcmp("-po", argv[i]) == 0) {
      PRINT_OP  = true;
    } else if (strcmp("-pb", argv[i]) == 0) {
      PRINT_BYTE = true;
    } else if (strcmp("-pc", argv[i]) == 0) {
      PRINT_CONST_POOL = true;
    } else if (strcmp("-ps", argv[i]) == 0) {
      PRINT_STACK = true;
    }else {
      PRINT_FLAGS();
      return EXIT_FAILURE;
    }
  }
  const char* file_path = argv[1];
  /* lexing */
  ast_t* root;
  lexer_t lexer;
  init_lexer(&lexer, file_path);
  lexer_get_tokens(&lexer);
  if (PRINT_TOKS)
    print_tokens(lexer.toks, lexer.tok_size);
  /* parsing and generating abstract syntax tree (AST) */
  
  create_const_asts(); /* allocate constant ASTs */
  parser_t parser;
  init_parser(&parser, &lexer);
  root = parser_parse(&parser);
  if (PRINT_AST)
    print_ast(root);

  cout_t cout;
  compile(&cout, root);
  if (PRINT_OP)
    print_op(cout.bc.bytecodes, cout.bc.size, cout.labels, cout.label_pool_size);
  if (PRINT_BYTE)
    PRINT_BYTE(cout.bc.bytecodes, cout.bc.size)
  if (PRINT_CONST_POOL)
    PRINT_CONST_POOL(cout);

  init_mod_cache(); /* initalize modules cache before running main file */

  vm_t vm;
  init_vm(&vm, &cout);
  svalue_t locals[cout.main_scope_local_size];
  struct local_frame main_frame = {
    .locals = locals,
    .bytecodes = vm.bytecodes,
    .ip = vm.bytecodes,
    .label_pool = cout.labels,
    .const_pool = cout.const_pool
  };
  eval_vm(&vm, &main_frame);
  if (PRINT_STACK)
    print_stack(&vm);

  //svalue_t func = hashmap_search(&vm.globals, "add")->val;
  //print_op(func.as.func.as.userdef.bytecode, 36, cout.labels, 0);

  /* FREE EVERYTHING AFTER BEING USED */
  
  return EXIT_SUCCESS;
}
