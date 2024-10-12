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

struct cdt_metaffi_handle;

class py_metaffi_handle : public py_object
{
public:
	static bool check(PyObject* obj);
	static py_object extract_pyobject_from_handle(const cdt_metaffi_handle& cdt_handle);
	
public:
	explicit py_metaffi_handle(PyObject* obj);
	py_metaffi_handle(py_metaffi_handle&& other) noexcept;
	py_metaffi_handle& operator=(const py_metaffi_handle& other);
	
	[[nodiscard]] cdt_metaffi_handle* as_cdt_metaffi_handle() const;
};
