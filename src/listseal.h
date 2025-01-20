#ifndef LIST_H
#define LIST_H

#include "ast.h"

void list_push(ast_t* list, ast_t* element);
ast_t* list_pop(ast_t* list);

#endif
