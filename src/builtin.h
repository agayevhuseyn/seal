#ifndef BUILTIN_H
#define BUILTIN_H

#include "ast.h"

ast_t* builtin_write(ast_t**, size_t);
ast_t* builtin_len(ast_t**, size_t);

#endif
