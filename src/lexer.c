#include "lexer.h"

#define LEXEME_ID                   0
#define LEXEME_DIGIT                1
#define LEXEME_FRACTION_BEGIN_DIGIT 2
#define LEXEME_STRING               3

#define SINGLE_LINE_COMMENT 0
#define MULTILINE_COMMENT   1

#define lexer_is_end(lexer) ( \
  lexer->i >= lexer->src_size)

#define lexer_peek_offset(lexer, offset) ( \
  (lexer->i + offset >= 0 && lexer->i + offset < lexer->src_size) ? lexer->src[lexer->i + offset] : '\0')

#define lexer_peek(lexer) ( \
  lexer_is_end(lexer) ? '\0' : lexer->src[lexer->i])
  
#define lexer_is_alpha(c) ( \
  (c >= 'a' && c <= 'z') || \
  (c >= 'A' && c <= 'Z'))

#define lexer_is_digit(c) ( \
  c >= '0' && c <= '9')

#define lexer_is_id(c) ( \
  lexer_is_alpha(c) || c == '_')

#define lexer_is_alnum_(c) ( \
  lexer_is_id(c) || lexer_is_digit(c))

static inline void lexer_error(lexer_t* lexer, const char* err, int line)
{
  fprintf(stderr, "seal: line %d\nsyntax error: %s\n", line == 0 ? lexer->line : line, err);
  exit(EXIT_FAILURE);
}

/* main functions */
inline void lexer_get_tokens(lexer_t* lexer)
{
  while (!lexer_is_end(lexer)) {
    lexer_get_token(lexer);
  }
  if (lexer->paren_stack_ptr > 0) {
    char err[ERR_LEN];
    sprintf(err, "'%c' was never closed", lexer->paren_stack[lexer->paren_stack_ptr]);
    lexer_error(lexer, err, lexer->paren_lines_stack[lexer->paren_lines_ptr]);
  }

  if (lexer->tok_size > 0 && lexer->toks[lexer->tok_size - 1]->type != TOK_NEWL) {
    lexer_add_token(lexer, create_token(TOK_NEWL, NULL, lexer->line));
  }

  for (int i = 0; i < lexer->indent_stack_ptr; i++) {
    lexer_add_token(lexer, create_token(TOK_DEDENT, NULL, lexer->line));
    lexer_add_token(lexer, create_token(TOK_NEWL, NULL, lexer->line));
  }

  lexer_add_token(lexer, create_token(TOK_EOF, NULL, lexer->line));

  SEAL_FREE((char*)lexer->src); // free source file after lexing
}

static inline void stack_push(int val, int stack[], int stack_size, int* stack_ptr)
{
  if (*stack_ptr == stack_size - 1) {
    fprintf(stderr, "%d stack limit exceeded\n", stack_size);
    exit(EXIT_FAILURE);
  }
  stack[++*stack_ptr] = val;
}

static inline int stack_pop(int stack[], int stack_size, int* stack_ptr)
{
  if (*stack_ptr == 0) {
    fprintf(stderr, "popping empty stack\n");
    exit(EXIT_FAILURE);
  }
  int popped = stack[*stack_ptr];
  stack[(*stack_ptr)--] = -1;
  return popped;
}

