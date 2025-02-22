#ifndef SEAL_GC_H
#define SEAL_GC_H

#include "seal.h"

#include "ast.h"
#include "list.h"
#include "scope.h"

#define GC_DEBUG 0

typedef struct {
  list_t* tracked;
  list_t* ret_tracked;
} gc_t;

bool gc_free_ast(ast_t* node);

static inline void gc_retain(ast_t* node)
{
  node->ref_counter++;
}

static inline void gc_release(ast_t* node)
{
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
#if GC_DEBUG
  printf("GC LEN: %d\n", len);
#endif
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
  if (tracked->is_static || tracked->type == AST_RETURNED_VAL) return;
  // last way
  list_iterate(gc->tracked) {
    if (it->val == tracked) {
      return;
    }
  }
  list_push(&gc->tracked, tracked);
}

static void gc_track_ret(gc_t* gc, ast_t* tracked)
{
  if (tracked->is_static || tracked->type != AST_RETURNED_VAL) return;
  // last way
  list_iterate(gc->ret_tracked) {
    if (it->val == tracked) {
      return;
    }
  }
  tracked->ret_val_life = MAX_RET_TIME;
  list_push(&gc->ret_tracked, tracked);
}

static void gc_flush_ret(gc_t* gc)
{
  // READ AND ANALYZE THIS CODE
  list_t **current = &gc->ret_tracked;
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
    if (--entry->val->ret_val_life <= 0) {
      gc_release(entry->val->returned_val.val);
      *current = entry->next;
      SEAL_FREE(entry->val);
      SEAL_FREE(entry);
    } else {
      current = &entry->next;
    }
    len++;
  }
#if GC_DEBUG
  printf("GC LEN: %d\n", len);
#endif
}

static inline void gc_print(gc_t* gc)
{
  int len = 0;
  list_iterate(gc->tracked) {
    printf("ref counter: %d:\n", it->val->ref_counter);
    print_ast(it->val);
    len++;
  }
  if (len == 0) {
    printf("GC is empty\n");
  }
}

static inline void gc_print_ret(gc_t* gc)
{
  int len = 0;
  list_iterate(gc->ret_tracked) {
    printf("static: %d, ref counter: %d:\n", it->val->is_static, it->val->ref_counter);
    print_ast(it->val);
    len++;
  }
  if (len == 0) {
    printf("GC is empty\n");
  }
}

#endif
