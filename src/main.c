#include "ast.h"
#include "seal.h"

#include "io.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "gc.h"

static inline void usage(const char* program_name)
{
  fprintf(stderr, "seal: usage: %s filename.seal\n", program_name);
}

static inline void flags()
{
  fprintf(stderr, "seal: flags: -pt (print tokens), -pa (print AST(abstact syntax tree))\n");
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

  for (int i = 2; i < argc; i++) {
    if (strcmp("-pt", argv[i]) == 0) {
      PRINT_TOKS = true;
    } else if (strcmp("-pa", argv[i]) == 0) {
      PRINT_AST = true;
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
  create_typeof_asts(); // allocate typeof instances
  parser_t parser;
  init_parser(&parser, &lexer);
  ast_t* root = parser_parse(&parser);
  if (PRINT_AST)
    print_ast(root);

  visitor_t visitor;
  gc_t gc = {NULL, NULL};
  init_visitor(&visitor, &gc, file_path);
  scope_t global_scope;
  init_scope(&global_scope, NULL);
  visitor_visit(&visitor, &global_scope, root);

  // FREE EVERYTHING AFTER BEING USED
  
  return EXIT_SUCCESS;
}
