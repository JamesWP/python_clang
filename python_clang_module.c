#include <Python.h>


#include "python_clang_compiler.h"

int python_clang_clear(PyObject *module);

PyModuleDef python_clang_module = {
    PyModuleDef_HEAD_INIT,
    "python_clang", // Module name
    "This is the module docstring",
    -1,                 // Optional size of the module state memory
    NULL,               // Module method table
    NULL,               // Optional slot definitions
    NULL,               // Optional traversal function
    python_clang_clear, // Optional clear function
    NULL                // Optional module deallocation function
};

// The module init function
PyMODINIT_FUNC PyInit_python_clang(void)
{
  printf("Init python_clang module\n");

  PyObject *m = PyModule_Create(&python_clang_module);
  if (m == NULL)
  {
    return NULL;
  }

  if(!add_compiler_to_module(m)) {
    Py_DECREF(m);
    return NULL;
  }

  return m;
}

int python_clang_clear(PyObject *module)
{
  // TODO: test this is called
  remove_compiler_from_module(module);
  
  return 0;
}