#include "cdts_python3.h"
#include "py_bool.h"
#include "py_bytes.h"
#include "py_float.h"
#include "py_int.h"
#include "py_list.h"
#include "py_metaffi_callable.h"
#include "py_metaffi_handle.h"
#include "py_str.h"
#include "py_tuple.h"
#include "runtime_id.h"
#include "utils.h"
#include <mutex>
#include <runtime/cdts_traverse_construct.h>
#include <utils/defines.h>

using namespace metaffi::runtime;

//--------------------------------------------------------------------

void set_element(PyObject* container, PyObject* item, metaffi_size index)
{
	if(PyList_Check(container))
	{
		PyList_SetItem(container, index, item);
	}
	else if(PyTuple_Check(container))
	{
		PyTuple_SetItem(container, index, item);
	}
	else
	{
		std::stringstream ss;
		ss << "Expected list or tuple. Received: " << container->ob_type->tp_name;
		throw std::runtime_error(ss.str());
	}
}

PyObject* get_element(PyObject* obj, const metaffi_size* index, metaffi_size index_size)
{
	PyObject* tmp = obj;

	for(metaffi_size i = 0; i < index_size; i++)
	{
		if(PyList_Check(tmp))
		{
			tmp = PyList_GetItem(tmp, index[i]);
		}
		else if(PyTuple_Check(tmp))
		{
			tmp = PyTuple_GetItem(tmp, index[i]);
		}
		else if(PyBytes_Check(tmp))
		{
			py_bytes bytes(tmp);
			auto b = bytes[i];
			tmp = (PyObject*)py_int(b).detach();
		}
		else
		{
			std::stringstream ss;
			ss << "Expected list or tuple. Received: " << tmp->ob_type->tp_name;
			throw std::runtime_error(ss.str());
		}
	}
	return tmp;
}

