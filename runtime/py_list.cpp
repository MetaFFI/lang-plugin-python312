#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_list.h"
#include <stdexcept>
#include "utils.h"
#include "py_metaffi_handle.h"
#include "runtime_id.h"
#include "py_bytes.h"

py_list::py_list(Py_ssize_t size /*= 0*/): py_object()
{
	instance = PyList_New(size);
	if (!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_list::py_list(PyObject* obj): py_object(obj)
{
	if (!PyList_Check(obj))
	{
		std::stringstream ss;
		ss << "Object is not a list. It is " << obj->ob_type->tp_name;
		throw std::runtime_error(ss.str());
	}
	instance = obj;
}

py_list::py_list(py_list&& other) noexcept : py_object(std::move(other))
{
}

py_list& py_list::operator=(const py_list& other)
{
	if (this == &other)
	{
		return *this;
	}
	
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

PyObject* py_list::operator[](int index)
{
	PyObject* item = PyList_GetItem(instance, index);
	if (!item)
	{
		throw std::runtime_error(check_python_error());
	}
	return item;
}

Py_ssize_t py_list::length() const
{
	return PyList_Size(instance);
}

void py_list::append(PyObject* obj)
{
	int res = PyList_Append(instance, obj);
	if (res == -1)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_list& py_list::operator=(PyObject* other)
{
	if (instance == other)
	{
		return *this;
	}
	
	instance = other;
	Py_XINCREF(instance);
	return *this;
}

bool py_list::check(PyObject* obj)
{
	return PyList_Check(obj);
}

py_list::py_list(py_list& other) noexcept
{
	instance = other.instance;
	Py_XINCREF(instance);
}

void py_list::get_metadata(PyObject* obj, bool& out_is_1d_array, bool& out_is_fixed_dimension, Py_ssize_t& out_size, metaffi_type& out_common_type)
{
	out_size = PyList_Size(obj);
	out_is_1d_array = true;
	out_is_fixed_dimension = true;
	out_common_type = 0;
	
#define ARRAY       1
#define NOT_ARRAY   2
	
	int8_t last_item = 0;
	for(Py_ssize_t i = 0; i < out_size; i++)
	{
		PyObject* item = PyList_GetItem(obj, i);
		
		if(i == 0)
		{
			out_common_type = py_object::get_metaffi_type(item);
		}
		else if(out_common_type != 0 && out_common_type != py_object::get_metaffi_type(item))
		{
			out_common_type = 0; // no common type
		}
		
		if(PyList_Check(item) || PyTuple_Check(item) || PyBytes_Check(item))
		{
			out_is_1d_array = false;
			
			if(i == 0)
			{
				last_item = ARRAY;
			}
			else if(last_item == NOT_ARRAY)
			{
				out_is_fixed_dimension = false;
				break;
			}
		}
		else
		{
			if(last_item == 0)
			{
				last_item = NOT_ARRAY;
			}
			else if(last_item == ARRAY)
			{
				out_is_fixed_dimension = false;
				break;
			}
		}
	}
	
	if(out_size == 0)
	{
		out_is_fixed_dimension = true;
	}
	
	if(!out_is_fixed_dimension)
	{
		out_is_1d_array = false;
	}
	else if(out_is_fixed_dimension && (out_size == 0 || last_item == NOT_ARRAY) )
	{
		out_is_1d_array = true;
	}
}