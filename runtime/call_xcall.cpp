#include "call_xcall.h"
#include "host_cdts_converter.h"
#include "py_tuple.h"
#include "utils.h"
#include <stdexcept>


PyObject* call_xcall(void* pxcall_ptr, void* context, PyObject* param_metaffi_types, PyObject* retval_metaffi_types, PyObject* args)
{
	if(pxcall_ptr == nullptr)
	{
		PyErr_SetString(PyExc_ValueError, "xcall is null");
		return Py_None;
	}

	xcall pxcall(pxcall_ptr, context);

	pyscope();
	
	Py_ssize_t retval_count = py_tuple::get_size(retval_metaffi_types);
	
	py_tuple param_metaffi_types_tuple(param_metaffi_types);
	Py_ssize_t params_count = param_metaffi_types_tuple.size();

	std::vector<metaffi_type_info> param_metaffi_types_vec(params_count);
	
	for(int i=0 ; i<params_count ; i++)
	{
		PyObject* item = param_metaffi_types_tuple[i];
		if(strcmp(item->ob_type->tp_name, "metaffi_type_info") == 0)
		{
			// Get the fields of the metaffi_type_info instance
			PyObject* type = PyObject_GetAttrString(item, "type");
			PyObject* alias = PyObject_GetAttrString(item, "alias");
			PyObject* dimensions = PyObject_GetAttrString(item, "fixed_dimensions");
		
			// Convert the fields to C types
			uint64_t type_c = PyLong_AsUnsignedLongLong(type);
			char* alias_c = Py_IsNone(alias) ? nullptr : (char*)PyUnicode_AsUTF8(alias);
			int64_t dimensions_c = PyLong_AsLong(dimensions);
		
			if(PyErr_Occurred())
			{
				return Py_None;
			}
			
			// Create a metaffi_type_info struct and populate it with the field values
			metaffi_type_info info;
			info.type = type_c;
			info.alias = alias_c;
			info.is_free_alias = alias_c != nullptr ? 1 : 0;
			info.fixed_dimensions = dimensions_c;
		
			param_metaffi_types_vec[i] = std::move(info);
		
			// Decrement the reference counts of the field objects
			Py_DECREF(type);
			Py_DECREF(alias);
			Py_DECREF(dimensions);
			
		}
		else if(PyLong_Check(item))
		{
			uint64_t type_c = PyLong_AsUnsignedLongLong(item);
			metaffi_type_info info;
			info.type = type_c;
			info.alias = nullptr;
			info.is_free_alias = 0;
			info.fixed_dimensions = 0;
			param_metaffi_types_vec[i] = std::move(info);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError, "expected metaffi_type_info");
			return Py_None;
		}
	}
	
	if (params_count > 0 || retval_count > 0)
	{
		cdts* pcdts = convert_host_params_to_cdts(args, param_metaffi_types_vec.data(), params_count, retval_count);
		if(pcdts == nullptr)
		{
			return Py_None;
		}

		char* out_err = nullptr;
		
		pxcall(pcdts, &out_err);

		if(out_err)
		{
			PyErr_SetString(PyExc_ValueError, out_err);
			xllr_free_string(out_err);
			return Py_None;
		}

		if(retval_count == 0) {
			return Py_None;
		}

		return convert_host_return_values_from_cdts(pcdts, 1);
	}
	else
	{
		char* out_err = nullptr;
		pxcall(&out_err);
		if (out_err)
		{
			PyErr_SetString(PyExc_ValueError, out_err);
			xllr_free_string(out_err);
			return Py_None;
		}

		return Py_None;
	}
}

