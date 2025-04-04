#ifndef SEAL_BUILTIN_H
#define SEAL_BUILTIN_H

#include "seal.h"

#include "ast.h"
#include "gc.h"
#include "sealtypes.h"

static inline ast_t* func_error(const char* name, ast_t* fcall, const char* err)
{
  fprintf(stderr, "seal: at line: %d\nerror: %s in function \'%s\'\n", fcall->line, err, name);
  exit(1);
  return ast_null();
}

static void check_arg_size(ast_t* fcall_node, const char* func_name, size_t arg_size, size_t type_size) {
  if (arg_size != type_size) {
    char err[ERR_LEN];
    sprintf(err, "expected %lu arg%s, got %lu", type_size, type_size > 1 ? "s" : "\0", arg_size);
    func_error(func_name, fcall_node, err);
  }
}
static void check_args(ast_t* fcall_node,
                       const char* func_name,
                       int expected_types[],
                       ast_t* args[],
                       size_t arg_size)
{
  for (int i = 0; i < arg_size; i++) {
    sealobj* arg = args[i];
    switch (expected_types[i]) {
      case SEAL_NULL:
        if (IS_SEAL_NULL(arg)) continue;
        break;
      case SEAL_INT:
        if (IS_SEAL_INT(arg)) continue;
        break;
      case SEAL_FLOAT:
        if (IS_SEAL_FLOAT(arg)) continue;
        break;
      case SEAL_STRING:
        if (IS_SEAL_STRING(arg)) continue;
        break;
      case SEAL_BOOL:
        if (IS_SEAL_BOOL(arg)) continue;
        break;
      case SEAL_LIST:
        if (IS_SEAL_LIST(arg)) continue;
        break;
      case SEAL_OBJECT:
        if (IS_SEAL_OBJECT(arg)) continue;
        break;
      case SEAL_NUMBER:
        if (IS_SEAL_NUMBER(arg)) continue;
        break;
      case SEAL_ITERABLE:
        if (IS_SEAL_ITERABLE(arg)) continue;
        break;
      case SEAL_ANY:
        if (IS_SEAL_ANY(arg)) continue;
        break;
    }

    char err[ERR_LEN];
    sprintf(err,
            "expected arg %d to be %s, got %s",
            i + 1,
            seal_type_name(expected_types[i]),
            seal_type_name(arg->type));
    func_error(func_name, fcall_node, err);
  }
}


static ast_t* builtin_writeln(ast_t* args[], size_t arg_size, bool is_main, bool is_single)
{
  for (int i = 0; i < arg_size; i++) {
    ast_t* arg = args[i];
    switch (arg->type) {
      case AST_NULL:
        printf("%s", "null");
        goto space;
      case AST_INT:
        printf("%lld", arg->integer.val);
        goto space;
      case AST_FLOAT:
        printf("%f", arg->floating.val);
        goto space;
      case AST_STRING:
        if (is_single)
          printf("%s", arg->string.val);
        else
          printf("\'%s\'", arg->string.val);
        goto space;
      case AST_BOOL:
        printf("%s", arg->boolean.val ? "true" : "false");
        goto space;
      case AST_LIST:
        printf("[");
        for (int j = 0; j < arg->list.mem_size; j++) {
          builtin_writeln(&arg->list.mems[j], 1, false, false);
          if (j < arg->list.mem_size - 1) {
            printf(", ");
          }
        }
        printf("]");
        goto space;
      case AST_OBJECT:
        printf("{");
        for (int j = 0; j < arg->object.field_size; j++) {
          printf("%s = ", arg->object.field_names[j]);
          builtin_writeln(&arg->object.field_vals[j], 1, false, false);
          if (j < arg->object.field_size - 1) {
            printf(", ");
          }
        }
        printf("}");
        goto space;
      default:
        printf("UNRECOGNIZED TYPE: \'%s\'", hast_type_name(arg->type));
    }
    space:
    if (is_main) printf(" ");
  }
  if (is_main) printf("\n");
  return ast_null();
}

static ast_t* builtin_readln(ast_t* arg)
{
  if (arg) builtin_writeln(&arg, 1, false, true);
  ast_t* line = create_ast(AST_STRING);
  line->string.val = (char*)SEAL_CALLOC(1, sizeof(char));
  
  size_t len = 0;
  char c;
  while ((c = getchar()) != '\n' && c != EOF) {
    len++;
    line->string.val = (char*)SEAL_REALLOC(line->string.val, (len + 1) * sizeof(char));
    line->string.val[len - 1] = c;
  }
  line->string.val[len] = '\0';

  return line;
}

static inline ast_t* builtin_len(ast_t* arg)
{
  ast_t* len = create_ast(AST_INT);
  len->integer.val = arg->type == AST_STRING ? strlen(arg->string.val) : arg->list.mem_size;
  return len;
}

