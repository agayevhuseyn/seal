#include <stdio.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "io.h"
#include "visitor.h"

int main(int argc, char** argv)
{
  bool PRINT_TOKENS = false;
  bool PARSE        = true;
  bool PRINT_AST    = false;
  bool VISIT        = true;
  bool PRINT_GSCOPE = false;

  if (argc == 1) {
    printf("Usage: %s file_path -pt --noparse -pa --novisit\n", argv[0]);
    return -1;
  }
  if (!check_if_seal_file(argv[1])) {
    fprintf(stderr, "\"%s\" is not seal file format\n", argv[1]);
    return 1;
  }
  const char* src = read_file(argv[1]);
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-pt") == 0) {
      PRINT_TOKENS = true;
    } else if (strcmp(argv[i], "--noparse") == 0) {
      PARSE = false;
    } else if (strcmp(argv[i], "-pa") == 0) {
      PRINT_AST = true;
    } else if (strcmp(argv[i], "--novisit") == 0) {
      VISIT = false;
    } else if (strcmp(argv[i], "-ps") == 0) {
      PRINT_GSCOPE = true;
    } else {
      fprintf(stderr, "invalid option -- \'%s\'\n", argv[i]);
      return 1;
    }
  }
  lexer_t* lexer = init_lexer(src);
  lexer_collect_tokens(lexer);
  if (PRINT_TOKENS)
    print_tokens(lexer->tokens, lexer->token_size);
  if (!PARSE)
    return 0;
  parser_t* parser = init_parser(lexer);
  ast_t* root = parser_parse(parser);
  if (PRINT_AST)
    print_ast(root);
  if (!VISIT)
    return 0;
  visitor_t* visitor = init_visitor(parser);
  visitor_visit(visitor, visitor->g_scope, root);
  if (PRINT_GSCOPE)
    print_scope(visitor->g_scope);
  
  return 0;
}
