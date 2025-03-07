#include "state.h"
#include "visitor.h"

static inline ast_t* state_error(state_t* state, ast_t* node, const char* err)
{
  fprintf(stderr, "seal file: \'%s\', at line: %d\nerror: %s\n", state->file_path, node->line, err);
  exit(1);
  return ast_null();
}

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

ast_t* state_call_func(state_t* state, ast_t* func_def, ast_t** args, size_t arg_size)
{
  scope_t local_scope; // allocate local scope on stack
  init_scope(&local_scope, &state->g_scope); // initialize local scope

  if (arg_size != func_def->func_def.param_size) { // arguments and parameters size must match
    char err[ERR_LEN];
    sprintf(err,
            "function \'%s\' requires %zu argument%s, got %zu",
            func_def->func_def.name,
            func_def->func_def.param_size,
            func_def->func_def.param_size == 1 ? "" : "s",
            arg_size);
    return state_error(state, func_def, err);
  }

  for (int i = 0; i < arg_size; i++) { // push variables to local scope
    ast_t* arg_var = create_var_ast(func_def->func_def.param_names[i],
                                    args[i],
                                    false,
                                    func_def->line);
    scope_push_var(&local_scope, arg_var);
  }
  
  ast_t* returned_val = visitor_visit(state->visitor, &local_scope, func_def->func_def.comp); // should be visited
  // free scope
  gc_free_scope(&local_scope);
  gc_flush(&state->visitor->gc);

  gc_flush_ret(&state->visitor->gc);
  /*gc_print_ret(&visitor->gc);*/
  /*gc_print(&visitor->gc);*/
  return visitor_visit(state->visitor, NULL, returned_val);
}
