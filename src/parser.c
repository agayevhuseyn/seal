#include "parser.h"
#include "ast.h"
#include "token.h"
#include <stdio.h>
#include <string.h>

parser_t* init_parser(lexer_t* lexer)
{
  parser_t* parser = calloc(1, sizeof(parser_t));

  parser->tokens = lexer->tokens;
  parser->token_size = lexer->token_size;
  parser->i = 0;
  parser->func_defs = (void*)0;
  parser->func_size = 0;
  parser->obj_defs = (void*)0;
  parser->obj_size = 0;

  return parser;
}

static inline ast_t* parser_error(parser_t* parser, const char* msg)
{
  printf("Parser-> Error at line: %u, %s\n", parser_peek(parser)->line, msg);
  exit(1);
  return ast_noop();
}

bool parser_is_end(parser_t* parser)
{
  return parser->i >= parser->token_size;
}

token_t* parser_peek(parser_t* parser)
{
  return parser->tokens[parser->i];
}

token_t* parser_advance(parser_t* parser)
{
  return parser->tokens[parser->i++];
}

token_t* parser_eat(parser_t* parser, Token_Type type)
{
  if (parser_peek(parser)->type != type) {
    char msg[128];
    sprintf(msg, "expected \"%s\", got \"%s\"", token_name(type), token_name(parser_peek(parser)->type));
    parser_error(parser, msg);
  }
  return parser_advance(parser);
}

token_t* parser_peek_offset(parser_t* parser, int offset)
{
  int i = parser->i + offset;
  return i < parser->token_size && i >= 0 ? parser->tokens[i] : parser->tokens[parser->token_size - 1];
}

ast_t* parser_parse(parser_t* parser)
{
  return parser_parse_statements(parser, false, false, false);
}

ast_t* parser_parse_statements(parser_t* parser, bool is_func, bool is_ifelse, bool is_loop)
{
  ast_t* ast = init_ast(AST_COMP);

  ast->comp.stmt_size = 1; 
  ast->comp.stmts = calloc(1, sizeof(ast_t*));
  ast->comp.stmts[0] = parser_parse_statement(parser, is_func, is_ifelse, is_loop);

  while (!parser_is_end(parser) && parser_peek(parser)->type == TOK_NEWL) {
    parser_eat(parser, TOK_NEWL);

    ast->comp.stmt_size++;
    ast->comp.stmts = realloc(ast->comp.stmts, ast->comp.stmt_size * sizeof(ast_t*));
    ast->comp.stmts[ast->comp.stmt_size - 1] = parser_parse_statement(parser, is_func, is_ifelse, is_loop);
  }
  if ((!is_func && !is_ifelse && !is_loop) && parser_peek(parser)->type != TOK_EOF) {
    char msg[128];
    sprintf(msg, "syntax error: encountered \"%s\"", token_name(parser_peek(parser)->type));
    return parser_error(parser, msg);
  } else if ((is_func || is_ifelse || is_loop) && parser_peek(parser)->type != TOK_DEDENT) {
    char msg[128];
    sprintf(msg, "syntax error: encountered \"%s\"", token_name(parser_peek(parser)->type));
    return parser_error(parser, msg);
  }

  return ast;
}

ast_t* parser_parse_statement(parser_t* parser, bool is_func, bool is_ifelse, bool is_loop)
{
  switch (parser_peek(parser)->type) {
    case TOK_INCLUDE:
      if (is_func || is_ifelse || is_loop) goto error;
      return parser_parse_include(parser);
    case TOK_OBJECT:
      if (is_func || is_ifelse || is_loop) goto error;
      return parser_parse_obj_def(parser);
    case TOK_SKIP:
      if (!is_loop) goto error;
      return parser_parse_skip(parser);
    case TOK_STOP:
      if (!is_loop) goto error;
      return parser_parse_stop(parser);
    case TOK_VAR:
      return parser_parse_vardef(parser);
    case TOK_WHILE:
      return parser_parse_while(parser, is_func);
    case TOK_IF:
      return parser_parse_if(parser, is_func, is_loop);
    case TOK_DEFINE:
      if (is_func || is_ifelse || is_loop) goto error;
      return parser_parse_func_def(parser);
    case TOK_EOF:
      if (is_func || is_ifelse || is_loop) goto error;
      return ast_noop();
    case TOK_DEDENT:
      if (!is_func && !is_ifelse && !is_loop) goto error;
      return ast_noop();
    case TOK_RETURN:
      if (!is_func) goto error;
      return parser_parse_return(parser);
    default: 
      return parser_parse_expr(parser);
  }
  error: {
    char msg[128];
    sprintf(msg, "unexpected token at parse statement: \"%s\"", token_name(parser_peek(parser)->type));
    return parser_error(parser, msg);
  }
}

