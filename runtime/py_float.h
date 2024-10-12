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

class py_float : public py_object
{
public:
	static bool check(PyObject* obj);
	
	explicit py_float(float val);
	explicit py_float(double val);
	explicit py_float(PyObject* obj);
	py_float(py_float&& other) noexcept ;
	py_float& operator=(const py_float &other);
	explicit operator float() const;
	explicit operator double() const;
};
