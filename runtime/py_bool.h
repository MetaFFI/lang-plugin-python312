#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include <stdexcept>
#include <string>
#include "py_object.h"

class py_bool : public py_object
{
public:
	static bool check(PyObject* obj);
	
public:
	explicit py_bool(bool val);
	explicit py_bool(PyObject* obj);
	py_bool(py_bool&& other) noexcept ;
	py_bool& operator=(const py_bool& other);
	explicit operator bool() const;
	explicit operator metaffi_bool() const;
};
