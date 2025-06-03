#include "gc.h"

inline void gc_decref(svalue_t s)
{
  if (s.type != SEAL_STRING)
    return;
  if (s.as.string == NULL)
    return;
  if (s.as.string->is_static)
    return;
  //printf("DEC REF COUNT OF \'%s\': %d\n", AS_STRING(s), s.as.string->ref_count);
  if (--s.as.string->ref_count <= 0) {
    //printf("FREEING %s\n", AS_STRING(s));
    free((char*)(s.as.string->val));
    free((s.as.string));
    s.as.string = NULL;
  }
}
inline void gc_incref(svalue_t s)
{
  if (s.type != SEAL_STRING)
    return;
  if (s.as.string->is_static)
    return;
  //printf("INC REF COUNT OF \'%s\': %d\n", AS_STRING(s), s.as.string->ref_count);
  s.as.string->ref_count++;
}
inline void gc_free(svalue_t s)
{
  if (s.type != SEAL_STRING)
    return;
  if (s.as.string->is_static)
    return;
}
