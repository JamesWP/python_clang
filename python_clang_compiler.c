#include <Python.h>
#include <stdbool.h>

#include "clang_interface.h"

struct ClangCompiler
{
    PyObject_HEAD;
    clang_interface_EnvironmentHandle handle;
    bool init;
};

static PyObject *
ClangCompiler_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    struct ClangCompiler *self;

    self = (struct ClangCompiler *)type->tp_alloc(type, 0);
    if (self == NULL)
    {
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

    if (compiler_object->init == true)
    {
        rc = clang_interface_destroyEnvironment(&compiler_object->handle);
        if (rc != 0)
        {
            printf("ClangCompiler init unable to destroy old env. rc=%d\n", rc);
        }
    }

    compiler_object->init = true;

    rc = clang_interface_createEnvironment(&compiler_object->handle);

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

    if (compiler_object->init == false)
    {
        return;
    }

    int rc = clang_interface_destroyEnvironment(&compiler_object->handle);

    if (rc != 0)
    {
        printf("Error destroying env. rc=%d\n", rc);
    }

    printf("ClangCompiler dealoc %p finished\n", (void *)self);
}

PyObject *ClangCompiler_compile(PyObject *self, PyObject *arg)
{
    printf("ClangCompiler compile %p\n", (void *)self);
    struct ClangCompiler *compiler_object = (struct ClangCompiler *)self;

    if (compiler_object->init != true)
    {
        Py_DecRef(arg);
        return PyErr_Format(PyExc_ValueError, "Compiler instance not initialized");
    }

    printf("Yeee using new\n");

    size_t arg_code_size = 0;
    const char *arg_code_str = PyUnicode_AsUTF8AndSize(arg, &arg_code_size);

    if (arg_code_str == NULL)
    {
        Py_DecRef(arg);
        return PyErr_Format(PyExc_ValueError, "Unable to parse code");
    }

    int rc = clang_interface_compileCode(&compiler_object->handle, arg_code_str, arg_code_size);

    // Causes segfault??
    //Py_DecRef(arg);

    if (rc != 0)
    {
        return PyErr_Format(PyExc_ValueError, "Unable to compile code");
    }

    printf("Yeee working\n");

    return PyErr_Format(PyExc_NotImplementedError, "Not implemented yet");
}

static PyMethodDef ClangCompiler_methods[] = {
    {.ml_name = "compile",
     .ml_meth = ClangCompiler_compile,
     .ml_flags = METH_O,
     .ml_doc = NULL},
    NULL};

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
    .tp_methods = ClangCompiler_methods,
};

bool add_compiler_to_module(PyObject *module)
{
    if (!PyModule_Check(module)) {
        PyErr_BadInternalCall();
        return false;
    }

    if (PyType_Ready(&CompilerType) < 0)
    {
        return false;
    }

    Py_INCREF(&CompilerType);

    if (PyModule_AddObject(module, "Compiler", (PyObject *)&CompilerType) < 0)
    {
        Py_DECREF(&CompilerType);
        return false;
    }

    // initialize clang interface module
    clang_interface_init();

    return true;
}

bool remove_compiler_from_module(PyObject* m) {
    (void) m;
    clang_interface_fini();
}