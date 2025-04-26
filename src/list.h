#ifndef SEAL_LIST_H
#define SEAL_LIST_H

#include "seal.h"

#include "ast.h"

#define list_is_empty(l) ( \
    l->next == NULL)

#define list_iterate(l) \
  for (list_t* it = l; it; it = it->next)

typedef struct list {
  struct list* next;
  ast_t* val;
} list_t;

static inline list_t* create_list(ast_t* val)
{
  list_t* list = (list_t*)SEAL_CALLOC(1, sizeof(list_t));
  list->val = val;
  list->next = NULL;
  return list;
}

static inline void list_push(list_t** head, ast_t* tail_val)
{
  if (!*head) {
    *head = create_list(tail_val);
    return;
  }
  list_t* cur = *head;
  while (cur->next) cur = cur->next;
  cur->next = create_list(tail_val);
}

#endif
