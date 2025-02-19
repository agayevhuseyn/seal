#include "parser.h"
#include "ast.h"

#define parser_is_end(parser) ( \
  parser->i >= parser->tok_size)

#define parser_eof_index(parser) ( \
  parser->tok_size - 1)

#define parser_eof(parser) ( \
  parser->toks[parser->tok_size - 1])

#define parser_peek(parser) ( \
  parser->toks[parser->i])

#define parser_match(parser, t) ( \
  parser_peek(parser)->type == t)

#define parser_line(parser) ( \
  parser_peek(parser)->line)

#define parser_peek_offset(parser, offset) ( \
  (parser->i + (offset) >= 0 && parser->i + (offset) < parser->tok_size)\
  ? parser->toks[parser->i + (offset)]\
  : parser_eof(parser))

#define is_lvalue(node) ( \
  node->type == AST_VAR_REF || \
  node->type == AST_SUBSCRIPT || \
  node->type == AST_MEMACC)
  
static inline ast_t* parser_error(parser_t* parser, const char* err)
{
  fprintf(stderr, "seal: line %d\nsyntax error: %s\n", parser_line(parser), err);
  exit(1);
  return ast_null();
}

static inline bool is_reserved_name(const char* name)
{
  for (int i = 0; i < RESERVED_NAMES_SIZE; i++) {
    if (strcmp(name, RESERVED_NAMES[i]) == 0) return true;
  }
  return false;
}

static inline void kill_if_reserved_name(parser_t* parser, const char* name)
{
  if (is_reserved_name(name)) {
    char err[ERR_LEN];
    sprintf(err, "reserved name: \'%s\' used", name);
    parser_error(parser, err);
  }
}

static inline void kill_if_duplicated_name(parser_t* parser,
                                           const char* name,
                                           const char** names,
                                           size_t name_size)
{
  for (int i = 0; i < name_size; i++) {
    if (strcmp(name, names[i]) == 0) {
      char err[ERR_LEN];
      sprintf(err, "duplicate parameter: \'%s\' used", name);
      parser_error(parser, err);
    }
  }
}

static inline token_t* parser_advance(parser_t* parser)
{
  return parser->toks[parser_is_end(parser) ? parser_eof_index(parser) : parser->i++];
}

static inline token_t* parser_eat(parser_t* parser, int type)
{
  if (parser_match(parser, type)) return parser_advance(parser);
  if (type == TOK_DEDENT && parser_match(parser, TOK_EOF)) return parser_peek(parser);

  int prev_tok_type = parser_peek_offset(parser, -1)->type;
  bool followed_word = prev_tok_type != TOK_NEWL &&
                       prev_tok_type != TOK_INDENT &&
                       prev_tok_type != TOK_DEDENT;

  char err[ERR_LEN];
  sprintf(err,
          "expected \'%s\' instead of \'%s\'",
          htoken_type_name(type),
          parser_peek(parser)->val);
  if (followed_word) {
    char after[ERR_LEN];
    sprintf(after, " after \'%s\'", parser_peek_offset(parser, -1)->val);
    strcat(err, after);
  }

  parser_error(parser, err);
  return NULL;
}

/* main function */
inline ast_t* parser_parse(parser_t* parser)
{
  create_const_asts(); // allocate constant ASTs

  return parser_parse_statements(parser, false, false, false, false);
}

/* parsing statement */
static ast_t* parser_parse_statements(parser_t* parser,
                                      bool is_func,
                                      bool is_ifelse,
                                      bool is_loop,
                                      bool is_inline)
{
  ast_t* ast = static_create_ast(AST_COMP, parser_line(parser));

  ast->comp.stmt_size = 1;
  ast->comp.stmts = SEAL_CALLOC(1, sizeof(ast_t*));
  ast->comp.stmts[0] = parser_parse_statement(parser, is_func, is_ifelse, is_loop, is_inline);

  while (!parser_is_end(parser) && parser_match(parser, TOK_NEWL)) {
    parser_eat(parser, TOK_NEWL);

    ast->comp.stmt_size++;
    ast->comp.stmts = SEAL_REALLOC(ast->comp.stmts, ast->comp.stmt_size * sizeof(ast_t*));
    ast->comp.stmts[ast->comp.stmt_size - 1] = parser_parse_statement(parser, is_func, is_ifelse, is_loop, is_inline);
  }

  bool is_global_scope = !is_func && !is_ifelse && !is_loop;
  if (is_global_scope && !parser_match(parser, TOK_EOF)) {
    char err[ERR_LEN];
    sprintf(err, "unexpected \'%s\' after \'%s\'", parser_peek(parser)->val, parser_peek_offset(parser, -1)->val);
    return parser_error(parser, err);
  } else if (!is_global_scope && !parser_match(parser, TOK_DEDENT)) {
    char err[ERR_LEN];
    sprintf(err, "unexpected \'%s\' after \'%s\'", parser_peek(parser)->val, parser_peek_offset(parser, -1)->val);
    return parser_error(parser, err);
  }

  return ast;
}

