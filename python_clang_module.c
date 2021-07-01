#include <Python.h>

PyModuleDef python_clang_module = {
    PyModuleDef_HEAD_INIT,
    "python_clang", // Module name
    "This is the module docstring",
    -1,   // Optional size of the module state memory
    NULL, // Module method table
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL  // Optional module deallocation function
};

struct ClangCompiler {
  PyObject_HEAD
};

static PyTypeObject CompilerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "python_clang.Compiler",
    .tp_doc = "Clang compiler",
    .tp_basicsize = sizeof(struct ClangCompiler),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
#if 0
    .tp_init = (initproc) Custom_init,
    .tp_dealloc = (destructor) Custom_dealloc,
    .tp_members = Custom_members,
    .tp_methods = Custom_methods,
#endif
};

// The module init function
PyMODINIT_FUNC PyInit_python_clang(void) {
  printf("Init python_clang module\n");

  if (PyType_Ready(&CompilerType) < 0) {
    return NULL;
  }

  PyObject *m = PyModule_Create(&python_clang_module);
  if (m == NULL) {
    return NULL;
  }

  Py_INCREF(&CompilerType);

  if (PyModule_AddObject(m, "Compiler", (PyObject *)&CompilerType) < 0) {
    Py_DECREF(&CompilerType);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
