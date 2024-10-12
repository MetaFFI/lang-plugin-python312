#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_bytes.h"
#include "utils.h"

py_bytes::py_bytes(const char* val, Py_ssize_t size) : py_object()
{
	instance = PyBytes_FromStringAndSize(val, size);
	if (!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_bytes::py_bytes(PyObject* obj) : py_object(obj)
{
	if (!PyBytes_Check(obj))
	{
		throw std::runtime_error("Object is not a bytes");
	}
	
}

py_bytes::py_bytes(py_bytes&& other) noexcept : py_object(std::move(other))
{
}

py_bytes& py_bytes::operator=(const py_bytes& other)
{
	if(this->instance == other.instance){
		return *this;
	}
	
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

py_bytes::operator const uint8_t*() const
{
	return (const uint8_t*)PyBytes_AsString(instance);
}

py_bytes::operator uint8_t*() const
{
	auto buf = new uint8_t[PyBytes_Size(instance)];
	memcpy(buf, PyBytes_AsString(instance), PyBytes_Size(instance));
	return buf;
}

bool py_bytes::check(PyObject* obj)
{
	return PyBytes_Check(obj);
}

Py_ssize_t py_bytes::size() const
{
	return PyBytes_Size(instance);
}

uint8_t py_bytes::operator[](int i) const
{
	return PyBytes_AsString(instance)[i];
}