static ast_t* parser_parse_statement(parser_t* parser,
                                     bool is_func,
                                     bool is_ifelse,
                                     bool is_loop,
                                     bool is_inline)
{
  bool is_global_scope = !is_func && !is_ifelse && !is_loop;
  switch (parser_peek(parser)->type) {
    case TOK_VAR:
    case TOK_CONST:
      if (is_inline) goto error;
      return parser_parse_var_def(parser);
    case TOK_IF:
      if (is_inline) goto error;
      return parser_parse_if(parser, true, is_func, is_loop);
    case TOK_DO:
      if (is_inline) goto error;
      return parser_parse_dowhile(parser, is_func);
    case TOK_WHILE:
      if (is_inline) goto error;
      return parser_parse_while(parser, is_func);
    case TOK_FOR:
      if (is_inline) goto error;
      return parser_parse_for(parser, is_func);
    case TOK_DEFINE:
      if (!is_global_scope) goto error;
      return parser_parse_func_def(parser);
    case TOK_STRUCT:
      if (!is_global_scope) goto error;
      return parser_parse_struct_def(parser);
    case TOK_SKIP:
      if (!is_loop) goto error;
      return parser_parse_skip(parser);
    case TOK_STOP:
      if (!is_loop) goto error;
      return parser_parse_stop(parser);
    case TOK_RETURN:
      if (!is_func) goto error;
      return parser_parse_return(parser);
    case TOK_INCLUDE:
      if (!is_global_scope) goto error;
      return parser_parse_include(parser);
    case TOK_DEDENT:
      if (is_global_scope) goto error;
      return ast_null();
    case TOK_EOF:
      if (!is_global_scope) goto error;
      return ast_null();
    default:
      return parser_parse_expr(parser);
  }
  error: {
    char err[ERR_LEN];
    sprintf(err, "unexpected \'%s\'", parser_peek(parser)->val);
    return parser_error(parser, err);
  }
}

/* parse datas */
static inline ast_t* parser_parse_int(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_INT, parser_line(parser));
  ast->integer.val = atoi(parser_eat(parser, TOK_INT)->val);
  return ast;
}

static inline ast_t* parser_parse_float(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_FLOAT, parser_line(parser));
  ast->floating.val = atof(parser_eat(parser, TOK_FLOAT)->val);
  return ast;
}

static inline ast_t* parser_parse_string(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_STRING, parser_line(parser));
  ast->string.val = (char*)parser_eat(parser, TOK_STRING)->val;
  return ast;
}

static ast_t* parser_parse_list(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_LIST_LIT, parser_line(parser));
  ast->list_lit.mem_size = 0;
  ast->list_lit.mems = NULL;

  parser_eat(parser, TOK_LBRACK);
  if (!parser_match(parser, TOK_RBRACK)) {
    ast->list_lit.mem_size = 1;
    ast->list_lit.mems = SEAL_CALLOC(1, sizeof(ast_t*));
    ast->list_lit.mems[0] = parser_parse_expr(parser);
  }
  while (parser_match(parser, TOK_COMMA)) {
    parser_eat(parser, TOK_COMMA);

    ast->list_lit.mem_size++;
    ast->list_lit.mems = SEAL_REALLOC(ast->list_lit.mems, ast->list_lit.mem_size * sizeof(ast_t*));
    ast->list_lit.mems[ast->list_lit.mem_size - 1] = parser_parse_expr(parser);
  }
  parser_eat(parser, TOK_RBRACK);

  return ast;
}

