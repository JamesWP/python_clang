#pragma once
#include <stdbool.h>

struct PyObject;

bool add_compiler_to_module(PyObject *);
void remove_compiler_from_module(PyObject *);