#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_float.h"
#include "utils.h"

py_float::py_float(float val) : py_object()
{
	instance = PyFloat_FromDouble(static_cast<double>(val));
	if (!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_float::py_float(double val) : py_object()
{
	instance = PyFloat_FromDouble(val);
	if (!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_float::py_float(PyObject* obj) : py_object(obj)
{
	if (!PyFloat_Check(obj))
	{
		throw std::runtime_error(check_python_error());
	}
	instance = obj;
}

py_float::py_float(py_float&& other) noexcept : py_object(std::move(other))
{
}

py_float& py_float::operator=(const py_float& other)
{
	if (instance == other.instance)
	{
		return *this;
	}
	
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

py_float::operator float() const
{
	return static_cast<float>(PyFloat_AsDouble(instance));
}

py_float::operator double() const
{
	return PyFloat_AsDouble(instance);
}

bool py_float::check(PyObject* obj)
{
	return PyFloat_Check(obj);
}