static ast_t* parser_parse_object(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_OBJECT_LIT, parser_line(parser));
  ast->object_lit.field_size = 0;
  ast->object_lit.field_names = NULL;
  ast->object_lit.field_vals = NULL;

  parser_eat(parser, TOK_LBRACE);
  if (!parser_match(parser, TOK_RBRACE)) {
    ast->object_lit.field_size = 1;
    // field names
    ast->object_lit.field_names = SEAL_CALLOC(1, sizeof(char*));
    ast->object_lit.field_names[0] = parser_eat(parser, TOK_ID)->val;

    parser_eat(parser, TOK_ASSIGN); // require '='
    // field vals
    ast->object_lit.field_vals = SEAL_CALLOC(1, sizeof(ast_t*));
    ast->object_lit.field_vals[0] = parser_parse_expr(parser);
  }
  while (parser_match(parser, TOK_COMMA)) {
    parser_eat(parser, TOK_COMMA);

    // no duplicate allowed
    const char* field_name = parser_eat(parser, TOK_ID)->val;
    kill_if_duplicated_name(parser, field_name, ast->object_lit.field_names, ast->object_lit.field_size);

    ast->object_lit.field_size++;
    // field names
    ast->object_lit.field_names = SEAL_REALLOC(ast->object_lit.field_names, ast->object_lit.field_size * sizeof(char*));
    ast->object_lit.field_names[ast->object_lit.field_size - 1] = field_name;

    parser_eat(parser, TOK_ASSIGN); // require '='
    // field vals
    ast->object_lit.field_vals = SEAL_REALLOC(ast->object_lit.field_vals, ast->object_lit.field_size * sizeof(ast_t*));
    ast->object_lit.field_vals[ast->object_lit.field_size - 1] = parser_parse_expr(parser);
  }
  parser_eat(parser, TOK_RBRACE);

  return ast;
}

static inline ast_t* parser_parse_id(parser_t* parser)
{
  if (parser_peek_offset(parser, 1)->type == TOK_LPAREN)
    return parser_parse_func_call(parser);
  return parser_parse_var_ref(parser);
}
static inline ast_t* parser_parse_var_ref(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_VAR_REF, parser_line(parser));
  ast->var_ref.name = parser_eat(parser, TOK_ID)->val;
  return ast;
}
static ast_t* parser_parse_func_call(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_FUNC_CALL, parser_line(parser));

  ast->func_call.name = parser_eat(parser, TOK_ID)->val;
  ast->func_call.arg_size = 0;
  parser_eat(parser, TOK_LPAREN);

  if (!parser_match(parser, TOK_RPAREN)) {
    ast->func_call.arg_size = 1;
    ast->func_call.args = SEAL_CALLOC(1, sizeof(ast_t*));
    ast->func_call.args[0] = parser_parse_expr(parser);
  }

  while (!parser_match(parser, TOK_RPAREN)) {
    parser_eat(parser, TOK_COMMA);

    ast->func_call.arg_size++;
    ast->func_call.args = SEAL_REALLOC(ast->func_call.args, ast->func_call.arg_size * sizeof(ast_t*));
    ast->func_call.args[ast->func_call.arg_size - 1] = parser_parse_expr(parser);
  }

  parser_eat(parser, TOK_RPAREN);

  return ast;
}

