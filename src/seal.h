#ifndef SEAL_H
#define SEAL_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define SEAL_MALLOC(size)       malloc(size)
#define SEAL_CALLOC(n, size)    calloc(n, size)
#define SEAL_REALLOC(ptr, size) realloc(ptr, size)
#define SEAL_FREE(ptr)          free(ptr)

#define ERR_LEN 256

#define MAX_RET_TIME 3

typedef long long Seal_int;
typedef double    Seal_float;

#define RESERVED_NAMES ( \
    (char*[]) { "writeln",\
                "readln",\
                "format",\
                "open",\
                "write",\
                "read",\
                "close",\
                "len",\
                "push",\
                "pop",\
                "assert",\
                "typeof",\
                "int",\
                "float",\
                "str",\
                "bool"})

#define RESERVED_NAMES_SIZE (sizeof(RESERVED_NAMES) / sizeof(RESERVED_NAMES[0]))

#endif
