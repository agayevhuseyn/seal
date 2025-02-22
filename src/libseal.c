#include "libseal.h"

#include <dlfcn.h>

static inline void libseal_error()
{
#ifdef _WIN32
  fprintf(stderr, "seal library include error: %lu\n", GetLastError());
#else
  fprintf(stderr, "seal library include error: %s\n", dlerror());
#endif
  exit(1);
}

libseal_t* create_libseal(const char* name, bool has_alias, const char* alias_name)
{
  libseal_t* libseal = SEAL_CALLOC(1, sizeof(libseal_t));
  libseal->name = has_alias ? alias_name : name;

  char path[256];
#ifdef _WIN32
  sprintf(path, "./%s.dll", name);
  libseal->handle = LoadLibrary(path);
  if (!libseal->handle) {
    libseal_error();
  }
#else
  sprintf(path, "./%s.so", name);
  libseal->handle = dlopen(path, RTLD_LAZY);
  if (!libseal->handle) {
    libseal_error();
  }
#endif
  libseal->funcs = NULL;
  libseal->func_names = NULL;
  libseal->func_size = 0;

  // call initlib func in included lib
  const char initlib_fname[] = "_initlib";
#ifdef _WIN32
    FARPROC function = GetProcAddress(libseal->handle, initlib_fname);
    if (!function) {
      fprintf(stderr, "%s function is not found in libseal: %s\n", initlib_fname, libseal->name);
      libseal_error();
    }
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))function;
#else
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))dlsym(libseal->handle, initlib_fname);
    if (!function_ptr) {
      libseal_error();
    }
#endif
  function_ptr(NULL, 0);
  
  return libseal;
}
ast_t* libseal_function_call(libseal_t* libseal, const char* func_name, ast_t** args, size_t arg_size)
{
  for (int i = 0; i < libseal->func_size; i++) {
    char act_fn_name[strlen(func_name) + 2];
    sprintf(act_fn_name, "_%s", func_name);
    if (strcmp(libseal->func_names[i], act_fn_name) == 0) {
      return libseal->funcs[i](args, arg_size);
    }
  }
  
  libseal->func_size++;
  libseal->func_names = realloc(libseal->func_names, libseal->func_size * sizeof(char*));
  libseal->funcs = realloc(libseal->funcs, libseal->func_size * sizeof(ast_t* (*) (ast_t** args, size_t arg_size)));

  char* act_fn_name = calloc(strlen(func_name) + 2, sizeof(char));
  sprintf(act_fn_name, "_%s", func_name);
  libseal->func_names[libseal->func_size - 1] = act_fn_name;

#ifdef _WIN32
    FARPROC function = GetProcAddress(libseal->handle, act_fn_name);
    if (!function) {
      fprintf(stderr, "%s function is not found in library: %s\n", func_name, libseal->name);
      libseal_error();
    }
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))function;
#else
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))dlsym(libseal->handle, act_fn_name);
    if (!function_ptr) {
      libseal_error();
    }
#endif

  libseal->funcs[libseal->func_size - 1] = function_ptr;
  return function_ptr(args, arg_size);
}
