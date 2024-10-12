#ifdef _WIN32

#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)

#endif

#include "py_str.h"
#include <string>

py_str::py_str() : py_object()
{
	instance = PyUnicode_FromString("");
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(PyObject* obj) : py_object(obj)
{
	if(!PyUnicode_Check(obj))
	{
		throw std::runtime_error("Object is not a unicode string");
	}
}

py_str::py_str(py_str&& other) noexcept : py_object(std::move(other))
{
}

py_str& py_str::operator=(const py_str& other)
{
	if(this->instance == other.instance)
	{
		return *this;
	}
	
	Py_XDECREF(instance);
	instance = other.instance;
	Py_XINCREF(instance);
	return *this;
}

Py_ssize_t py_str::length() const
{
	return PyUnicode_GetLength(instance);
}

py_str::py_str(const char8_t* s)
{
	instance = PyUnicode_FromString(reinterpret_cast<const char*>(s));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(const char32_t* s)
{
	uint64_t length = std::char_traits<char32_t>::length(s);
	
	instance = PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, s, (Py_ssize_t) length);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(const char16_t* s)
{
	uint64_t length = std::char_traits<char16_t>::length(s);
	
	instance = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, s, (Py_ssize_t) length);
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

metaffi_string8 py_str::to_utf8() const
{
	PyObject* utf8 = PyUnicode_AsUTF8String(instance);
	if(!utf8)
	{
		throw std::runtime_error(check_python_error());
	}
	char* s;
	Py_ssize_t len;
	PyBytes_AsStringAndSize(utf8, &s, &len);
	
	// Allocate memory for the metaffi_string8
	metaffi_string8 result = new char8_t[len + 1];

	// Copy the string to the allocated memory
	memcpy(result, s, len);
	result[len] = '\0'; // Null-terminate the string

	return result;
}

metaffi_string16 py_str::to_utf16() const
{
	PyObject* bytes = PyUnicode_AsUTF16String(instance);
	if(!bytes)
	{
		throw std::runtime_error(check_python_error());
	}
	char* s = PyBytes_AsString(bytes);
	Py_ssize_t size = PyBytes_Size(bytes);
	
	// Allocate memory for the metaffi_string16
	metaffi_string16 result = new char16_t[size + 1];

	// Copy the string to the allocated memory
	memcpy(result, s, size);
	result[size / 2] = '\0'; // Null-terminate the string

	Py_DECREF(bytes);
	return result;
}

metaffi_string32 py_str::to_utf32() const
{
	PyObject* bytes = PyUnicode_AsUTF32String(instance);
	if(!bytes)
	{
		throw std::runtime_error(check_python_error());
	}
	char* s = PyBytes_AsString(bytes);
	Py_ssize_t size = PyBytes_Size(bytes);
	
	// Allocate memory for the metaffi_string32
	metaffi_string32 result = new char32_t[size + 1];
	
	// Copy the string to the allocated memory
	memcpy(result, s, size);
	result[size / 4] = '\0'; // Null-terminate the string

	Py_DECREF(bytes);
	return result;
}

py_str::operator std::u8string() const
{
	return std::move(to_utf8());
}

py_str::operator std::u16string() const
{
	return std::move(to_utf16());
}

py_str::operator std::u32string() const
{
	return std::move(to_utf32());
}

bool py_str::check(PyObject* obj)
{
	return PyUnicode_Check(obj);
}

py_str::py_str(const metaffi_char8 c)
{
	instance = PyUnicode_FromStringAndSize(reinterpret_cast<const char*>(c.c), metaffi_char8::num_of_bytes(c.c));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(const metaffi_char16 c)
{
	instance = PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, reinterpret_cast<const char*>(c.c), metaffi_char16::num_of_bytes(c.c));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}

py_str::py_str(const metaffi_char32 c)
{
	instance = PyUnicode_FromKindAndData(PyUnicode_4BYTE_KIND, reinterpret_cast<const char*>(c.c), sizeof(char32_t));
	if(!instance)
	{
		throw std::runtime_error(check_python_error());
	}
}
