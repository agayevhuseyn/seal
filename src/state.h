#ifndef SEAL_STATE_H
#define SEAL_STATE_H

#include "seal.h"

#include "io.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "scope.h"

typedef struct visitor visitor_t;

typedef struct state {
  const char* file_path;
  lexer_t lexer;
  parser_t parser;
  ast_t* root;
  visitor_t* visitor; // it is pointer because of forward declaration
  scope_t g_scope;
} state_t;

void init_state(state_t* state, const char* file_path);

ast_t* state_call_func();

#endif
