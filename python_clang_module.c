#include <Python.h>
#include <stdbool.h>

#include "clang_interface.h"

int python_clang_clear(PyObject* module);

PyModuleDef python_clang_module = {
    PyModuleDef_HEAD_INIT,
    "python_clang", // Module name
    "This is the module docstring",
    -1,   // Optional size of the module state memory
    NULL, // Module method table
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    python_clang_clear, // Optional clear function
    NULL  // Optional module deallocation function
};

struct ClangCompiler
{
  PyObject_HEAD;
  EnvironmentHandle handle;
  bool init;
};

static PyObject *
ClangCompiler_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    struct ClangCompiler *self;

    self = (struct ClangCompiler *)type->tp_alloc(type, 0);
    if (self == NULL) {
      printf("ClangCompiler new unable to alloc\n");
      return NULL;
    }

    memset(&self->handle, '\0', sizeof(self->handle));
    self->init = false;

    return (PyObject *)self;
}

static int ClangCompiler_init(PyObject *self, PyObject *args, PyObject *kwds)
{
  printf("ClangCompiler init %p\n", (void *)self);

  struct ClangCompiler *compiler_object = (struct ClangCompiler *)self;
  
  int rc = 0;

  if(compiler_object->init == true) {
    rc = destroyEnvironment(&compiler_object->handle);
    if(rc != 0) {
      printf("ClangCompiler init unable to destroy old env. rc=%d\n", rc);
    }
  }

  compiler_object->init = true;

  rc = createEnvironment(&compiler_object->handle);

  if (rc != 0)
  {
    printf("Error creating env. rc=%d\n", rc);
    return rc;
  }

  return 0;
}

static void ClangCompiler_dealloc(PyObject *self)
{
  printf("ClangCompiler dealoc %p\n", (void *)self);

  PyTypeObject *tp = Py_TYPE(self);

  struct ClangCompiler *compiler_object = (struct ClangCompiler *)self;

  int rc = destroyEnvironment(&compiler_object->handle);

  if (rc != 0)
  {
    printf("Error destroying env. rc=%d\n", rc);
  }
}

static PyTypeObject CompilerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "python_clang.Compiler",
    .tp_doc = "Clang compiler",
    .tp_basicsize = sizeof(struct ClangCompiler),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)ClangCompiler_init,
    .tp_dealloc = (destructor)ClangCompiler_dealloc,
#if 0
    .tp_members = Custom_members,
    .tp_methods = Custom_methods,
#endif
};

// The module init function
PyMODINIT_FUNC PyInit_python_clang(void)
{
  printf("Init python_clang module\n");

  if (PyType_Ready(&CompilerType) < 0)
  {
    return NULL;
  }

  PyObject *m = PyModule_Create(&python_clang_module);
  if (m == NULL)
  {
    return NULL;
  }

  Py_INCREF(&CompilerType);

  if (PyModule_AddObject(m, "Compiler", (PyObject *)&CompilerType) < 0)
  {
    Py_DECREF(&CompilerType);
    Py_DECREF(m);
    return NULL;
  }

  init();

  return m;
}

int python_clang_clear(PyObject* module) {
  // TODO: test this is called
  fin();
  return 0;
}