/* parse blocks */
static ast_t* parser_parse_var_def(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_VAR_DEF, parser_line(parser));
  bool is_const = ast->var_def.is_const = parser_match(parser, TOK_CONST); // constant when declared with 'const'
  parser_advance(parser); // either 'var' or 'const'

  // at lemain 1 definition
  ast->var_def.size = 1;
  ast->var_def.names = SEAL_CALLOC(1, sizeof(char*));
  ast->var_def.exprs = SEAL_CALLOC(1, sizeof(ast_t*));

  if (parser_match(parser, TOK_NEWL)) { // declare line-by-line
    parser_advance(parser); // newline
    parser_eat(parser, TOK_INDENT); // require indentation

    ast->var_def.names[0] = parser_eat(parser, TOK_ID)->val;
    kill_if_reserved_name(parser, ast->var_def.names[0]); // kill if reserved name
    if (is_const || parser_match(parser, TOK_ASSIGN)) { // assign value
      parser_eat(parser, TOK_ASSIGN); // '='
      ast->var_def.exprs[0] = parser_parse_expr(parser);
    } else
      ast->var_def.exprs[0] = ast_null();

  get_def_line:
    parser_eat(parser, TOK_NEWL); // newl

    if (parser_match(parser, TOK_DEDENT)) {
      parser_advance(parser);
      return ast;
    }

    ast->var_def.size++;
    ast->var_def.names = SEAL_REALLOC(ast->var_def.names, ast->var_def.size * sizeof(char*));
    ast->var_def.exprs = SEAL_REALLOC(ast->var_def.exprs, ast->var_def.size * sizeof(ast_t*));

    ast->var_def.names[ast->var_def.size - 1] = parser_eat(parser, TOK_ID)->val;
    kill_if_reserved_name(parser, ast->var_def.names[ast->var_def.size - 1]); // kill if reserved name
    if (is_const || parser_match(parser, TOK_ASSIGN)) { // assign value
      parser_eat(parser, TOK_ASSIGN); // '='
      ast->var_def.exprs[ast->var_def.size - 1] = parser_parse_expr(parser);
    } else
      ast->var_def.exprs[ast->var_def.size - 1] = ast_null();

    goto get_def_line; // repeat
  } else { // declare comma-by-comma
    ast->var_def.names[0] = parser_eat(parser, TOK_ID)->val;
    kill_if_reserved_name(parser, ast->var_def.names[0]); // kill if reserved name
    if (is_const || parser_match(parser, TOK_ASSIGN)) { // assign value
      parser_eat(parser, TOK_ASSIGN); // '='
      ast->var_def.exprs[0] = parser_parse_expr(parser);
    } else
      ast->var_def.exprs[0] = ast_null();

  get_def_comma:
    if (parser_match(parser, TOK_COMMA)) {
      parser_advance(parser); // ','

      ast->var_def.size++;
      ast->var_def.names = SEAL_REALLOC(ast->var_def.names, ast->var_def.size * sizeof(char*));
      ast->var_def.exprs = SEAL_REALLOC(ast->var_def.exprs, ast->var_def.size * sizeof(ast_t*));

      ast->var_def.names[ast->var_def.size - 1] = parser_eat(parser, TOK_ID)->val;
      kill_if_reserved_name(parser, ast->var_def.names[ast->var_def.size - 1]); // kill if reserved name
      if (is_const || parser_match(parser, TOK_ASSIGN)) { // assign value
        parser_eat(parser, TOK_ASSIGN); // '='
        ast->var_def.exprs[ast->var_def.size - 1] = parser_parse_expr(parser);
      } else
        ast->var_def.exprs[ast->var_def.size - 1] = ast_null();

      goto get_def_comma; // repeat
    }
  }

  return ast;
}

static ast_t* parser_parse_if(parser_t* parser, bool can_be_ternary, bool is_func, bool is_loop)
{
  if (can_be_ternary) {
    for (token_t** toks = parser->toks; *toks != NULL; toks++) {
      if ((*toks)->type == TOK_NEWL) break;
      if ((*toks)->type == TOK_ELSE) {
        return parser_parse_ternary(parser);
      }
    }
  }

  parser_eat(parser, TOK_IF);

  ast_t* ast = static_create_ast(AST_IF, parser_line(parser));
  ast->_if.has_else = false;

  ast->_if.cond = parser_parse_expr(parser);

  if (parser_match(parser, TOK_THEN)) { // inline if
    parser_advance(parser); // 'then'
    ast->_if.comp = parser_parse_statement(parser, is_func, true, is_loop, true);
  } else {
    parser_eat(parser, TOK_NEWL);
    parser_eat(parser, TOK_INDENT);

    ast->_if.comp = parser_parse_statements(parser, is_func, true, is_loop, false);

    parser_eat(parser, TOK_DEDENT);
  }
  
  if (parser_peek_offset(parser, 1)->type == TOK_ELSE) {
    parser_eat(parser, TOK_NEWL);
    ast->_if.has_else = true;
    if (parser_peek_offset(parser, 1)->type == TOK_IF) {
      parser_advance(parser); // 'else'
      ast->_if._else = parser_parse_if(parser, false, is_func, is_loop);
    } else {
      ast->_if._else = parser_parse_else(parser, is_func, is_loop);
    }
  }

  return ast;
}

