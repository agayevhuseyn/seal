#ifndef IO_H
#define IO_H

#include <stdbool.h>

bool check_if_seal_file(const char* path);
const char* read_file(const char* path);

#endif
