#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_bool.h"


py_bool::py_bool(bool val) : py_object()
{
	instance = PyBool_FromLong(val);
	if (!instance)
	{
		throw std::runtime_error("Failed to create Python bool");
	}
}

py_bool::py_bool(PyObject* obj) : py_object(obj)
{
	if (!PyBool_Check(obj))
	{
		throw std::runtime_error("Object is not a bool");
	}
}

py_bool::py_bool(py_bool&& other) noexcept : py_object(std::move(other))
{
}

py_bool& py_bool::operator=(const py_bool& other)
{
	if(this->instance == other.instance){
		return *this;
	}
	
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

py_bool::operator bool() const
{
	return PyObject_IsTrue(instance);
}

py_bool::operator metaffi_bool() const
{
	return PyObject_IsTrue(instance) ? 1 : 0;
}

bool py_bool::check(PyObject* obj)
{
	return PyBool_Check(obj);
}