static ast_t* parser_parse_else(parser_t* parser, bool is_func, bool is_loop)
{
  ast_t* ast = static_create_ast(AST_ELSE, parser_line(parser));

  parser_eat(parser, TOK_ELSE);
  if (parser_match(parser, TOK_NEWL)) {
    parser_advance(parser); // newl
    parser_eat(parser, TOK_INDENT);

    ast->_else.comp = parser_parse_statements(parser, is_func, true, is_loop, false);

    parser_eat(parser, TOK_DEDENT);
  } else { // inline
    ast->_else.comp = parser_parse_statement(parser, is_func, true, is_loop, true);
  }

  return ast;
}
static ast_t* parser_parse_dowhile(parser_t* parser, bool is_func)
{
  ast_t* ast = static_create_ast(AST_DOWHILE, parser_line(parser));

  parser_eat(parser, TOK_DO);
  if (parser_match(parser, TOK_NEWL)) {
    parser_advance(parser); // newl
    parser_eat(parser, TOK_INDENT);

    ast->_while.comp = parser_parse_statements(parser, is_func, false, true, false);

    parser_eat(parser, TOK_DEDENT);
    parser_eat(parser, TOK_NEWL);
    parser_eat(parser, TOK_WHILE);
    ast->_while.cond = parser_parse_expr(parser);
  } else {
    ast->_while.comp = parser_parse_statement(parser, is_func, false, true, true);
    parser_eat(parser, TOK_WHILE);
    ast->_while.cond = parser_parse_expr(parser);
  }

  return ast;
}
static ast_t* parser_parse_while(parser_t* parser, bool is_func)
{
  parser_eat(parser, TOK_WHILE);

  ast_t* ast = static_create_ast(AST_WHILE, parser_line(parser));

  ast->_while.cond = parser_parse_expr(parser);

  if (parser_match(parser, TOK_DO)) { // inline while
    parser_advance(parser); // 'do'
    ast->_while.comp = parser_parse_statement(parser, is_func, false, true, true);
  } else {
    parser_eat(parser, TOK_NEWL);
    parser_eat(parser, TOK_INDENT);

    ast->_while.comp = parser_parse_statements(parser, is_func, false, true, false);

    parser_eat(parser, TOK_DEDENT);
  }

  return ast;
}
static ast_t* parser_parse_for(parser_t* parser, bool is_func)
{
  parser_eat(parser, TOK_FOR);

  bool has_to = false;
  bool has_by = false;
  bool is_numerical = false;

  for (token_t** toks = parser->toks; *toks != NULL; toks++) {
    if ((*toks)->type == TOK_NEWL) break;
    if ((*toks)->type == TOK_TO) {
      has_to = true;
      is_numerical = true;
    } else if ((*toks)->type == TOK_BY) {
      has_by = true;
      is_numerical = true;
    }
  }

  ast_t* ast = static_create_ast(AST_FOR, parser_line(parser));
  ast->_for.is_numerical = is_numerical;
  ast->_for.start = NULL;
  ast->_for.end   = NULL;
  ast->_for.step  = NULL;

  kill_if_reserved_name(parser, (ast->_for.it_name = parser_eat(parser, TOK_ID)->val));
  parser_eat(parser, TOK_IN);

  if (is_numerical) {
    ast->_for.start = parser_parse_expr(parser);
    if (has_to) {
      parser_eat(parser, TOK_TO);
      ast->_for.end = parser_parse_expr(parser);
    }
    if (has_by) {
      parser_eat(parser, TOK_BY);
      ast->_for.step = parser_parse_expr(parser);
    }
  } else {
    ast->_for.ited = parser_parse_expr(parser);
  }

  if (parser_match(parser, TOK_DO)) { // inline for
    parser_advance(parser); // 'do'
    ast->_for.comp = parser_parse_statement(parser, is_func, false, true, true);
  } else {
    parser_eat(parser, TOK_NEWL);
    parser_eat(parser, TOK_INDENT);

    ast->_for.comp = parser_parse_statements(parser, is_func, false, true, false);

    parser_eat(parser, TOK_DEDENT);
  }

  return ast;
}
static ast_t* parser_parse_func_def(parser_t* parser)
{
  parser_eat(parser, TOK_DEFINE);

  ast_t* ast = static_create_ast(AST_FUNC_DEF, parser_line(parser));
  ast->func_def.param_size = 0;

  kill_if_reserved_name(parser, ast->func_def.name = parser_eat(parser, TOK_ID)->val); // kill if reserved name

  parser_eat(parser, TOK_LPAREN);

  if (!parser_match(parser, TOK_RPAREN)) {
    ast->func_def.param_size = 1;
    ast->func_def.param_names = SEAL_CALLOC(1, sizeof(char*));
    kill_if_reserved_name(parser, ast->func_def.param_names[0] = parser_eat(parser, TOK_ID)->val);
  }

  while (!parser_match(parser, TOK_RPAREN)) {
    parser_eat(parser, TOK_COMMA); // ',' separator

    const char* param = parser_eat(parser, TOK_ID)->val;
    kill_if_reserved_name(parser, param);
    kill_if_duplicated_name(parser, param, ast->func_def.param_names, ast->func_def.param_size);

    ast->func_def.param_size++;
    ast->func_def.param_names = SEAL_REALLOC(ast->func_def.param_names, ast->func_def.param_size * sizeof(char*));
    ast->func_def.param_names[ast->func_def.param_size - 1] = param;
  }

  parser_eat(parser, TOK_RPAREN);

  parser_eat(parser, TOK_NEWL);
  parser_eat(parser, TOK_INDENT);

  ast->func_def.comp = parser_parse_statements(parser, true, false, false, false);

  parser_eat(parser, TOK_DEDENT);

  return ast;
}
static ast_t* parser_parse_struct_def(parser_t* parser)
{
  parser_eat(parser, TOK_STRUCT);

  ast_t* ast = static_create_ast(AST_STRUCT_DEF, parser_line(parser));
  ast->struct_def.param_size = 0;
  ast->struct_def.field_size = 0;

  kill_if_reserved_name(parser, ast->struct_def.name = parser_eat(parser, TOK_ID)->val); // kill if reserved name

  parser_eat(parser, TOK_LPAREN);

  if (!parser_match(parser, TOK_RPAREN)) {
    ast->struct_def.param_size = 1;
    ast->struct_def.param_names = SEAL_CALLOC(1, sizeof(char*));
    kill_if_reserved_name(parser, ast->struct_def.param_names[0] = parser_eat(parser, TOK_ID)->val);
  }

  while (!parser_match(parser, TOK_RPAREN)) {
    parser_eat(parser, TOK_COMMA); // ',' separator

    const char* param = parser_eat(parser, TOK_ID)->val;
    kill_if_reserved_name(parser, param);
    kill_if_duplicated_name(parser, param, ast->struct_def.param_names, ast->struct_def.param_size);

    ast->struct_def.param_size++;
    ast->struct_def.param_names = SEAL_REALLOC(ast->struct_def.param_names, ast->struct_def.param_size * sizeof(char*));
    ast->struct_def.param_names[ast->struct_def.param_size - 1] = param;
  }

  parser_eat(parser, TOK_RPAREN);

  // field block
  parser_eat(parser, TOK_NEWL);
  parser_eat(parser, TOK_INDENT);

  while (!parser_match(parser, TOK_DEDENT)) {
    parser_eat(parser, TOK_DOT);

    // no duplicate allowed
    const char* field_name = parser_eat(parser, TOK_ID)->val;
    kill_if_duplicated_name(parser, field_name, ast->struct_def.field_names, ast->struct_def.field_size);

    ast->struct_def.field_size++;
    // field names
    ast->struct_def.field_names = SEAL_REALLOC(ast->struct_def.field_names, ast->struct_def.field_size * sizeof(char*));
    ast->struct_def.field_names[ast->struct_def.field_size - 1] = field_name;

    // field vals
    ast->struct_def.field_exprs = SEAL_REALLOC(ast->struct_def.field_exprs, ast->struct_def.field_size * sizeof(ast_t*));

    if (parser_match(parser, TOK_ASSIGN)) {
      parser_advance(parser); // '='
      ast->struct_def.field_exprs[ast->struct_def.field_size - 1] = parser_parse_expr(parser);
    } else {
      ast->struct_def.field_exprs[ast->struct_def.field_size - 1] = ast_null();
    }

    parser_eat(parser, TOK_NEWL);
  }

  parser_eat(parser, TOK_DEDENT);

  return ast;
}
/* parse block control */
static inline ast_t* parser_parse_skip(parser_t* parser)
{
  parser_advance(parser); // 'skip'
  ast_t* ast = static_create_ast(AST_SKIP, parser_line(parser));
  return ast;
}
static inline ast_t* parser_parse_stop(parser_t* parser)
{
  parser_advance(parser); // 'stop'
  ast_t* ast = static_create_ast(AST_STOP, parser_line(parser));
  return ast;
}
static inline ast_t* parser_parse_return(parser_t* parser)
{
  parser_advance(parser); // 'return'
  ast_t* ast = static_create_ast(AST_RETURN, parser_line(parser));
  ast->_return.expr = parser_match(parser, TOK_NEWL) ? ast_null() : parser_parse_expr(parser);
  return ast;
}
/* parse operations & expressions */
static inline ast_t* parser_parse_expr(parser_t* parser)
{
  return parser_match(parser, TOK_IF) ? parser_parse_ternary(parser) : parser_parse_or(parser);
}
static ast_t* parser_parse_ternary(parser_t* parser)
{
  ast_t* ast = static_create_ast(AST_TERNARY, parser_line(parser));
  parser_eat(parser, TOK_IF);
  ast->ternary.cond = parser_parse_expr(parser);
  parser_eat(parser, TOK_THEN);
  ast->ternary.expr_true = parser_parse_expr(parser);
  parser_eat(parser, TOK_ELSE);
  ast->ternary.expr_false = parser_parse_expr(parser);
  return ast;
}
static ast_t* parser_parse_or(parser_t* parser)
{
  ast_t* left = parser_parse_and(parser);

  while (parser_match(parser, TOK_OR)) {
    ast_t* bin = static_create_ast(AST_BINARY_BOOL, parser_line(parser));
    bin->binary_bool.left = left;
    bin->binary_bool.op_type = parser_advance(parser)->type; // optype
    bin->binary_bool.right = parser_parse_and(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_and(parser_t* parser)
{
  ast_t* left = parser_parse_equal(parser);

  while (parser_match(parser, TOK_AND)) {
    ast_t* bin = static_create_ast(AST_BINARY_BOOL, parser_line(parser));
    bin->binary_bool.left = left;
    bin->binary_bool.op_type = parser_advance(parser)->type; // optype
    bin->binary_bool.right = parser_parse_equal(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_equal(parser_t* parser)
{
  ast_t* left = parser_parse_compare(parser);

  while (parser_match(parser, TOK_EQ) ||
         parser_match(parser, TOK_NE)) {
    ast_t* bin = static_create_ast(AST_BINARY, parser_line(parser));
    bin->binary.left = left;
    bin->binary.op_type = parser_advance(parser)->type; // optype
    bin->binary.right = parser_parse_compare(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_compare(parser_t* parser)
{
  ast_t* left = parser_parse_term(parser);

  while (parser_match(parser, TOK_GT) ||
         parser_match(parser, TOK_GE) ||
         parser_match(parser, TOK_LT) ||
         parser_match(parser, TOK_LE)) {
    ast_t* bin = static_create_ast(AST_BINARY, parser_line(parser));
    bin->binary.left = left;
    bin->binary.op_type = parser_advance(parser)->type; // optype
    bin->binary.right = parser_parse_term(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_term(parser_t* parser)
{
  ast_t* left = parser_parse_factor(parser);

  while (parser_match(parser, TOK_PLUS) ||
         parser_match(parser, TOK_MINUS)) {
    ast_t* bin = static_create_ast(AST_BINARY, parser_line(parser));
    bin->binary.left = left;
    bin->binary.op_type = parser_advance(parser)->type; // optype
    bin->binary.right = parser_parse_factor(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_factor(parser_t* parser)
{
  ast_t* left = parser_parse_pow(parser);

  while (parser_match(parser, TOK_MUL) ||
         parser_match(parser, TOK_DIV) ||
         parser_match(parser, TOK_MOD)) {
    ast_t* bin = static_create_ast(AST_BINARY, parser_line(parser));
    bin->binary.left = left;
    bin->binary.op_type = parser_advance(parser)->type; // optype
    bin->binary.right = parser_parse_pow(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_pow(parser_t* parser)
{
  ast_t* left = parser_parse_unary(parser);

  if (parser_match(parser, TOK_POW)) {
    ast_t* bin = static_create_ast(AST_BINARY, parser_line(parser));
    bin->binary.left = left;
    bin->binary.op_type = parser_advance(parser)->type; // optype
    bin->binary.right = parser_parse_pow(parser);
    left = bin;
  }

  return left;
}
static ast_t* parser_parse_unary(parser_t* parser)
{
  if (parser_match(parser, TOK_NOT) ||
      parser_match(parser, TOK_MINUS) ||
      parser_match(parser, TOK_PLUS)) {
    ast_t* ast = static_create_ast(AST_UNARY, parser_line(parser));
    ast->unary.op_type = parser_advance(parser)->type; // optype
    ast->unary.expr = parser_parse_unary(parser);
    return ast;
  }
  return parser_parse_primary(parser);
}
static ast_t* parser_parse_primary(parser_t* parser)
{
  ast_t* main = NULL;
  switch (parser_peek(parser)->type) {
    case TOK_ID:
      main = parser_parse_id(parser);
      break;
    case TOK_LPAREN:
      parser_advance(parser); // '('
      main = parser_parse_expr(parser);
      parser_eat(parser, TOK_RPAREN);
      break;
    case TOK_LBRACK:
      main = parser_parse_list(parser);
      break;
    case TOK_LBRACE:
      main = parser_parse_object(parser);
      break;
    case TOK_INT:
      main = parser_parse_int(parser);
      break;
    case TOK_FLOAT:
      main = parser_parse_float(parser);
      break;
    case TOK_STRING:
      main = parser_parse_string(parser);
      break;
    case TOK_TRUE:
      parser_advance(parser);
      main = ast_true();
      break;
    case TOK_FALSE:
      parser_advance(parser);
      main = ast_false();
      break;
    case TOK_NULL:
      parser_advance(parser);
      main = ast_null();
      break;
    default: {
      char err[ERR_LEN];
      sprintf(err, "invalid primary expression: \'%s\'", parser_peek(parser)->val);
      return parser_error(parser, err);
    }
  }

  // handle postfix operators
  while (true) {
    if (parser_match(parser, TOK_LBRACK)) { // subscript
      parser_advance(parser); // '['
      ast_t* index = parser_parse_expr(parser);
      parser_eat(parser, TOK_RBRACK); // require ']'

      ast_t* subscript = static_create_ast(AST_SUBSCRIPT, main->line);
      subscript->subscript.main = main;
      subscript->subscript.index = index;

      main = subscript; // assign to 'main'
    } else if (parser_match(parser, TOK_DOT)) { // member access
      parser_advance(parser); // '.'
      ast_t* mem = parser_parse_var_ref(parser); // must be only 'identifier' (variable reference)

      ast_t* memacc = static_create_ast(AST_MEMACC, main->line);
      memacc->memacc.main = main;
      memacc->memacc.mem = mem;

      main = memacc; // assign to 'main'
    } else if (parser_match(parser, TOK_COLON)) { // library function call
      if (main->type != AST_VAR_REF) {
        char err[ERR_LEN];
        sprintf(err, "invalid library name");
        return parser_error(parser, err);
      }
      parser_advance(parser); // ':'
      ast_t* func_call = parser_parse_func_call(parser); // must be only 'function call'

      ast_t* lib_fcall = static_create_ast(AST_LIB_FUNC_CALL, main->line);
      lib_fcall->lib_func_call.lib = main;
      lib_fcall->lib_func_call.func_call = func_call;

      main = lib_fcall; // assign to 'main'
    } else {
      break;
    }
  }

  // handle assignment
  if (parser_match(parser, TOK_ASSIGN) && is_lvalue(main)) {
    parser_advance(parser); // '='
    ast_t* assign = static_create_ast(AST_ASSIGN, parser_line(parser));
    assign->assign.var = main;
    assign->assign.expr = parser_parse_expr(parser);
    main = assign;
  }

  return main;
}
/* parse others */
static ast_t* parser_parse_include(parser_t* parser)
{
  parser_advance(parser); // 'include'
  ast_t* ast = static_create_ast(AST_INCLUDE, parser_line(parser));
  ast->include.name = parser_match(parser, TOK_STRING) ? parser_advance(parser)->val : parser_eat(parser, TOK_ID)->val;
  return ast;
}
