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
#include "utils.h"
#include "py_object.h"

class py_int : public py_object
{
public:
	static bool check(PyObject* obj);
	
public:
	explicit py_int(int8_t val);
	explicit py_int(int16_t val);
	explicit py_int(int32_t val);
	explicit py_int(int64_t val);
	explicit py_int(uint8_t val);
	explicit py_int(uint16_t val);
	explicit py_int(uint32_t val);
	explicit py_int(uint64_t val);
	explicit py_int(PyObject* obj);
	explicit py_int(void* obj);
	py_int(py_int&& other) noexcept ;
	py_int& operator=(const py_int& other);
	
	explicit operator int8_t() const;
	explicit operator int16_t() const;
	explicit operator int32_t() const;
	explicit operator int64_t() const;
	explicit operator uint8_t() const;
	explicit operator uint16_t() const;
	explicit operator uint32_t() const;
	explicit operator uint64_t() const;
};
