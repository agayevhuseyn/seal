#ifndef MODDEF_H
#define MODDEF_H

#include "sealtypes.h"
#include "hashmap.h"
#include <stdarg.h>

#define PARAM_TYPES(...) ((seal_type[]) { __VA_ARGS__ })

#define MOD_REGISTER_FUNC(mod, _name, str, _argc, _is_vararg) do { \
  svalue_t func = { \
    .type = SEAL_FUNC, \
    .as.func = { \
      .type = FUNC_BUILTIN, \
      .name = str, \
      .is_vararg = _is_vararg, \
      .as.builtin = { \
        .cfunc = _name, \
        .argc = _argc \
      } \
    } \
  }; \
  hashmap_insert(AS_MOD(mod)->globals, str, func); \
} while (0)

#define MOD_ERROR(mod_name, func_name, ...) do { \
  fprintf(stderr, "seal: at module \'%s\': function \'%s\'\n", mod_name, func_name); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
  exit(1); \
} while (0)

void seal_parse_args(const char *mod_name,
                    const char *func_name,
                    seal_byte argc,
                    svalue_t *argv,
                    seal_byte typec,
                    int *typev,
                    ...);
svalue_t seal_map_get_field(const char*, const char*, svalue_t*, const char*);

#endif /* MODDEF_H */
