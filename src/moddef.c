#include "moddef.h"


void seal_parse_args(const char *mod_name,
                    const char *func_name,
                    seal_byte argc,
                    svalue_t *argv,
                    seal_byte typec,
                    int *typev,
                    ...)
{
  if (argc != typec)
    MOD_ERROR(mod_name, func_name, "expected %d argument%s, got %d", typec, typec != 1 ? "s" : "", argc);

  va_list vargs;
  va_start(vargs, typev);

  for (int i = 0; i < typec; i++) {
    int type = VAL_TYPE(argv[i]);
    if (!(type & typev[i])) {
      MOD_ERROR(mod_name,
                func_name,
                "expected argument %d to be \'%s\', got \'%s\'",
                i,
                seal_type_name(typev[i]),
                seal_type_name(type));
    }

    svalue_t *ps = va_arg(vargs, svalue_t*);

    *ps = argv[i];
  }

  va_end(vargs);
}

svalue_t seal_map_get_field(const char *libname, const char *func_name, svalue_t *map, const char *key)
{
  return SEAL_VALUE_NULL;
}
