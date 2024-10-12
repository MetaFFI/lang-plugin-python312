#pragma once

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif
#include "py_object.h"
#include "runtime/metaffi_primitives.h"
#include "py_int.h"

class py_list : public py_object
{
public:
	static bool check(PyObject* obj);
	static void get_metadata(PyObject* obj, bool& out_is_1d_array, bool& out_is_fixed_dimension, Py_ssize_t& out_size, metaffi_type& out_common_type);
	
public:
	explicit py_list(Py_ssize_t size = 0);
	explicit py_list(PyObject* obj);
	py_list(py_list& other) noexcept;
	py_list(py_list&& other) noexcept;
	py_list& operator=(const py_list& other);
	py_list& operator=(PyObject* other);
	PyObject* operator[](int index);
	
	void append(PyObject* obj);
	[[nodiscard]] Py_ssize_t length() const;
	
	//--------------------------------------------------------------------
	
	template<typename ctype_t, typename py_type_t>
	void add_numeric_array(const ctype_t* arr, const metaffi_size* lengths, metaffi_size dimensions)
	{
		if (dimensions == 1)
		{
			for (int i = 0; i < lengths[0]; ++i)
			{
				py_type_t val(arr[i]);
				PyList_Append(instance, (PyObject*)val);
			}
		}
		else
		{
			for (int i = 0; i < lengths[0]; ++i)
			{
				py_list sublist;
				sublist.add_numeric_array<ctype_t, py_type_t>(arr + i * lengths[1], lengths + 1, dimensions - 1);
				PyList_Append(instance, (PyObject*)sublist);
			}
		}
	}
	
	template<typename ctype_t, typename py_type_t>
	void add_string_array(const ctype_t* arr, const metaffi_size* strings_lengths, const metaffi_size* lengths, metaffi_size dimensions)
	{
		if (dimensions == 1)
		{
			for (int i = 0; i < lengths[0]; i++)
			{
				py_type_t val(arr[i], strings_lengths[i]);
				PyList_Append(instance, (PyObject*)val);
			}
		}
		else
		{
			for (int i = 0; i < lengths[0]; i++)
			{
				py_list sublist;
				sublist.add_string_array<ctype_t, py_type_t>(arr + i * lengths[1], strings_lengths + i *  lengths[1], lengths + 1, dimensions - 1);
				PyList_Append(instance, (PyObject*)sublist);
			}
		}
	}
	
	void add_handle_array(const cdt_metaffi_handle* arr, const metaffi_size* lengths, metaffi_size dimensions);
	
	//--------------------------------------------------------------------
	
	template<typename ctype_t, typename py_type_t>
	void get_numeric_array(ctype_t** out_arr, metaffi_size** out_lengths, metaffi_size dimensions, metaffi_size* current_length = nullptr, ctype_t* current_arr = nullptr)
	{
		if (dimensions == 1)
		{
			Py_ssize_t size = PyList_Size(instance);
			*out_lengths = current_length ? current_length : new metaffi_size[dimensions];
			(*out_lengths)[0] = size;
			*out_arr = new ctype_t[size];
			
			for (metaffi_size i = 0; i < (*out_lengths)[0]; i++)
			{
				PyObject* item = PyList_GetItem(instance, (Py_ssize_t)i);
				if (!py_type_t::check(item))
				{
					throw std::runtime_error("Python object is not of expected type");
				}
				(*out_arr)[i] = (ctype_t)py_type_t(item);
			}
		}
		else
		{
			*out_lengths = current_length ? current_length : new metaffi_size[dimensions];
			(*out_lengths)[0] = PyList_Size(instance);
			*out_arr = current_arr ? current_arr : new ctype_t[(*out_lengths)[0]];
			
			for (metaffi_size i = 0; i < (*out_lengths)[0]; i++)
			{
				PyObject* item = PyList_GetItem(instance, (Py_ssize_t)i);
				if (!PyList_Check(item))
				{
					std::stringstream ss;
					ss << "Object is not a list. It is " << item->ob_type->tp_name;
					throw std::runtime_error(ss.str());
				}
				
				py_list sublist(item);
				sublist.get_numeric_array<ctype_t, py_type_t>(out_arr, out_lengths, dimensions - 1, *out_lengths + 1, *out_arr + i * (*out_lengths)[1]);
			}
		}
	}
	
	//--------------------------------------------------------------------
	
	template<typename ctype_t, typename py_type_t, typename char_t>
	void get_string_array(ctype_t** out_arr, metaffi_size** out_arr_lengths, metaffi_size** out_lengths, metaffi_size dimensions, metaffi_size* current_length = nullptr, ctype_t* current_arr = nullptr)
	{
		if(dimensions == 1)
		{
			Py_ssize_t size = PyList_Size(instance);
			*out_lengths = current_length ? current_length : new metaffi_size[dimensions];
			(*out_lengths)[0] = size;
			*out_arr = new ctype_t[size];
			*out_arr_lengths = new metaffi_size[size];
			
			for(metaffi_size i = 0; i < size; i++)
			{
				PyObject* item = PyList_GetItem(instance, (Py_ssize_t)i);
				if(!py_type_t::check(item))
				{
					throw std::runtime_error("Python object is not of expected type");
				}
				
				auto s = (std::basic_string<char_t>)py_type_t(item);
				(*out_arr)[i] = new char_t[s.size()];
				std::copy(s.begin(), s.end(), (*out_arr)[i]);
				(*out_arr_lengths)[i] = s.size();
			}
		}
		else
		{
			*out_lengths = current_length ? current_length : new metaffi_size[dimensions];
			(*out_lengths)[0] = PyList_Size(instance);
			*out_arr = current_arr ? current_arr : new ctype_t[(*out_lengths)[0]];
			
			for (metaffi_size i = 0; i < (*out_lengths)[0]; i++)
			{
				PyObject* item = PyList_GetItem(instance, (Py_ssize_t)i);
				if (!PyList_Check(item))
				{
					std::stringstream ss;
					ss << "Object is not a list. It is " << item->ob_type->tp_name;
					throw std::runtime_error(ss.str());
				}
				
				py_list sublist(item);
				sublist.get_string_array<ctype_t, py_type_t, char_t>(out_arr, out_arr_lengths, out_lengths, dimensions - 1, *out_lengths + 1, *out_arr + i * (*out_lengths)[1]);
			}
		}
	}
	
	void get_handle_array(cdt_metaffi_handle** out_arr, metaffi_size** out_lengths, metaffi_size dimensions, metaffi_size* current_length = nullptr, cdt_metaffi_handle* current_arr = nullptr);
	void get_bytes_array(metaffi_uint8*** out_arr, metaffi_size** out_lengths, metaffi_size dimensions);
	
	//--------------------------------------------------------------------
	
	void add_bytes_array(metaffi_uint8* bytes, metaffi_size* bytes_lengths, metaffi_size len);
};