static void lexer_get_token(lexer_t* lexer)
{
  bool encountered_word = true;

  char c = lexer_advance(lexer);
  token_t* tok = NULL;

  switch (c) {
    case ' ':
    case '\t':
      if (!lexer->encountered_word) {
        lexer->cur_indent++;
        encountered_word = false;
      }
      break;
    case '\n':
      lexer->cur_indent = 0;
      encountered_word  = false;
      if (lexer->encountered_word && lexer->paren_lines_ptr == 0) {
        tok = create_token(TOK_NEWL, NULL, lexer->line);
      }
      lexer->token_after_comment = false;
      lexer->token_after_paren   = false;
      lexer->line++;
      break;
    case '.':
      if (lexer_is_digit(lexer_peek(lexer)))
        tok = lexer_get_digit(lexer, LEXEME_FRACTION_BEGIN_DIGIT);
      else
        tok = create_token(TOK_DOT, NULL, lexer->line);
      break;
    case ':':
      tok = create_token(TOK_COLON, NULL, lexer->line);
      break;
    case ',':
      tok = create_token(TOK_COMMA, NULL, lexer->line);
      break;
    case '+':
      tok = create_token(TOK_PLUS, NULL, lexer->line);
      break;
    case '-':
      tok = create_token(TOK_MINUS, NULL, lexer->line);
      break;
    case '*':
      tok = create_token(TOK_MUL, NULL, lexer->line);
      break;
    case '/':
      if (lexer_match(lexer, '/')) 
        lexer_ignore_comment(lexer, SINGLE_LINE_COMMENT);
      else if (lexer_match(lexer, '*'))
        lexer_ignore_comment(lexer, MULTILINE_COMMENT);
      else
        tok = create_token(TOK_DIV, NULL, lexer->line);
      encountered_word = lexer->encountered_word;
      break;
    case '%':
      tok = create_token(TOK_MOD, NULL, lexer->line);
      break;
    case '^':
      tok = create_token(TOK_POW, NULL, lexer->line);
      break;
    case '=': // ==
      if (lexer_match(lexer, '='))
        tok = create_token(TOK_EQ, NULL, lexer->line);
      else
        tok = create_token(TOK_ASSIGN, NULL, lexer->line);
      break;
    case '!': // !=
      if (lexer_match(lexer, '='))
        tok = create_token(TOK_NE, NULL, lexer->line);
      else
        tok = create_token(TOK_NOT, "!", lexer->line);
      break;
    case '>': // >=
      if (lexer_match(lexer, '='))
        tok = create_token(TOK_GE, NULL, lexer->line);
      else
        tok = create_token(TOK_GT, NULL, lexer->line);
      break;
    case '<': // <=
      if (lexer_match(lexer, '='))
        tok = create_token(TOK_LE, NULL, lexer->line);
      else
        tok = create_token(TOK_LT, NULL, lexer->line);
      break;
    case '(':
      tok = create_token(TOK_LPAREN, NULL, lexer->line);
      break;
    case ')':
      tok = create_token(TOK_RPAREN, NULL, lexer->line);
      break;
    case '[':
      tok = create_token(TOK_LBRACK, NULL, lexer->line);
      break;
    case ']':
      tok = create_token(TOK_RBRACK, NULL, lexer->line);
      break;
    case '{':
      tok = create_token(TOK_LBRACE, NULL, lexer->line);
      break;
    case '}':
      tok = create_token(TOK_RBRACE, NULL, lexer->line);
      break;
    case '\'':
    case '\"':
      tok = lexer_get_string(lexer, c);
      break;
    default:
      if (lexer_is_id(c))
        tok = lexer_get_id(lexer);
      else if (lexer_is_digit(c))
        tok = lexer_get_digit(lexer, LEXEME_DIGIT);
      else {
        char err[ERR_LEN];
        sprintf(err, "unexpected character: '%c'", c);
        lexer_error(lexer, err, 0);
      }
      break;
  }

  // indentation stack
  if (lexer->paren_stack_ptr == 0 && !lexer->encountered_word && encountered_word) {
    lexer->encountered_word = encountered_word;
    if (lexer->cur_indent > lexer->indent_stack[lexer->indent_stack_ptr]) {
      stack_push(lexer->cur_indent, lexer->indent_stack, MAX_INDENT_LEVEL, &lexer->indent_stack_ptr);
      lexer_add_token(lexer, create_token(TOK_INDENT, NULL, lexer->line));
    } else if (lexer->cur_indent < lexer->indent_stack[lexer->indent_stack_ptr]) {
      int times_pop = 0;
      pop_indent:
        stack_pop(lexer->indent_stack, MAX_INDENT_LEVEL, &lexer->indent_stack_ptr);
        times_pop++;
        if (lexer->cur_indent != lexer->indent_stack[lexer->indent_stack_ptr]) {
          if (lexer->indent_stack_ptr > 0) {
            goto pop_indent;
          } else if (lexer->indent_stack_ptr == 0) {
            lexer_error(lexer, "mismatch unindetation", 0);
          }
        }
      /* end label pop_indent */

      for (int i = 0; i < times_pop; i++) {
        lexer_add_token(lexer, create_token(TOK_DEDENT, NULL, lexer->line));
        lexer_add_token(lexer, create_token(TOK_NEWL, NULL, lexer->line));
      }
    }
  }

  // parenthesis stack
  bool is_closing_paren = false;
  bool same_line_paren = false;
  if (tok && tok->type >= TOK_LPAREN && tok->type <= TOK_RBRACE) {
    if (tok->type == TOK_LPAREN || tok->type == TOK_LBRACK || tok->type == TOK_LBRACE) {
      stack_push(c, lexer->paren_stack, MAX_NESTED_PAREN_LEVEL, &lexer->paren_stack_ptr);
      stack_push(lexer->line, lexer->paren_lines_stack, MAX_NESTED_PAREN_LEVEL, &lexer->paren_lines_ptr);
    } else {
      if (lexer->paren_stack_ptr == 0) {
        char err[ERR_LEN];
        sprintf(err, "unmatched '%c'", c);
        lexer_error(lexer, err, 0);
      }
      int popped = stack_pop(lexer->paren_stack, MAX_NESTED_PAREN_LEVEL, &lexer->paren_stack_ptr);
      int popped_line = stack_pop(lexer->paren_lines_stack, MAX_NESTED_PAREN_LEVEL, &lexer->paren_lines_ptr);
      if (lexer->line == popped_line) {
        same_line_paren = true;
      }
      switch (c) {
        case ')':
          if (popped != '(') goto error;
          break;
        case ']':
          if (popped != '[') goto error;
          break;
        case '}':
          if (popped != '{') goto error;
          break;
        error: {
          char err[ERR_LEN];
          sprintf(err, "'%c' does not balance '%c'", c, popped);
          lexer_error(lexer, err, 0);
        }
      }
      is_closing_paren = true;
    }
  }

  if (tok) {
    if (lexer->token_after_comment) {
      char err[ERR_LEN];
      sprintf(err, "token after comment block not allowed");
      lexer_error(lexer, err, 0);
    } else if (lexer->token_after_paren) {
      char err[ERR_LEN];
      sprintf(err, "token after final parenthesis not allowed");
      lexer_error(lexer, err, 0);
    }
    lexer_add_token(lexer, tok);
  }

  lexer->encountered_word = encountered_word;

  /* assuming it is parenthesis */
  if (is_closing_paren && !same_line_paren && lexer->paren_stack_ptr == 0) {
    lexer->token_after_paren = true;
  }
}

