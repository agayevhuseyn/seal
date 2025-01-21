#ifndef GC_H
#define GC_H

#include "ast.h"
#include "scope.h"

typedef struct {
  ast_t** tracked_nodes;
  size_t tracked_node_size;
} gc_t;

bool gc_free_node(ast_t*);
void gc_free_scope(scope_t*);
void gc_free_var(ast_t*);
void gc_free_visited_args(ast_t**, size_t);

// tracking nodes
void gc_add_tracked_node(gc_t*, ast_t*);
void gc_flush_garbage(gc_t*);

#endif
