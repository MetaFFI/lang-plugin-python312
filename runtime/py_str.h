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

class py_str : public py_object
{
public:
	static bool check(PyObject* obj);
	
public:
	py_str();
	explicit py_str(PyObject* obj);
	explicit py_str(metaffi_char8 c);
	explicit py_str(metaffi_char16 c);
	explicit py_str(metaffi_char32 c);
	explicit py_str(const char8_t* s);
	explicit py_str(const char32_t* s);
	explicit py_str(const char16_t* s);
	py_str(py_str&& other) noexcept ;
	py_str& operator=(const py_str& other);
	[[nodiscard]] Py_ssize_t length() const;
	[[nodiscard]] metaffi_string8 to_utf8() const;
	[[nodiscard]] metaffi_string16 to_utf16() const;
	[[nodiscard]] metaffi_string32 to_utf32() const;
	
	explicit operator std::u8string() const;
	explicit operator std::u16string() const;
	explicit operator std::u32string() const;
};
