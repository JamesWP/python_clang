#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


// C interface declaration

int test();

struct EnvironmentHandle {
    void* handle;
};

typedef struct EnvironmentHandle EnvironmentHandle;

int init();
int fin();

int createEnvironment(EnvironmentHandle *);
int destroyEnvironment(EnvironmentHandle *);
int compileCode(EnvironmentHandle *, const char* code, size_t code_size);
int runCode(EnvironmentHandle *, int* output);


#ifdef __cplusplus
} // Extern C
#endif
