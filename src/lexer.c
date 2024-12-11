#include "lexer.h"
#include "token.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

lexer_t* init_lexer(const char* src)
{
  lexer_t* lexer = calloc(1, sizeof(lexer_t));

  lexer->src = src;
  lexer->src_size = strlen(src);
  lexer->tokens = (void*)0;
  lexer->token_size = 0;
  lexer->i = 0;
  lexer->line = 1;
  lexer->encountered_word = false;
  lexer->cur_indent = 0;
  lexer->prev_indent = 0;

  return lexer;
}

static inline void lexer_error(lexer_t* lexer, const char* msg)
{
  printf("Lexer-> Error at line: %u, %s\n", lexer->line, msg);
  exit(1);
}

void lexer_collect_tokens(lexer_t* lexer)
{
  while (!lexer_is_end(lexer)) {
    lexer_collect_token(lexer);
  }
  lexer_add_token(lexer, init_token(TOK_EOF, "\0", lexer->line));
}

void lexer_collect_token(lexer_t* lexer)
{
  char c = lexer_advance(lexer);

  switch (c) {
    case '\\':
      while (lexer_advance(lexer) != '\n');
      lexer->line++;
      break;
    case '(':
      lexer_add_token(lexer, init_token(TOK_LPAREN, "(", lexer->line));
      break;
    case ')':
      lexer_add_token(lexer, init_token(TOK_RPAREN, ")", lexer->line));
      break;
    case '[':
      lexer_add_token(lexer, init_token(TOK_LBRACK, "[", lexer->line));
      break;
    case ']':
      lexer_add_token(lexer, init_token(TOK_RBRACK, "]", lexer->line));
      break;
    case '{':
      lexer_add_token(lexer, init_token(TOK_LBRACE, "{", lexer->line));
      break;
    case '}':
      lexer_add_token(lexer, init_token(TOK_RBRACE, "}", lexer->line));
      break;
    case '=':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOK_EQ, "==", lexer->line));
      }
      else
        lexer_add_token(lexer, init_token(TOK_ASSIGN, "=", lexer->line));
      break;
    case ':':
      if (lexer_peek(lexer) == ':') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOK_DCOLON, "::", lexer->line));
      }
      else
        lexer_add_token(lexer, init_token(TOK_COLON, ":", lexer->line));
      break;
    case '!':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOK_NE, "!=", lexer->line));
      }
      else
        lexer_add_token(lexer, init_token(TOK_NOT, "!", lexer->line));
      break;
    case '>':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOK_GE, ">=", lexer->line));
      }
      else
        lexer_add_token(lexer, init_token(TOK_GT, ">", lexer->line));
      break;
    case '<':
      if (lexer_peek(lexer) == '=') {
        lexer_advance(lexer);
        lexer_add_token(lexer, init_token(TOK_LE, "<=", lexer->line));
      }
      else
        lexer_add_token(lexer, init_token(TOK_LT, "<", lexer->line));
      break;
    case '+':
      lexer_add_token(lexer, init_token(TOK_PLUS, "+", lexer->line));
      break;
    case '-':
      lexer_add_token(lexer, init_token(TOK_MINUS, "-", lexer->line));
      break;
    case '*':
      lexer_add_token(lexer, init_token(TOK_MUL, "*", lexer->line));
      break;
    case '/':
      lexer_add_token(lexer, init_token(TOK_DIV, "/", lexer->line));
      break;
    case '%':
      lexer_add_token(lexer, init_token(TOK_MOD, "%", lexer->line));
      break;
    case '.':
      if (isdigit(lexer_peek(lexer))) {
        lexer_get_digit(lexer, false);
      } else {
        lexer_add_token(lexer, init_token(TOK_DOT, ".", lexer->line));
      }
      break;
    case ',':
      lexer_add_token(lexer, init_token(TOK_COMMA, ",", lexer->line));
      break;
    case ';':
      lexer_add_token(lexer, init_token(TOK_SEMI, ";", lexer->line));
      break;
    case '\t':
      if (!lexer->encountered_word) {
        lexer->cur_indent++;
      }
      break;
    case '\n':
      if (!lexer->encountered_word) {
        lexer->cur_indent = 0;
      } else {
        lexer_add_token(lexer, init_token(TOK_NEWL, "\\n", lexer->line));
      }
      lexer->line++;
      break;
    case ' ':
      break;
    case '\"':
      lexer_get_string(lexer);
      break;
    case '_':
      lexer_get_id(lexer);
      break;
    default: {
      if (isalpha(c)) {
        lexer_get_id(lexer);
        break;
      } else if (isdigit(c)) {
        lexer_get_digit(lexer, true);
        break;
      }
      char msg[128];
      sprintf(msg, "unexpected character: \"%c\"", c);
      lexer_error(lexer, msg);
    }
    break;
  }
}

