#include "state.h"
#include "visitor.h"

inline void init_state(state_t* state, const char* file_path)
{
  state->file_path = file_path;
  const char* src = read_file(file_path);
  if (!src) {
    fprintf(stderr, "seal: cannot open \'%s\': No such file or directory\n", file_path);
    exit(EXIT_FAILURE);
  }
  init_lexer(&state->lexer, src);
  lexer_get_tokens(&state->lexer);
  init_parser(&state->parser, &state->lexer);
  state->root = parser_parse(&state->parser);
  state->visitor = SEAL_CALLOC(1, sizeof(visitor_t));
  visitor_visit(state->visitor, &state->g_scope, state->root);
}
