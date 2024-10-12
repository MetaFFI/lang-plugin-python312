#pragma once
#include <runtime/cdt.h>
#include <runtime/xcall.h>

// In order not to require python debug library
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

extern "C"
{
    PyObject* call_xcall(void* pxcall_ptr, void* context, PyObject* param_metaffi_types, PyObject* retval_metaffi_types, PyObject* args);
}