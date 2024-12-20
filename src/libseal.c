#include "libseal.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <dlfcn.h>
#endif

static void libseal_error()
{
#ifdef _WIN32
  DWORD error = GetLastError();
  fprintf(stderr, "Libseal-> error: %lu\n", error);
#else
  fprintf(stderr, "Libseal-> error: %s\n", dlerror());
#endif
  exit(1);
}

libseal_t* init_libseal(const char* name, bool has_alias, const char* alias_name)
{
  libseal_t* libseal = calloc(1, sizeof(libseal_t));

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
  libseal->functions = (void*)0;
  libseal->function_names = (void*)0;
  libseal->function_size = 0;

  libseal->name = has_alias ? alias_name : name;

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
  function_ptr((void*)0, 0);
  
  return libseal;
}

ast_t* libseal_function_call(libseal_t* libseal, const char* func_name, ast_t** args, size_t arg_size)
{
  for (int i = 0; i < libseal->function_size; i++) {
    char act_fn_name[strlen(func_name) + 2];
    sprintf(act_fn_name, "_%s", func_name);
    if (strcmp(libseal->function_names[i], act_fn_name) == 0) {
      return libseal->functions[i](args, arg_size);
    }
  }
  
  libseal->function_size++;
  libseal->function_names = realloc(libseal->function_names, libseal->function_size * sizeof(char*));
  libseal->functions = realloc(libseal->functions, libseal->function_size * sizeof(ast_t* (*) (ast_t** args, size_t arg_size)));

  char* act_fn_name = calloc(strlen(func_name) + 2, sizeof(char));
  sprintf(act_fn_name, "_%s", func_name);
  libseal->function_names[libseal->function_size - 1] = act_fn_name;

#ifdef _WIN32
    FARPROC function = GetProcAddress(libseal->handle, act_fn_name);
    if (!function) {
      fprintf(stderr, "%s function is not found in libseal: %s\n", func_name, libseal->name);
      libseal_error();
    }
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))function;
#else
    ast_t* (*function_ptr)(ast_t**, size_t) = (ast_t* (*)(ast_t**, size_t))dlsym(libseal->handle, act_fn_name);
    if (!function_ptr) {
      libseal_error();
    }
#endif

  libseal->functions[libseal->function_size - 1] = function_ptr;
  return function_ptr(args, arg_size);
}