/* char functions */
static inline char lexer_advance(lexer_t* lexer)
{
  return lexer->src[lexer->i++];
}

static inline char lexer_match(lexer_t* lexer, char c)
{
  return lexer_peek(lexer) == c ? lexer_advance(lexer) : '\0';
}

/* token functions */
static inline void lexer_add_token(lexer_t* lexer, token_t* tok)
{
  lexer->tok_size++;
  lexer->toks = SEAL_REALLOC(lexer->toks, lexer->tok_size * sizeof(token_t*));
  lexer->toks[lexer->tok_size - 1] = tok;
}

static token_t* lexer_get_id(lexer_t* lexer)
{
  char* lexeme = lexer_get_lexeme(lexer, LEXEME_ID, 0);
  for (int i = TOK_VAR; i < TOK_ID; i++) {
    if (strcmp(lexeme, htoken_type_name(i)) == 0) {
      free(lexeme);
      return create_token(i, NULL, lexer->line);
    }
  }
  return create_token(TOK_ID, lexeme, lexer->line);
}

static token_t* lexer_get_digit(lexer_t* lexer, int lexeme_type)
{
  token_t* tok = NULL;
  const char* lexeme = lexer_get_lexeme(lexer, lexeme_type, 0);
  if (lexeme_type == LEXEME_FRACTION_BEGIN_DIGIT) {
    tok = create_token(TOK_FLOAT, lexeme, lexer->line);
  } else if (lexeme_type == LEXEME_DIGIT) {
    for (const char* cp_lexeme = lexeme; *cp_lexeme != '\0'; cp_lexeme++) {
      if (*cp_lexeme == '.') {
        return create_token(TOK_FLOAT, lexeme, lexer->line);
      }
    }
    tok = create_token(TOK_INT, lexeme, lexer->line);
  } else {
    fprintf(stderr, "unrecognized digit type to get\n");
  }

  return tok;
}

