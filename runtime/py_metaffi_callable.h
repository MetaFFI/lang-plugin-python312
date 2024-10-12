#pragma once
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include <runtime/metaffi_primitives.h>
#include "py_object.h"
#include <runtime/cdt.h>

class py_metaffi_callable : public py_object
{
public:
	static bool check(PyObject* obj);
	
private:
	static PyObject* create_lambda;
	
private:
	py_metaffi_callable();
	
public:
	explicit py_metaffi_callable(const cdt_metaffi_callable& cdt_callable);
	py_metaffi_callable(py_metaffi_callable&& other) noexcept;
	explicit py_metaffi_callable(PyObject* obj);
	py_metaffi_callable& operator=(const py_metaffi_callable& other);
	
};
