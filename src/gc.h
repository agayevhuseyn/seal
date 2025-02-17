#ifndef SEAL_GC_H
#define SEAL_GC_H

#include "seal.h"

#include "ast.h"
#include "list.h"
#include "scope.h"

typedef struct {
  list_t* tracked;
} gc_t;

bool gc_free_ast(ast_t* node);

static inline void gc_retain(ast_t* node)
{
  if (node->is_static) return;
  node->ref_counter++;
}

static inline void gc_release(ast_t* node)
{
  if (node->is_static) return;
  node->ref_counter--;
}

static void gc_flush(gc_t* gc)
{
  // READ AND ANALYZE THIS CODE
  list_t **current = &gc->tracked;
  int len = 0;
  while (*current) {
    list_t *entry = *current;
    //printf("$type: %s: %d\n", hast_type_name(entry->val->type), entry->val->type);
    //printf("$$$$$ %s\n", entry->val->string.val);
    /*
    printf("NAME: %s\n", hast_type_name(entry->val->type));
    print_ast(entry->val);
    printf("\tREF: %d\n", entry->val->ref_counter);
    */
    if (gc_free_ast(entry->val)) {
      *current = entry->next;
      SEAL_FREE(entry);
    } else {
      current = &entry->next;
    }
    len++;
  }
  printf("GC LEN: %d\n", len);
}

// TODO IMPLEMENT
static void gc_free_scope(scope_t* scope)
{
  /*
  list_iterate(scope->var_list) {
    ast_t* var = it->val;
    gc_release(var->variable.val);
    SEAL_FREE(var);

  }
  */
  list_t **current = &scope->var_list;
  while (*current) {
    list_t *entry = *current;
    ast_t* var = entry->val;
    gc_release(var->variable.val);
    SEAL_FREE(var);
    *current = entry->next;
    SEAL_FREE(entry);
  }
}

static void gc_track(gc_t* gc, ast_t* tracked)
{
  if (tracked->is_static) return;
  // last way
  list_iterate(gc->tracked) {
    if (it->val == tracked) {
      return;
    }
  }
  list_push(&gc->tracked, tracked);
}

#endif