char lexer_advance(lexer_t* lexer)
{
  return lexer->src[lexer->i++];
}

char lexer_peek(lexer_t* lexer)
{
  return lexer->src[lexer->i];
}

char lexer_peek_offset(lexer_t* lexer, int offset)
{
  int i = lexer->i + offset;
  return i < lexer->src_size && i >= 0 ? lexer->src[i] : '\0';
}

char lexer_eat(lexer_t* lexer, char expected)
{
  if (lexer_peek(lexer) != expected) {
    char msg[128];
    sprintf(msg, "expected %c, got %c", expected, lexer_peek(lexer));
    lexer_error(lexer, msg);
  }
  return lexer_advance(lexer);
}

bool lexer_is_end(lexer_t* lexer)
{
  return lexer->i >= lexer->src_size;
}

void lexer_add_token(lexer_t* lexer, token_t* token)
{
  switch (token->type) {
    case TOK_NEWL:
      if (lexer->encountered_word) {
        lexer->encountered_word = false;
        lexer->prev_indent = lexer->cur_indent;
        lexer->cur_indent = 0;
      }
      break;
    case TOK_INDENT:
    case TOK_DEDENT:
      break;
    default:
      if (!lexer->encountered_word) {
        if (lexer->cur_indent > lexer->prev_indent) {
          for (int i = 0; i < lexer->cur_indent - lexer->prev_indent; i++) {
            lexer_add_token(lexer, init_token(TOK_INDENT, "indent", lexer->line));
          }
        } else if (lexer->cur_indent < lexer->prev_indent) {
          for (int i = 0; i < lexer->prev_indent - lexer->cur_indent; i++) {
            lexer_add_token(lexer, init_token(TOK_DEDENT, "dedent", lexer->line));
            lexer_add_token(lexer, init_token(TOK_NEWL, "\\n", lexer->line));
          }
        }
        lexer->encountered_word = true;
      }
    break;
  }
  lexer->token_size++;
  lexer->tokens = realloc(lexer->tokens, lexer->token_size * sizeof(token_t*));
  lexer->tokens[lexer->token_size - 1] = token;
}

