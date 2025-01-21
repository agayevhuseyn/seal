#include "gc.h"
#include <stdlib.h>
#include <stdio.h>

bool gc_free_node(ast_t* node)
{
  if (node->ref_counter > 0 || node->is_static) return false;
  switch (node->type) {
    case AST_INT:
      free(node);
      break;
    case AST_FLOAT:
      free(node);
      break;
    case AST_STRING:
      free(node->string.val);
      free(node);
      break;
    case AST_LIST:
      for (int i = 0; i < node->list.mem_size; i++) {
        node->list.mems[i]->ref_counter--;
        gc_free_node(node->list.mems[i]);
      }
      free(node->list.mems);
      free(node);
      break;
    case AST_OBJECT:
      for (int i = 0; i < node->object.field_size; i++) {
        node->object.field_vars[i]->ref_counter--;
        gc_free_node(node->object.field_vars[i]);
      }
      free(node->object.field_vars);
      free(node);
      break;
    default:
      break;
  }

  return true; // freed successfully
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
  free(var);
}

void gc_free_visited_args(ast_t** args, size_t arg_size)
{
  return;
  for (int i = 0; i < arg_size; i++) {
    gc_free_node(args[i]);
  }
}

// tracking nodes

void gc_add_tracked_node(gc_t* gc, ast_t* node)
{
  if (gc->tracked_node_size++ > 10000) {
    printf("LIMIT EXCEEDED\n");
    exit(1);
  }
  gc->tracked_nodes = realloc(gc->tracked_nodes, gc->tracked_node_size * sizeof(ast_t*));
  gc->tracked_nodes[gc->tracked_node_size - 1] = node;
}

void gc_flush_garbage(gc_t* gc)
{
  for (int i = 0; i < gc->tracked_node_size; i++) {
    if (gc->tracked_nodes[i] == NULL) continue;
    if (gc_free_node(gc->tracked_nodes[i])) {
      gc->tracked_nodes[i] = NULL;
    }
  }
}
