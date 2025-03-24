#ifndef SEAL_LIBDEF_H
#define SEAL_LIBDEF_H

#include "seal.h"

#include "ast.h"
#include "sealtypes.h"

void seal_func_err(const char* libname, const char* func_name, const char* err);

void seal_check_args(const char* libname,
                     const char* func_name,
                     seal_type expected_types[],
                     size_t type_size,
                     sealobj* args[],
                     size_t arg_size);

sealobj* seal_get_obj_mem(sealobj* obj,
                          const char* mem_name,
                          seal_type type,
                          const char* libname,
                          const char* func_name);

sealobj* seal_create_object(const char* field_names[], sealobj* field_vals[], size_t field_size);

#endif
