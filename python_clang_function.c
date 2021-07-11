#include <Python.h>
#include "python_clang_function.h"

struct ClangFunction {
    PyObject_HEAD;
};

static int ClangFunction_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    printf("ClangFunction init %p\n", (void*) self);

    return 0;
}

static void ClangFunction_dealloc(PyObject *self) 
{
    printf("ClangFunction dealoc %p\n", (void *)self);
}

PyObject *ClangFunction_call(PyObject *self, PyObject *args, PyObject *kwargs)
{
    return PyErr_Format(PyExc_ValueError, "Unable to run code");
}


static PyTypeObject FunctionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "python_clang.Function",
    .tp_doc = "Clang function",
    .tp_basicsize = sizeof(struct ClangFunction),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)ClangFunction_init,
    .tp_dealloc = (destructor)ClangFunction_dealloc,
    .tp_call = (ternaryfunc)ClangFunction_call,
};

PyObject* FunctionType_p = (PyObject*)&FunctionType;

bool add_function_to_module(PyObject *module) {
    assert(PyModule_Check(module));

    if (PyType_Ready(&FunctionType) < 0)
    {
        return false;
    }

    Py_INCREF(&FunctionType);

    if (PyModule_AddObject(module, "Function", (PyObject *)&FunctionType) < 0)
    {
        Py_DECREF(&FunctionType);
        return false;
    }

    return true;
}

void remove_function_from_module(PyObject *module) {
    assert(PyModule_Check(module));
}