void lexer_get_id(lexer_t* lexer)
{
  int len = 1;
  char* s = calloc(len + 1, sizeof(char));
  s[0] = lexer_peek_offset(lexer, -1);
  s[1] = '\0';

  char c;
  while (isalnum(c = lexer_peek(lexer)) || c == '_') {
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';   
    lexer_advance(lexer);
  }

  if (strcmp(s, "var") == 0) lexer_add_token(lexer, init_token(TOK_VAR, s, lexer->line));
  else if (strcmp(s, "object") == 0) lexer_add_token(lexer, init_token(TOK_OBJECT, s, lexer->line));
  else if (strcmp(s, "define") == 0) lexer_add_token(lexer, init_token(TOK_DEFINE, s, lexer->line));
  else if (strcmp(s, "and") == 0) lexer_add_token(lexer, init_token(TOK_AND, s, lexer->line));
  else if (strcmp(s, "or") == 0) lexer_add_token(lexer, init_token(TOK_OR, s, lexer->line));
  else if (strcmp(s, "not") == 0) lexer_add_token(lexer, init_token(TOK_NOT, s, lexer->line));
  else if (strcmp(s, "true") == 0) lexer_add_token(lexer, init_token(TOK_TRUE, s, lexer->line));
  else if (strcmp(s, "false") == 0) lexer_add_token(lexer, init_token(TOK_FALSE, s, lexer->line));
  else if (strcmp(s, "null") == 0) lexer_add_token(lexer, init_token(TOK_NULL, s, lexer->line));
  else if (strcmp(s, "return") == 0) lexer_add_token(lexer, init_token(TOK_RETURN, s, lexer->line));
  else if (strcmp(s, "if") == 0) lexer_add_token(lexer, init_token(TOK_IF, s, lexer->line));
  else if (strcmp(s, "else") == 0) lexer_add_token(lexer, init_token(TOK_ELSE, s, lexer->line));
  else if (strcmp(s, "while") == 0) lexer_add_token(lexer, init_token(TOK_WHILE, s, lexer->line));
  else if (strcmp(s, "for") == 0) lexer_add_token(lexer, init_token(TOK_FOR, s, lexer->line));
  else if (strcmp(s, "skip") == 0) lexer_add_token(lexer, init_token(TOK_SKIP, s, lexer->line));
  else if (strcmp(s, "stop") == 0) lexer_add_token(lexer, init_token(TOK_STOP, s, lexer->line));
  else if (strcmp(s, "include") == 0) lexer_add_token(lexer, init_token(TOK_INCLUDE, s, lexer->line));
  else if (strcmp(s, "in") == 0) lexer_add_token(lexer, init_token(TOK_IN, s, lexer->line));
  else lexer_add_token(lexer, init_token(TOK_ID, s, lexer->line));
}

void lexer_get_digit(lexer_t* lexer, bool has_whole)
{
  Token_Type type = TOK_INT;
  int len = 1;
  char* s = calloc(len + 1, sizeof(char));
  s[0] = lexer_peek_offset(lexer, -1);
  s[1] = '\0';

  char c;
  if (has_whole) {
    while (isdigit(c = lexer_peek(lexer))) {
      len++;
      s = realloc(s, (len + 1) * sizeof(char));
      s[len - 1] = c;
      s[len] = '\0';   
      lexer_advance(lexer);
    }
    if ((c = lexer_peek(lexer)) == '.') {
      len++;
      s = realloc(s, (len + 1) * sizeof(char));
      s[len - 1] = c;
      s[len] = '\0';   
      lexer_advance(lexer);
      type = TOK_FLOAT;
    } 
    while (isdigit(c = lexer_peek(lexer))) {
      len++;
      s = realloc(s, (len + 1) * sizeof(char));
      s[len - 1] = c;
      s[len] = '\0';   
      lexer_advance(lexer);
    }
  } else {
    type = TOK_FLOAT;
    while (isdigit(c = lexer_peek(lexer))) {
      len++;
      s = realloc(s, (len + 1) * sizeof(char));
      s[len - 1] = c;
      s[len] = '\0';   
      lexer_advance(lexer);
    }
  }

  lexer_add_token(lexer, init_token(type, s, lexer->line));
}

void lexer_get_string(lexer_t* lexer)
{
  int len = 0;
  char* s = calloc(len + 1, sizeof(char));
  s[0] = '\0';

  char c;
  while ((c = lexer_peek(lexer)) != '\n' && c != '\"') {
    len++;
    s = realloc(s, (len + 1) * sizeof(char));
    s[len - 1] = c;
    s[len] = '\0';   
    lexer_advance(lexer);
  }
  
  if (c != '\"') {
    char msg[128];
    sprintf(msg, "unterminated string \"%s\"", s);
    lexer_error(lexer, msg);
  }
  lexer_eat(lexer, '\"');

  lexer_add_token(lexer, init_token(TOK_STRING, s, lexer->line));
}