DLL_PRIVATE void py_on_traverse_float64(const metaffi_size* index, metaffi_size index_size, metaffi_float64 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_float(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_float(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_float32(const metaffi_size* index, metaffi_size index_size, metaffi_float32 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_float(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_float(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_int8(const metaffi_size* index, metaffi_size index_size, metaffi_int8 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_uint8(const metaffi_size* index, metaffi_size index_size, metaffi_uint8 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else
	{
		// shouldn't get here, this is for python "bytes" and it is handles in on_traverse_array
		throw std::runtime_error("Received uint8 array, which is \"bytes\". Should be handled by on_traverse_array");
	}
}

DLL_PRIVATE void py_on_traverse_int16(const metaffi_size* index, metaffi_size index_size, metaffi_int16 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_uint16(const metaffi_size* index, metaffi_size index_size, metaffi_uint16 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_int32(const metaffi_size* index, metaffi_size index_size, metaffi_int32 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_uint32(const metaffi_size* index, metaffi_size index_size, metaffi_uint32 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_int64(const metaffi_size* index, metaffi_size index_size, metaffi_int64 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_uint64(const metaffi_size* index, metaffi_size index_size, metaffi_uint64 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_int(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_int(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_bool(const metaffi_size* index, metaffi_size index_size, metaffi_bool val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_bool(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_bool(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_char8(const metaffi_size* index, metaffi_size index_size, metaffi_char8 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_string8(const metaffi_size* index, metaffi_size index_size, metaffi_string8 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_char16(const metaffi_size* index, metaffi_size index_size, metaffi_char16 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_string16(const metaffi_size* index, metaffi_size index_size, metaffi_string16 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_char32(const metaffi_size* index, metaffi_size index_size, metaffi_char32 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_string32(const metaffi_size* index, metaffi_size index_size, metaffi_string32 val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_str(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_str(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_handle(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_handle& val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_metaffi_handle::extract_pyobject_from_handle(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_metaffi_handle::extract_pyobject_from_handle(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_callable(const metaffi_size* index, metaffi_size index_size, const cdt_metaffi_callable& val, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, (PyObject*) py_metaffi_callable(val).detach());
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, (PyObject*) py_metaffi_callable(val).detach(), index[index_size - 1]);
	}
}

DLL_PRIVATE void py_on_traverse_null(const metaffi_size* index, metaffi_size index_size, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		tuple->set_item(0, Py_None);
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);
		set_element(elem, Py_None, index[index_size - 1]);
	}
}

DLL_PRIVATE metaffi_bool py_on_traverse_array(const metaffi_size* index, metaffi_size index_size, const cdts& val, metaffi_int64 fixed_dimensions,
                       metaffi_type common_type, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	if(index_size == 0)
	{
		// check if it is a bytes array
		if((common_type & metaffi_any_type) && fixed_dimensions == 1)
		{
			bool is_byte_array = true;
			for(metaffi_size i = 0; i < val.length; i++)
			{
				if(val[i].type != metaffi_uint8_type)
				{
					is_byte_array = false;
					break;
				}
			}
			
			if(is_byte_array){
				common_type = metaffi_uint8_type;
			}
		}
		
		// handle byte[], in python it is an object of "bytes"
		if((common_type & metaffi_uint8_type) && fixed_dimensions == 1)
		{
			std::vector<metaffi_uint8> bytes(val.length);
			for(metaffi_size i = 0; i < val.length; i++)
			{
				bytes[i] = val[i].cdt_val.uint8_val;
			}

			tuple->set_item(0, (PyObject*) py_bytes((const char*) bytes.data(), (Py_ssize_t) bytes.size()).detach());
			return 0;
		}
		else
		{
			tuple->set_item(0, (PyObject*) py_list(val.length));
			return 1;
		}
	}
	else// if is part of an array
	{
		PyObject* elem = get_element((PyObject*) (*tuple), index, index_size - 1);

		if((common_type & metaffi_uint8_type) && fixed_dimensions == 1)// bytes
		{
			std::vector<metaffi_uint8> bytes(val.length);
			for(metaffi_size i = 0; i < val.length; i++)
			{
				bytes[i] = val[i].cdt_val.uint8_val;
			}

			set_element(elem, (PyObject*) py_bytes((const char*) bytes.data(), (Py_ssize_t) bytes.size()).detach(), index[index_size - 1]);
			return 0;
		}
		else
		{
			set_element(elem, (PyObject*) py_list(val.length).detach(), index[index_size - 1]);
			return 1;
		}
	}
}

DLL_PRIVATE metaffi::runtime::traverse_cdts_callbacks py_get_traverse_cdts_callback(void* context)
{
	metaffi::runtime::traverse_cdts_callbacks tcc = {
	        context,
	        &py_on_traverse_float64,
	        &py_on_traverse_float32,
	        &py_on_traverse_int8,
	        &py_on_traverse_uint8,
	        &py_on_traverse_int16,
	        &py_on_traverse_uint16,
	        &py_on_traverse_int32,
	        &py_on_traverse_uint32,
	        &py_on_traverse_int64,
	        &py_on_traverse_uint64,
	        &py_on_traverse_bool,
	        &py_on_traverse_char8,
	        &py_on_traverse_string8,
	        &py_on_traverse_char16,
	        &py_on_traverse_string16,
	        &py_on_traverse_char32,
	        &py_on_traverse_string32,
	        &py_on_traverse_handle,
	        &py_on_traverse_callable,
	        &py_on_traverse_null,
	        &py_on_traverse_array};

	return tcc;
}

DLL_PRIVATE metaffi_size py_get_array_metadata(const metaffi_size* index, metaffi_size index_length, metaffi_bool* is_fixed_dimension, metaffi_bool* is_1d_array, metaffi_type* common_type, metaffi_bool* is_manually_construct_array, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);

	PyObject* elem = get_element((PyObject*) *tuple, index, index_length);
	
	if(PyTuple_Check(elem))
	{
		Py_ssize_t len;
		bool bis_1d_array, bis_fixed_dimension;
		py_tuple::get_metadata(elem, bis_1d_array, bis_fixed_dimension, len, *common_type);
		*is_fixed_dimension = bis_fixed_dimension? 1 : 0;
		*is_1d_array = bis_1d_array? 1 : 0;
		*is_manually_construct_array = 0;
		return len;
	}
	else if(PyList_Check(elem))
	{
		Py_ssize_t len;
		bool bis_1d_array, bis_fixed_dimension;
		py_list::get_metadata(elem, bis_1d_array, bis_fixed_dimension, len, *common_type);
		*is_fixed_dimension = bis_fixed_dimension? 1 : 0;
		*is_1d_array = bis_1d_array? 1 : 0;
		*is_manually_construct_array = 0;
		return len;
	}
	else if(PyBytes_Check(elem))
	{
		py_bytes bytes(elem);
		*is_fixed_dimension = 1;
		*is_1d_array = 1;
		*common_type = metaffi_uint8_type;
		*is_manually_construct_array = 1;
		return bytes.size();
	}
	else
	{
		std::stringstream ss;
		ss << "Expected list, tuple or bytes. Received: " << elem->ob_type->tp_name;
		throw std::runtime_error(ss.str());
	}
}

DLL_PRIVATE void py_construct_cdt_array(const metaffi_size* index, metaffi_size index_length, cdts* manually_fill_array, void* context)
{
	auto* tuple = static_cast<py_tuple*>(context);
	PyObject* elem = get_element((PyObject*) *tuple, index, index_length);
	
	if(!PyBytes_Check(elem))
	{
		std::stringstream ss;
		ss << "Expected bytes. Received: " << elem->ob_type->tp_name;
		throw std::runtime_error(ss.str());
	}
	
	py_bytes bytes(elem);
	const uint8_t* data = (const uint8_t*)bytes;
	manually_fill_array->fixed_dimensions = 1;
	for(metaffi_size i = 0; i < manually_fill_array->length; i++)
	{
		(*manually_fill_array)[i].type = metaffi_uint8_type;
		(*manually_fill_array)[i].cdt_val.uint8_val = data[i];
	}
}

DLL_PRIVATE metaffi_size py_on_get_root_elements_count(void* context)
{
	return static_cast<py_tuple*>(context)->size();
}

DLL_PRIVATE metaffi_type_info py_on_get_type_info(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return metaffi_type_info{ py_object::get_metaffi_type(elem) };
}

DLL_PRIVATE metaffi_float64 py_on_construct_float64(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_float64) py_float(elem);
}

DLL_PRIVATE metaffi_float32 py_on_construct_float32(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_float32) py_float(elem);
}

DLL_PRIVATE metaffi_int8 py_on_construct_int8(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_int8) py_int(elem);
}

DLL_PRIVATE metaffi_uint8 py_on_construct_uint8(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	if(py_bytes::check(elem))
	{
		// get the byte at the index
		return (metaffi_int8) py_bytes(elem)[index[index_size - 1]];
	}
	else
	{
		return (metaffi_int8) py_int(elem);
	}
}

DLL_PRIVATE metaffi_int16 py_on_construct_int16(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_int16) py_int(elem);
}

DLL_PRIVATE metaffi_uint16 py_on_construct_uint16(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_uint16) py_int(elem);
}

DLL_PRIVATE metaffi_int32 py_on_construct_int32(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_int32) py_int(elem);
}

DLL_PRIVATE metaffi_uint32 py_on_construct_uint32(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_uint32) py_int(elem);
}

DLL_PRIVATE metaffi_int64 py_on_construct_int64(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_int64) py_int(elem);
}

DLL_PRIVATE metaffi_uint64 py_on_construct_uint64(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_uint64) py_int(elem);
}

DLL_PRIVATE metaffi_bool py_on_construct_bool(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	return (metaffi_bool) py_bool(elem);
}

DLL_PRIVATE metaffi_char8 py_on_construct_char8(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	std::u8string str = py_str(elem).to_utf8();
	char8_t c = str.at(index[index_size - 1]);

	int8_t num_of_bytes = metaffi_char8::num_of_bytes(&c);

	switch(num_of_bytes)
	{
		case 1: {
			metaffi_char8 res{};
			res.c[0] = c;
			return res;
		}
		case 2: {
			metaffi_char8 res{};
			res.c[0] = c;
			res.c[1] = str.at(index[index_size - 1] + 1);
			return res;
		}
		case 3: {
			metaffi_char8 res{};
			res.c[0] = c;
			res.c[1] = str.at(index[index_size - 1] + 1);
			res.c[2] = str.at(index[index_size - 1] + 2);
			return res;
		}
		case 4: {
			metaffi_char8 res{};
			res.c[0] = c;
			res.c[1] = str.at(index[index_size - 1] + 1);
			res.c[2] = str.at(index[index_size - 1] + 2);
			res.c[3] = str.at(index[index_size - 1] + 3);
			return res;
		}
		default: {
			throw std::runtime_error("Invalid number of bytes for char8");
		}
	}
}

DLL_PRIVATE metaffi_string8 py_on_construct_string8(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	*is_free_required = 1;
	return (metaffi_string8) py_str(elem).to_utf8();
}

DLL_PRIVATE metaffi_char16 py_on_construct_char16(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	std::u16string str = py_str(elem).to_utf16();
	char16_t c = str.at(index[index_size - 1]);

	int8_t num_of_bytes = metaffi_char16::num_of_bytes(&c);

	switch(num_of_bytes)
	{
		case 2: {
			metaffi_char16 res{};
			res.c[0] = c;
			return res;
		}
		case 4: {
			metaffi_char16 res{};
			res.c[0] = c;
			res.c[1] = str.at(index[index_size - 1] + 1);
			return res;
		}
		default: {
			throw std::runtime_error("Invalid number of bytes for char16");
		}
	}
}

DLL_PRIVATE metaffi_string16 py_on_construct_string16(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	*is_free_required = 1;
	return (metaffi_string16) py_str(elem).to_utf16();
}

DLL_PRIVATE metaffi_char32 py_on_construct_char32(const metaffi_size* index, metaffi_size index_size, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	std::u32string str = py_str(elem).to_utf32();
	metaffi_char32 c;
	c.c = str.at(index[index_size - 1]);
	return c;
}

DLL_PRIVATE metaffi_string32 py_on_construct_string32(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	*is_free_required = 1;
	return (metaffi_string32) py_str(elem).to_utf32();
}

DLL_PRIVATE void py_object_releaser(cdt_metaffi_handle* obj)
{
	Py_DECREF((PyObject*)obj->handle);
}

DLL_PRIVATE cdt_metaffi_handle* py_on_construct_handle(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	PyObject* elem = get_element((PyObject*) *static_cast<py_tuple*>(context), index, index_size);
	*is_free_required = 1;
	if(py_metaffi_handle::check(elem))
	{
		return py_metaffi_handle(elem).as_cdt_metaffi_handle();
	}
	else// different object - wrap in metaffi_handle
	{
		Py_INCREF(elem);
		return new cdt_metaffi_handle{elem, PYTHON312_RUNTIME_ID, py_object_releaser };
	}
}

DLL_PRIVATE cdt_metaffi_callable* py_on_construct_callable(const metaffi_size* index, metaffi_size index_size, metaffi_bool* is_free_required, void* context)
{
	// TODO: implement
	throw std::runtime_error("cannot export callable from Python3.11 - yet");
}

DLL_PRIVATE metaffi::runtime::construct_cdts_callbacks py_get_construct_cdts_callbacks(void* context)
{
	metaffi::runtime::construct_cdts_callbacks callbacks{};
	callbacks.context = context;
	callbacks.get_array_metadata = &py_get_array_metadata;
	callbacks.construct_cdt_array = &py_construct_cdt_array;
	callbacks.get_root_elements_count = &py_on_get_root_elements_count;
	callbacks.get_type_info = &py_on_get_type_info;
	callbacks.get_float64 = &py_on_construct_float64;
	callbacks.get_float32 = &py_on_construct_float32;
	callbacks.get_int8 = &py_on_construct_int8;
	callbacks.get_uint8 = &py_on_construct_uint8;
	callbacks.get_int16 = &py_on_construct_int16;
	callbacks.get_uint16 = &py_on_construct_uint16;
	callbacks.get_int32 = &py_on_construct_int32;
	callbacks.get_uint32 = &py_on_construct_uint32;
	callbacks.get_int64 = &py_on_construct_int64;
	callbacks.get_uint64 = &py_on_construct_uint64;
	callbacks.get_bool = &py_on_construct_bool;
	callbacks.get_char8 = &py_on_construct_char8;
	callbacks.get_string8 = &py_on_construct_string8;
	callbacks.get_char16 = &py_on_construct_char16;
	callbacks.get_string16 = &py_on_construct_string16;
	callbacks.get_char32 = &py_on_construct_char32;
	callbacks.get_string32 = &py_on_construct_string32;
	callbacks.get_handle = &py_on_construct_handle;
	callbacks.get_callable = &py_on_construct_callable;

	return callbacks;
}


//--------------------------------------------------------------------
cdts_python3::cdts_python3(cdts& pcdts) : pcdts(pcdts)
{
}

//--------------------------------------------------------------------
py_tuple cdts_python3::to_py_tuple()
{
	int cdts_length = pcdts.length;

	py_tuple res((Py_ssize_t) cdts_length);
	traverse_cdts(pcdts, py_get_traverse_cdts_callback(&res));

	return res;
}

//--------------------------------------------------------------------
void cdts_python3::to_cdts(PyObject* pyobject_or_tuple, metaffi_type_info* expected_types, int expected_types_length)
{
	py_tuple pyobjs = expected_types_length <= 1 && !PyTuple_Check(pyobject_or_tuple) ? py_tuple(&pyobject_or_tuple, 1)
	                                                                                  : py_tuple(pyobject_or_tuple);
	pyobjs.inc_ref();// TODO: (is it correct?) prevent py_tuple from releasing the object

	if(pyobjs.size() != expected_types_length)
	{
		throw std::runtime_error("tuple and tuple_types have different lengths");
	}

	construct_cdts(pcdts, py_get_construct_cdts_callbacks(&pyobjs));
}
//--------------------------------------------------------------------