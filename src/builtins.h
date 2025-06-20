#ifndef SEAL_BUILTINS_H
#define SEAL_BUILTINS_H

#include "sealconf.h"
#include "sealtypes.h"

svalue_t __seal_print(seal_byte argc, svalue_t* argv);
svalue_t __seal_scan(seal_byte argc, svalue_t* argv);
svalue_t __seal_exit(seal_byte argc, svalue_t* argv);
svalue_t __seal_len(seal_byte argc, svalue_t* argv);
svalue_t __seal_int(seal_byte argc, svalue_t* argv);
svalue_t __seal_float(seal_byte argc, svalue_t* argv);
svalue_t __seal_push(seal_byte argc, svalue_t* argv);
svalue_t __seal_pop(seal_byte argc, svalue_t* argv);

#endif /* SEAL_BUILTINS_H */
