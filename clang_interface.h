#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


// C interface declaration
struct clang_interface_EnvironmentHandle {
    void* handle;
};

struct clang_interface_FunctionHandle {
    int (*compiledFunctionHandle)();
};

typedef struct clang_interface_EnvironmentHandle clang_interface_EnvironmentHandle;
typedef struct clang_interface_FunctionHandle clang_interface_FunctionHandle;

int clang_interface_init();
int clang_interface_fini();

int clang_interface_createEnvironment(clang_interface_EnvironmentHandle *);
int clang_interface_destroyEnvironment(clang_interface_EnvironmentHandle *);
int clang_interface_compileCode(clang_interface_EnvironmentHandle *, clang_interface_FunctionHandle *, const char* code, size_t code_size);
int clang_interface_runCode(clang_interface_FunctionHandle *, int* output);


#ifdef __cplusplus
} // Extern C
#endif