ast_t* parser_parse_expr(parser_t* parser)
{
  return parser_parse_or(parser);
}

ast_t* parser_parse_or(parser_t* parser)
{
  ast_t* left = parser_parse_and(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_OR)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_and(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_and(parser_t* parser)
{
  ast_t* left = parser_parse_equal(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_AND)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_equal(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_equal(parser_t* parser)
{
  ast_t* left = parser_parse_compare(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_EQ ||
        t == TOK_NE)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_compare(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_compare(parser_t* parser)
{
  ast_t* left = parser_parse_term(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_GT ||
        t == TOK_GE ||
        t == TOK_LT ||
        t == TOK_LE)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_term(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_term(parser_t* parser)
{
  ast_t* left = parser_parse_factor(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_PLUS ||
        t == TOK_MINUS)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_factor(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_factor(parser_t* parser)
{
  ast_t* left = parser_parse_unary(parser);

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_MUL ||
        t == TOK_DIV ||
        t == TOK_MOD)
  ) {
    ast_t* bin = init_ast(AST_BINARY);
    bin->binary.left = left; 
    bin->binary.op = parser_advance(parser)->type;
    bin->binary.right = parser_parse_unary(parser); 

    left = bin;

    t = parser_peek(parser)->type;
  }

  return left;
}

ast_t* parser_parse_unary(parser_t* parser)
{
  ast_t* ast = (void*)0;
  switch (parser_peek(parser)->type) {
    case TOK_MINUS:
    case TOK_NOT:
      ast = init_ast(AST_UNARY);
      ast->unary.op = parser_advance(parser)->type;
      ast->unary.expr = parser_parse_primary(parser);
      break;
    default:
      ast = parser_parse_primary(parser);
      break;
  }

  return ast;
}

ast_t* parser_parse_primary(parser_t* parser)
{
  ast_t* ast = (void*)0;
  bool is_const = true;
  switch (parser_peek(parser)->type) {
    case TOK_INT: ast = parser_parse_int(parser); break;
    case TOK_FLOAT: ast = parser_parse_float(parser); break;
    case TOK_STRING: ast = parser_parse_string(parser); break;
    case TOK_TRUE: ast = ast_true(); parser_advance(parser); break;
    case TOK_FALSE: ast = ast_false(); parser_advance(parser); break;
    case TOK_NULL: ast = ast_null(); parser_advance(parser); break;
    case TOK_ID: ast = parser_parse_id(parser); is_const = false; break;
    case TOK_LBRACK: ast = parser_parse_list(parser); break;
    case TOK_LPAREN: {
      parser_advance(parser);
      ast = parser_parse_expr(parser);
      parser_eat(parser, TOK_RPAREN);
      break;
    }
    default: {
      char msg[128];
      sprintf(msg, "unexpected token at parse primary: \"%s\"", token_name(parser_peek(parser)->type));
      return parser_error(parser, msg);
    }
  }

  Token_Type t = parser_peek(parser)->type;
  while (!parser_is_end(parser) && (
        t == TOK_DOT ||
        t == TOK_LBRACK ||
        t == TOK_DCOLON)
  ) {
    if (t == TOK_DOT) {
      parser_eat(parser, TOK_DOT);

      ast_t* mem_acc = init_ast(AST_MEM_ACC);

      mem_acc->mem_acc.main = ast;
      mem_acc->mem_acc.mem = parser_parse_var(parser);

      ast = mem_acc;
    } else if (t == TOK_LBRACK) {
      parser_eat(parser, TOK_LBRACK);

      ast_t* sub = init_ast(AST_SUBSCRIPT);

      sub->subscript.main = ast;
      sub->subscript.index = parser_parse_expr(parser);

      parser_eat(parser, TOK_RBRACK);

      ast = sub;
    } else if (t == TOK_DCOLON) {
      parser_eat(parser, TOK_DCOLON);

      ast_t* mfcall = init_ast(AST_MODULE_FCALL);
      
      mfcall->module_fcall.module = ast;
      mfcall->module_fcall.func_call = parser_parse_func_call(parser);

      ast = mfcall;
    }

    t = parser_peek(parser)->type;
  }

  if (is_const) return ast;

  switch (parser_peek(parser)->type) {
    case TOK_ASSIGN: {
      ast_t* assign = init_ast(AST_ASSIGN);

      assign->assign.var = ast;
      parser_advance(parser);
      assign->assign.right = parser_parse_expr(parser);
      ast = assign;
    }
      break;
    default:
      break;
  }

  return ast;
}

ast_t* parser_parse_string(parser_t* parser)
{
  ast_t* ast = init_ast(AST_STRING);
  ast->string.val = parser_eat(parser, TOK_STRING)->value;
  if (parser_peek(parser)->type == TOK_LBRACK) {
    ast_t* subscript = init_ast(AST_SUBSCRIPT);
    parser_eat(parser, TOK_LBRACK);
    subscript->subscript.main = ast;
    subscript->subscript.index = parser_parse_expr(parser);
    parser_eat(parser, TOK_RBRACK);
    ast = subscript;
  }
  return ast;
}

ast_t* parser_parse_int(parser_t* parser)
{
  ast_t* ast = init_ast(AST_INT);
  ast->integer.val = atoi(parser_eat(parser, TOK_INT)->value);
  return ast;
}

ast_t* parser_parse_float(parser_t* parser)
{
  ast_t* ast = init_ast(AST_FLOAT);
  ast->floating.val = atof(parser_eat(parser, TOK_FLOAT)->value);
  return ast;
}

ast_t* parser_parse_list(parser_t* parser)
{
  ast_t* ast = init_ast(AST_LIST);
  ast->list.mem_size = 0;
  ast->list.mems = (void*)0;

  parser_eat(parser, TOK_LBRACK);

  if (parser_peek(parser)->type != TOK_RBRACK) {
    ast->list.mem_size = 1;
    ast->list.mems = calloc(1, sizeof(ast_t*));
    ast->list.mems[0] = parser_parse_expr(parser);
  }

  while (!parser_is_end(parser) && parser_peek(parser)->type == TOK_COMMA) {
    parser_eat(parser, TOK_COMMA);

    ast->list.mem_size++;
    ast->list.mems = realloc(ast->list.mems, ast->list.mem_size * sizeof(ast_t*));
    ast->list.mems[ast->list.mem_size - 1] = parser_parse_expr(parser);
  }

  parser_eat(parser, TOK_RBRACK);
  return ast;
}

ast_t* parser_parse_func_call(parser_t* parser)
{
  ast_t* ast = init_ast(AST_FUNC_CALL);

  ast->func_call.fname = parser_eat(parser, TOK_ID)->value;
  parser_eat(parser, TOK_LPAREN);

  ast->func_call.arg_size = 0;
  ast->func_call.args = (void*)0;

  if (parser_peek(parser)->type != TOK_RPAREN) {
    ast->func_call.arg_size = 1;
    ast->func_call.args = calloc(1, sizeof(ast_t*));
    ast->func_call.args[0] = parser_parse_expr(parser);
  }
  while (!parser_is_end(parser) && parser_peek(parser)->type != TOK_RPAREN) {
    parser_eat(parser, TOK_COMMA);

    ast->func_call.arg_size++;
    ast->func_call.args = realloc(ast->func_call.args, ast->func_call.arg_size * sizeof(ast_t*));
    ast->func_call.args[ast->func_call.arg_size - 1] = parser_parse_expr(parser);
  }
  parser_eat(parser, TOK_RPAREN);

  return ast;
}

ast_t* parser_parse_id(parser_t* parser)
{
  if (parser_peek_offset(parser, 1)->type == TOK_LPAREN)
    return parser_parse_func_call(parser);
  return parser_parse_var(parser);
}

ast_t* parser_parse_var(parser_t* parser)
{
  ast_t* ast = init_ast(AST_VAR_REF);
  ast->var_ref.name = parser_eat(parser, TOK_ID)->value;

  return ast;
}

ast_t* parser_parse_vardef(parser_t* parser)
{
  ast_t* ast = init_ast(AST_VARDEF);
  parser_eat(parser, TOK_VAR);

  ast->vardef.def_size = 1;
  ast->vardef.names = calloc(1, sizeof(char*));
  ast->vardef.is_defined_arr = calloc(1, sizeof(bool));
  ast->vardef.rights = calloc(1, sizeof(ast_t*));

  ast->vardef.names[0] = parser_eat(parser, TOK_ID)->value;
  ast->vardef.is_defined_arr[0] = false;
  ast->vardef.rights[0] = (void*)0;

  if (parser_peek(parser)->type == TOK_ASSIGN) {
    parser_advance(parser);

    ast->vardef.is_defined_arr[0] = true;
    ast->vardef.rights[0] = parser_parse_expr(parser);
  }

  while (!parser_is_end(parser) && parser_peek(parser)->type != TOK_NEWL) {
    parser_eat(parser, TOK_COMMA);

    ast->vardef.def_size++;
    ast->vardef.names = realloc(ast->vardef.names, ast->vardef.def_size * sizeof(char*));
    ast->vardef.is_defined_arr = realloc(ast->vardef.is_defined_arr, ast->vardef.def_size * sizeof(bool));
    ast->vardef.rights = realloc(ast->vardef.rights, ast->vardef.def_size * sizeof(ast_t*));

    ast->vardef.names[ast->vardef.def_size - 1] = parser_eat(parser, TOK_ID)->value;
    ast->vardef.is_defined_arr[ast->vardef.def_size - 1] = false;
    ast->vardef.rights[ast->vardef.def_size - 1] = (void*)0;

    if (parser_peek(parser)->type == TOK_ASSIGN) {
      parser_advance(parser);

      ast->vardef.is_defined_arr[ast->vardef.def_size - 1] = true;
      ast->vardef.rights[ast->vardef.def_size - 1] = parser_parse_expr(parser);
    }
  }

  return ast;
}

ast_t* parser_parse_func_def(parser_t* parser)
{
  parser_eat(parser, TOK_DEFINE);

  ast_t* ast = init_ast(AST_FUNC_DEF);
  ast->func_def.fname = parser_eat(parser, TOK_ID)->value;
  // check if function is defined before
  for (int i = 0 ; i < parser->func_size; i++) {
    if (strcmp(ast->func_def.fname, parser->func_defs[i]->func_def.fname) == 0) {
      char msg[128];
      sprintf(msg, "redefinition of function \"%s\"", ast->func_def.fname);
      return parser_error(parser, msg);
    }
  }
  for (int i = 0 ; i < parser->obj_size; i++) {
    if (strcmp(ast->func_def.fname, parser->obj_defs[i]->obj_def.oname) == 0) {
      char msg[128];
      sprintf(msg, "redefinition of function \"%s\"", ast->func_def.fname);
      return parser_error(parser, msg);
    }
  }

  ast->func_def.arg_size = 0;

  parser_eat(parser, TOK_LPAREN);

  if (parser_peek(parser)->type != TOK_RPAREN) {
    ast->func_def.arg_size = 1;
    ast->func_def.args = calloc(1, sizeof(char*));

    char* arg = parser_eat(parser, TOK_ID)->value;

    ast->func_def.args[0] = arg;
  }

  while(parser_peek(parser)->type != TOK_RPAREN) {
    parser_eat(parser, TOK_COMMA);
    
    ast->func_def.arg_size++;
    ast->func_def.args = realloc(ast->func_def.args, ast->func_def.arg_size * sizeof(ast_t*));

    char* arg = parser_eat(parser, TOK_ID)->value;

    ast->func_def.args[ast->func_def.arg_size - 1] = arg;

    for (int i = 0; i < ast->func_def.arg_size - 1; i++) {
      if (strcmp(arg, ast->func_def.args[i]) == 0) {
        char msg[128];
        sprintf(msg,
                "duplicate parameter \"%s\" in function \"%s\"",
                arg,
                ast->func_def.fname);
        return parser_error(parser, msg);
      }
    }
  }

  parser_eat(parser, TOK_RPAREN);
  parser_eat(parser, TOK_NEWL);

  parser_eat(parser, TOK_INDENT);

  ast->func_def.body = parser_parse_statements(parser, true, false, false);

  parser_eat(parser, TOK_DEDENT);

  // add to parser
  parser->func_size++;
  parser->func_defs = realloc(parser->func_defs, parser->func_size * sizeof(ast_t*));
  parser->func_defs[parser->func_size - 1] = ast;

  return ast_noop();
}

ast_t* parser_parse_obj_def(parser_t* parser)
{
  parser_eat(parser, TOK_OBJECT);

  ast_t* ast = init_ast(AST_OBJ_DEF);
  ast->obj_def.oname = parser_eat(parser, TOK_ID)->value;
  // check if object is defined before
  for (int i = 0 ; i < parser->obj_size; i++) {
    if (strcmp(ast->obj_def.oname, parser->obj_defs[i]->obj_def.oname) == 0) {
      char msg[128];
      sprintf(msg, "redefinition of object \"%s\"", ast->obj_def.oname);
      return parser_error(parser, msg);
    }
  }
  for (int i = 0 ; i < parser->func_size; i++) {
    if (strcmp(ast->obj_def.oname, parser->func_defs[i]->func_def.fname) == 0) {
      char msg[128];
      sprintf(msg, "redefinition of object \"%s\"", ast->obj_def.oname);
      return parser_error(parser, msg);
    }
  }
  ast->obj_def.field_size = 0;
  ast->obj_def.fields = (void*)0;
  ast->obj_def.params = (void*)0;
  ast->obj_def.param_size = 0;

  parser_eat(parser, TOK_LPAREN);

  if (parser_peek(parser)->type != TOK_RPAREN) {
    ast->obj_def.param_size = 1;
    ast->obj_def.params = calloc(1, sizeof(char*));

    char* arg = parser_eat(parser, TOK_ID)->value;

    ast->obj_def.params[0] = arg;
  }

  while (parser_peek(parser)->type != TOK_RPAREN) {
    parser_eat(parser, TOK_COMMA);
    
    ast->obj_def.param_size++;
    ast->obj_def.params = realloc(ast->obj_def.params, ast->obj_def.param_size * sizeof(char*));

    char* arg = parser_eat(parser, TOK_ID)->value;

    ast->obj_def.params[ast->obj_def.param_size - 1] = arg;
    for (int i = 0; i < ast->obj_def.param_size - 1; i++) {
      if (strcmp(arg, ast->obj_def.params[i]) == 0) {      
        char msg[128];
        sprintf(msg,
                "duplicate parameter \"%s\" in object initialization function \"%s\"",
                arg,
                ast->obj_def.oname);
        return parser_error(parser, msg);
      }
    }
  }

  parser_eat(parser, TOK_RPAREN);

  parser_eat(parser, TOK_NEWL);
  parser_eat(parser, TOK_INDENT);

  while (!parser_is_end(parser) && parser_peek(parser)->type != TOK_DEDENT) {
    ast->obj_def.field_size++;
    ast->obj_def.fields = realloc(ast->obj_def.fields, ast->obj_def.field_size * sizeof(struct field*));
    ast->obj_def.fields[ast->obj_def.field_size - 1] = calloc(1, sizeof(struct field));
    ast->obj_def.fields[ast->obj_def.field_size - 1]->is_defined = false;
    ast->obj_def.fields[ast->obj_def.field_size - 1]->name = parser_eat(parser, TOK_ID)->value;
    if (parser_peek(parser)->type == TOK_ASSIGN) {
      parser_advance(parser);
      ast->obj_def.fields[ast->obj_def.field_size - 1]->def = parser_parse_expr(parser);
      ast->obj_def.fields[ast->obj_def.field_size - 1]->is_defined = true;
    }
    parser_eat(parser, TOK_NEWL);
  }
  parser_eat(parser, TOK_DEDENT);

  // add to parser
  parser->obj_size++;
  parser->obj_defs = realloc(parser->obj_defs, parser->obj_size * sizeof(ast_t*));
  parser->obj_defs[parser->obj_size - 1] = ast;

  return ast_noop();
}

ast_t* parser_parse_return(parser_t* parser)
{
  ast_t* ast = init_ast(AST_RETURN);
  parser_eat(parser, TOK_RETURN);

  ast->__return.is_empty = true;
  if (parser_peek(parser)->type != TOK_NEWL) {
    ast->__return.is_empty = false;
    ast->__return.expr = parser_parse_expr(parser);
  }

  return ast;
}

ast_t* parser_parse_skip(parser_t* parser)
{
  parser_eat(parser, TOK_SKIP);
  return init_ast(AST_SKIP);
}

ast_t* parser_parse_stop(parser_t* parser)
{
  parser_eat(parser, TOK_STOP);
  return init_ast(AST_STOP);
}

ast_t* parser_parse_include(parser_t* parser)
{
  parser_eat(parser, TOK_INCLUDE);
  ast_t* ast = init_ast(AST_INCLUDE);
  ast->include.module_name = parser_eat(parser, TOK_ID)->value;
  return ast;
}

ast_t* parser_parse_if(parser_t* parser, bool is_func, bool is_loop)
{
  parser_eat(parser, TOK_IF);
  
  ast_t* ast = init_ast(AST_IF);
  ast->__if.cond = parser_parse_expr(parser);

  parser_eat(parser, TOK_NEWL);
  parser_eat(parser, TOK_INDENT);

  ast->__if.body = parser_parse_statements(parser, is_func, true, is_loop);
  ast->__if.has_else = false;

  parser_eat(parser, TOK_DEDENT);
  
  if (parser_peek_offset(parser, 1)->type == TOK_ELSE) {
    parser_eat(parser, TOK_NEWL);
    ast->__if.has_else = true;
    if (parser_peek_offset(parser, 1)->type == TOK_IF) {
      parser_advance(parser);
      ast->__if.else_part = parser_parse_if(parser, is_func, is_loop);
    } else {
      ast->__if.else_part = parser_parse_else(parser, is_func, is_loop);
    }
  }

  return ast;
}

ast_t* parser_parse_else(parser_t* parser, bool is_func, bool is_loop)
{
  parser_eat(parser, TOK_ELSE);
  parser_eat(parser, TOK_NEWL);

  ast_t* ast = init_ast(AST_ELSE);
  parser_eat(parser, TOK_INDENT);

  ast->__else.body = parser_parse_statements(parser, is_func, true, is_loop);

  parser_eat(parser, TOK_DEDENT);

  return ast;
}

ast_t* parser_parse_while(parser_t* parser, bool is_func)
{
  parser_eat(parser, TOK_WHILE);

  ast_t* ast = init_ast(AST_WHILE);
  ast->__while.cond = parser_parse_expr(parser);
  parser_eat(parser, TOK_NEWL);
  parser_eat(parser, TOK_INDENT);
  ast->__while.body = parser_parse_statements(parser, is_func, false, true);
  parser_eat(parser, TOK_DEDENT);

  return ast;
}