static inline ast_t* builtin_push(ast_t* args[2])
{
  args[0]->list.mem_size++;
  args[0]->list.mems = (ast_t**)SEAL_REALLOC(args[0]->list.mems, args[0]->list.mem_size * sizeof(ast_t*));
  args[0]->list.mems[args[0]->list.mem_size - 1] = args[1];
  gc_retain(args[1]);
  return ast_null();
}

static inline ast_t* builtin_pop(ast_t* arg, ast_t* fcall)
{
  if (arg->list.mem_size == 0) {
    func_error("pop", fcall, "pop from empty list");
  }
  arg->list.mem_size--;
  ast_t* popped = arg->list.mems[arg->list.mem_size];
  arg->list.mems = (ast_t**)SEAL_REALLOC(arg->list.mems, arg->list.mem_size * sizeof(ast_t*));
  gc_release(popped);
  return popped;
}

static ast_t* builtin_format(ast_t* fcall, ast_t* args[], size_t arg_size)
{
  if (arg_size == 0) {
    func_error("format", fcall, "empty argument");
  }
  if (args[0]->type != AST_STRING) {
    func_error("format", fcall, "first argument must be string");
  }
  const char* formatter = args[0]->string.val;
  char* result = (char*)SEAL_CALLOC(1, sizeof(char));
  int size = 0;
  int arg_counter = 1;
  int arg_format = arg_size - 1;
  for (; *formatter != '\0'; formatter++) {
    if (*formatter != '%' || *formatter == '%' && *(formatter + 1) == '%') {
      size++;
      result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
      result[size - 1] = *formatter;
      result[size] = '\0';
      if (*formatter == '%') formatter++;
    } else {
      if (arg_format == 0) {
        func_error("format", fcall, "more formatter than arguments");
      }
      int len;
      ast_t* arg = args[arg_counter];
      arg_counter++;
      arg_format--;
      switch (arg->type) {
        case AST_NULL: {
          size += 4;
          result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
          strcat(result, "null");
          break;
        }
        case AST_INT: {
          len = snprintf(NULL, 0, "%lld", arg->integer.val);
          size += len;
          result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
          char buffer[len];
          sprintf(buffer, "%lld", arg->integer.val);
          strcat(result, buffer);
          break;
        }
        case AST_FLOAT: {
          len = snprintf(NULL, 0, "%f", arg->floating.val);
          size += len;
          result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
          char buffer[len];
          sprintf(buffer, "%f", arg->floating.val);
          strcat(result, buffer);
          break;
        }
        case AST_STRING: {
          size += strlen(arg->string.val);
          result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
          strcat(result, arg->string.val);
          break;
        }
        case AST_BOOL: {
          const char* val = arg->boolean.val ? "true" : "false";
          size += strlen(val);
          result = (char*)SEAL_REALLOC(result, (size + 1) * sizeof(char));
          strcat(result, val);
          break;
        }
        default: {
          char err[ERR_LEN];
          sprintf(err, "unrecognized type: \'%s\'", hast_type_name(arg->type));
          func_error("format", fcall, err);
        }
        result[size] = '\0';
      }
    }
  }
  if (arg_format > 0) {
    func_error("format", fcall, "too many arguments");
  }

  result[size] = '\0';
  ast_t* res_ast = create_ast(AST_STRING);
  res_ast->string.val = result;
  return res_ast;
}

static ast_t* builtin_fopen(ast_t* args[2])
{
  FILE* file = fopen(args[0]->string.val, args[1]->string.val);
  if (!file) return ast_null();
  ast_t* file_ptr = create_ast(AST_INT);
  file_ptr->integer.val = (Seal_int) file;
  return file_ptr;
}

static ast_t* builtin_fread(ast_t* arg)
{
  FILE* file = (FILE*) arg->integer.val;
  fseek(file, 0, SEEK_END);
  unsigned len = ftell(file);
  char* content = (char*) SEAL_CALLOC(len + 1, sizeof(char));
  fseek(file, 0, SEEK_SET);
  if (content) {
    fread(content, sizeof(char), len, file);
  }
  content[len] = '\0';
  ast_t* res = create_ast(AST_STRING);
  res->string.val = content;
  return res;
}

static ast_t* builtin_fclose(ast_t* arg)
{
  FILE* file = (FILE*) arg->integer.val;
  fclose(file);
  return ast_null();
}

static ast_t* builtin_fwrite(ast_t* args[2])
{
  FILE* file = (FILE*) args[0]->integer.val;
  fprintf(file, "%s", args[1]->string.val);
  return ast_null();
}

static ast_t* builtin_exit(ast_t* arg)
{
  exit(arg->integer.val);
  return ast_null();
}

#endif
