#ifndef SEAL_H
#define SEAL_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define SEAL_MALLOC(size)       malloc(size)
#define SEAL_CALLOC(n, size)    calloc(n, size)
#define SEAL_REALLOC(ptr, size) realloc(ptr, size)
#define SEAL_FREE(ptr)          free(ptr)

#define ERR_LEN 256

typedef long long seal_int;
typedef double    seal_float;

#define RESERVED_NAMES ( \
    (char*[]) { "writeln",\
                "readln",\
                "format",\
                "fopen",\
                "fwrite",\
                "fread",\
                "fclose",\
                "len",\
                "push",\
                "pop",\
                "assert",\
                "int",\
                "float",\
                "str",\
                "bool",\
                "exit"})

#define RESERVED_NAMES_SIZE (sizeof(RESERVED_NAMES) / sizeof(RESERVED_NAMES[0]))

#endif
