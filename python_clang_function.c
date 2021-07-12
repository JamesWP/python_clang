#include <Python.h>
#include "python_clang_function.h"
#include "clang_interface.h"
#include <structmember.h>

struct ClangFunction
{
    PyObject_HEAD;
    clang_interface_FunctionHandle handle;
    PyObject *compiler;
};

static void ClangFunction_dealloc(PyObject *self)
{
    printf("ClangFunction dealoc %p\n", (void *)self);
    struct ClangFunction *function_object = (struct ClangFunction *)self;
    Py_XDECREF(function_object->compiler);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int ClangFunction_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    struct ClangFunction *function_object = (struct ClangFunction *)self;

    printf("ClangFunction init %p\n", (void *)self);

    PyObject *compiler = NULL;
    PyObject *function = NULL;

    static char *kwlist[] = {"compiler", "function", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist,
                                     &compiler, &function))
    {
        return -1;
    }

    if (compiler)
    {
        PyObject *tmp = function_object->compiler;
        Py_INCREF(compiler);
        function_object->compiler = compiler;
        Py_XDECREF(tmp);
    }

    if(function && PyCapsule_CheckExact(function) && PyCapsule_IsValid(function, NULL))
    {
        clang_interface_FunctionHandle* passed_function = (clang_interface_FunctionHandle*) PyCapsule_GetPointer(function, NULL);
    
        function_object->handle = *passed_function;
    }

    return 0;
}

static PyMemberDef ClangFunction_members[] = {
    {"compiler", T_OBJECT_EX, offsetof(struct ClangFunction, compiler), 0,
     "compiler"},
    {NULL} /* Sentinel */
};

PyObject *ClangFunction_call(PyObject *self, PyObject *args, PyObject *kwargs)
{
    struct ClangFunction *function_object = (struct ClangFunction *)self;

    printf("ClangFunction call %p\n", (void *)self);

    int output;
    if(0 != clang_interface_runCode(&function_object->handle, &output)) {
        return PyErr_Format(PyExc_ValueError, "Unable to run code");
    }
    
    return PyLong_FromLong(output);
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
    .tp_members = ClangFunction_members,
};

PyObject *FunctionType_p = (PyObject *)&FunctionType;

bool add_function_to_module(PyObject *module)
{
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

void remove_function_from_module(PyObject *module)
{
    assert(PyModule_Check(module));
}