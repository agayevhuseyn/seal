#include "list.h"
#include <stdio.h>

ast_t* list_push(ast_t* list, ast_t* element)
{
  list->list.mem_size++;
  list->list.mems = realloc(list->list.mems, list->list.mem_size * sizeof(ast_t*));
  list->list.mems[list->list.mem_size - 1] = element;

  return ast_noop();
}

ast_t* list_pop(ast_t* list)
{
  if (list->list.mem_size == 0) {
    printf("Empty list\n");
    return ast_noop();
  }
  ast_t* popped = list->list.mems[list->list.mem_size - 1];
  list->list.mem_size--;
  list->list.mems = realloc(list->list.mems, list->list.mem_size * sizeof(ast_t*));

  return popped;
}