static token_t* lexer_get_string(lexer_t* lexer, int str_sur)
{
  const char* lexeme = lexer_get_lexeme(lexer, LEXEME_STRING, str_sur);
  return create_token(TOK_STRING, lexeme ? lexeme : "", lexer->line);
}

/* custom append function for string */
static inline void lexeme_append(char** lexeme, size_t* l_size, char c)
{
  (*l_size)++;
  *lexeme = SEAL_REALLOC(*lexeme, (*l_size + 1) * sizeof(char));
  (*lexeme)[*l_size - 1] = c;
  (*lexeme)[*l_size] = '\0';
}

/* get lexeme */
static char* lexer_get_lexeme(lexer_t* lexer, int lexeme_type, int str_sur)
{
  char* lexeme  = NULL;
  size_t l_size = 0;

  if (lexeme_type != LEXEME_STRING) {
    lexeme_append(&lexeme, &l_size, lexer_peek_offset(lexer, -1));
  }

  char c;
  switch (lexeme_type) {
    case LEXEME_ID:
      while (lexer_is_alnum_((c = lexer_peek(lexer)))) {
        lexer_advance(lexer);
        lexeme_append(&lexeme, &l_size, c);
      }
      break;
    case LEXEME_DIGIT:
      while (lexer_is_digit((c = lexer_peek(lexer)))) {
        lexer_advance(lexer);
        lexeme_append(&lexeme, &l_size, c);
      }
      if (lexer_peek(lexer) == '.') {
        lexeme_append(&lexeme, &l_size, lexer_advance(lexer));
      }
      while (lexer_is_digit((c = lexer_peek(lexer)))) {
        lexer_advance(lexer);
        lexeme_append(&lexeme, &l_size, c);
      }
      break;
    case LEXEME_FRACTION_BEGIN_DIGIT:
      while (lexer_is_digit((c = lexer_peek(lexer)))) {
        lexer_advance(lexer);
        lexeme_append(&lexeme, &l_size, c);
      }
      break;
    case LEXEME_STRING:
      while ((c = lexer_peek(lexer)) != '\n' && c != str_sur) {
        lexer_advance(lexer);
        if (c == '\\') {
          char esc_c = lexer_advance(lexer);
          switch (esc_c) {
            case '\\':
              break;
            case 'n':
              c = '\n';
              break;
            case 't':
              c = '\t';
              break;
            case '\'':
              c = '\'';
              break;
            case '\"':
              c = '\"';
              break;
            case 'b':
              c = '\b';
              break;
            default: {
              char err[ERR_LEN];
              sprintf(err, "invalid escape sequence '\\%c'", esc_c);
              lexer_error(lexer, err, 0);
            }
          }
        }
        lexeme_append(&lexeme, &l_size, c);
      }
      if (c != str_sur) {
        lexer_error(lexer, "unterminated string", 0);
      }
      lexer_advance(lexer);
      break;
  }

  return lexeme;
}

static void lexer_ignore_comment(lexer_t* lexer, int comment_type)
{
  if (comment_type == SINGLE_LINE_COMMENT) {
    while (!lexer_is_end(lexer) && lexer_peek(lexer) != '\n')
      lexer_advance(lexer);
  } else if (comment_type == MULTILINE_COMMENT) {
    int start_line = lexer->line;
    while (!lexer_is_end(lexer) && (lexer_peek(lexer) != '*' || lexer_peek_offset(lexer, 1) != '/'))
      if (lexer_advance(lexer) == '\n') lexer->line++;

    if (!lexer_match(lexer, '*') || !lexer_match(lexer, '/')) {
      char err[ERR_LEN];
      sprintf(err, "unclosed comment block");
      lexer_error(lexer, err, start_line);
    }

    if ((lexer->line != start_line || !lexer->encountered_word) && lexer->paren_stack_ptr == 0)
      lexer->token_after_comment = true;
  }
}
