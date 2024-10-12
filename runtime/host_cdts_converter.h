#pragma once
#include <runtime/cdt.h>

// In order not to require python debug library
#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

extern "C"
{
	cdts* convert_host_params_to_cdts(PyObject* params_names, metaffi_type_info* param_metaffi_types, metaffi_size params_count, metaffi_size return_values_size);
	PyObject* convert_host_return_values_from_cdts(cdts* pcdts, uint64_t index);
}

