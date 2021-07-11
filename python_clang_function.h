#pragma once
#include <stdbool.h>

struct PyObject;

PyObject* FunctionType_p;

bool add_function_to_module(PyObject *);
void remove_function_from_module(PyObject *);