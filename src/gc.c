#include "gc.h"

bool gc_free_ast(ast_t* node)
{
  // temporary solution
  //if (node->type < AST_NULL || node->type > AST_VARIABLE) return true;
  if (node->is_static || node->ref_counter > 0) {
#if GC_DEBUG
    printf("Skipping node of type %s static: %s, with ref_counter %d\nnode:\n",
           ast_type_name(node->type),
           node->is_static ? "true" : "false",
           node->ref_counter);
    print_ast(node);
#endif
    return false;
  }

  switch (node->type) {
    case AST_INT:
    case AST_FLOAT:
      break;
    case AST_STRING:
      SEAL_FREE(node->string.val);
      break;
    case AST_LIST:
      for (int i = 0; i < node->list.mem_size; i++) {
        ast_t* mem = node->list.mems[i];
        gc_release(mem);
      }
      SEAL_FREE(node->list.mems);
      break;
    case AST_OBJECT:
      for (int i = 0; i < node->object.field_size; i++) {
        ast_t* field = node->object.field_vals[i];
        gc_release(field);
      }
      SEAL_FREE(node->object.field_vals);
      break;
    default:
      printf("seal: cannot free \'%s\'\n", ast_type_name(node->type));
      exit(1);
  }
  SEAL_FREE(node);

  return true;
}
