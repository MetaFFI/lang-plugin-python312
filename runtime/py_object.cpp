#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_bool.h"
#include "py_bytes.h"
#include "py_float.h"
#include "py_list.h"
#include "py_object.h"
#include "py_str.h"
#include "py_tuple.h"
#include <unordered_map>

py_object::py_object(PyObject* obj)
{
	instance = obj;
	if(!Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
}

py_object::py_object(py_object&& other) noexcept
{
	instance = other.instance;
	other.instance = nullptr;
}

py_object& py_object::operator=(const py_object& other)
{
	if(instance == other.instance)
	{
		return *this;
	}
	
	instance = other.instance;
	if(!Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
	return *this;
}

py_object::operator PyObject*() const
{
	return instance;
}

const char* py_object::get_type() const
{
	return instance->ob_type->tp_name;
}

void py_object::inc_ref()
{
	if(!Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
}

void py_object::dec_ref()
{
	if(!Py_IsNone(instance))
	{
		Py_XINCREF(instance);
	}
}

py_object::~py_object()
{
	if(instance != nullptr && !Py_IsNone(instance)){
		Py_XDECREF(instance);
	}
}

PyObject* py_object::get_attribute(const char* name) const
{
	return PyObject_GetAttrString(instance, name);
}

void py_object::set_attribute(const char *name, PyObject *val)
{
	PyObject_SetAttrString(instance, name, val);
}

PyObject* py_object::detach()
{
	PyObject* res = instance;
	instance = nullptr;
	return res;
}

const char* py_object::get_object_type(PyObject* obj)
{
	return obj->ob_type->tp_name;
}

metaffi_type py_object::get_metaffi_type(PyObject* obj)
{
	if(py_str::check(obj)){
		return metaffi_string8_type;
	}
	else if(py_bool::check(obj)){
		return metaffi_bool_type;
	}
	else if(py_int::check(obj)){
		return metaffi_int64_type;
	}
	else if(py_float::check(obj)){
		return metaffi_float64_type;
	}
	else if(py_list::check(obj) || py_tuple::check(obj)){
		return metaffi_array_type;
	}
	else if(py_bytes::check(obj)){
		return metaffi_uint8_array_type;
	}
	else if(Py_IsNone(obj)){
		return metaffi_null_type;
	}
	else{
		return metaffi_handle_type;
	}
}