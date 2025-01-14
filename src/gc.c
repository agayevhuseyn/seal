#include "gc.h"
#include <stdlib.h>
#include <stdio.h>

void gc_free_node(ast_t* node)
{
  //printf("%s, ref: %d\n", ast_name(node), node->ref_counter);
  if (node->type == AST_LIST) {
    printf("YESSLIST\n");
  }
  if (node->ref_counter > 0 || node->is_static) return;
  switch (node->type) {
    case AST_INT:
      printf("%d IS BEING FREED\n", node->integer.val);
    case AST_FLOAT:
      free(node);
      break;
    case AST_STRING:
      free(node->string.val);
      free(node);
      break;
    case AST_LIST:
      for (int i = 0; i < node->list.mem_size; i++) {
        printf("%d ref: %d\n", node->list.mems[i]->integer.val, node->list.mems[i]->ref_counter);
        gc_free_node(node->list.mems[i]);
      }
      free(node);
      printf("FREED\n");
      break;
    case AST_OBJECT:
      for (int i = 0; i < node->object.field_size; i++) {
        gc_free_node(node->object.field_vars[i]);
      }
      free(node->object.field_vars);
      free(node);
      break;
    default:
      break;
  }
}

void gc_free_scope(scope_t* scope)
{
  for (int i = 0; i < scope->var_size; i++) {
    gc_free_var(scope->vars[i]);
  }
  free(scope->vars);
  free(scope);
}

void gc_free_var(ast_t* var)
{
  if (var->variable.val->ref_counter > 0)
    var->variable.val->ref_counter--;
  gc_free_node(var->variable.val);
  free(var);
}

void gc_free_visited_args(ast_t** args, size_t arg_size)
{
  for (int i = 0; i < arg_size; i++) {
    gc_free_node(args[i]);
  }
}

// tracking nodes

void gc_add_tracked_node(gc_t* gc, ast_t* node)
{
  if (!gc->tracked_node) {
    gc->tracked_node = calloc(1, sizeof(tracked_node_t));
    gc->tracked_node->node = node;
    return;
  }
  tracked_node_t* tracked = gc->tracked_node;
  while (tracked->next) tracked = tracked->next;
  tracked->next = calloc(1, sizeof(tracked_node_t));
  tracked->next->node = node;
}

void gc_flush_garbage(gc_t* gc)
{
  tracked_node_t* head = gc->tracked_node, *prev = NULL;
  while (gc->tracked_node->next) {
    if (gc->tracked_node->node->ref_counter == 0) {
      if (prev) {
        prev->next = gc->tracked_node->next;
        gc_free_node(gc->tracked_node->node);
        free(gc->tracked_node);
        gc->tracked_node = prev;
      } else {
        tracked_node_t* next = gc->tracked_node->next;
        gc_free_node(gc->tracked_node->node);
        free(gc->tracked_node);
        gc->tracked_node = next;
        head = next;
        continue;
      }
    }
    prev = gc->tracked_node;
    gc->tracked_node = gc->tracked_node->next;
  }
  gc->tracked_node = head;
}
