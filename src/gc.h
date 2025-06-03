#ifndef SEAL_GC_H
#define SEAL_GC_H

#include "seal.h"
#include "sealtypes.h"

void gc_decref(svalue_t);
void gc_decref_nofree(svalue_t);
void gc_incref(svalue_t);

#endif /* SEAL_GC_H */
