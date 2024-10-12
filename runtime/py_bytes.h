#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include "py_object.h"

class py_bytes : public py_object
{
public:
	static bool check(PyObject* obj);
	
public:
	py_bytes() = default;
	explicit py_bytes(const char* val, Py_ssize_t size);
	explicit py_bytes(PyObject* obj);
	py_bytes(py_bytes&& other) noexcept;
	py_bytes& operator=(const py_bytes& other);
	
	[[nodiscard]] Py_ssize_t size() const;
	
	uint8_t operator[](int i) const;
	explicit operator const uint8_t*() const;
	explicit operator uint8_t*() const;
};
