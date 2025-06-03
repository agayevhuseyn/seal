#include "gc.h"

#define IS_ALLOCATED(s) (IS_LIST(s) || (IS_STRING(s) && !s.as.string->is_static))

inline void gc_decref(svalue_t s)
{
  if (!IS_ALLOCATED(s))
    return;
  //printf("DEC REF COUNT OF \'%s\': %d\n", AS_STRING(s), s.as.string->ref_count);
  switch (VAL_TYPE(s)) {
  case SEAL_STRING:
    if (--s.as.string->ref_count <= 0) {
      //printf("FREEING %s\n", AS_STRING(s));
      free((char*)(s.as.string->val));
      free((s.as.string));
      s.as.string = NULL;
    }
    break;
  case SEAL_LIST:
    if (--s.as.list->ref_count <= 0) {
      for (int i = 0; i < s.as.list->size; i++) {
        gc_decref(s.as.list->mems[i]);
      }
      free(s.as.list->mems);
      free(s.as.list);
    }
    break;
  }
}
void gc_decref_nofree(svalue_t s)
{
  if (!IS_ALLOCATED(s))
    return;
  switch (VAL_TYPE(s)) {
  case SEAL_STRING:
    --s.as.string->ref_count;
    break;
  case SEAL_LIST:
    --s.as.list->ref_count;
    break;
  }
}
inline void gc_incref(svalue_t s)
{
  if (!IS_ALLOCATED(s))
    return;
  switch (VAL_TYPE(s)) {
  case SEAL_STRING:
    s.as.string->ref_count++;
    break;
  case SEAL_LIST:
    s.as.list->ref_count++;
    break;
  }
}
