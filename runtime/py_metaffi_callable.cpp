#ifdef _WIN32
#include <corecrt.h> // <-- required as a python bug workaround (https://github.com/microsoft/onnxruntime/issues/9735)
#endif

#include "py_metaffi_callable.h"
#include "utils.h"
#include "py_int.h"
#include "py_tuple.h"


PyObject* py_metaffi_callable::create_lambda = nullptr;

py_metaffi_callable::py_metaffi_callable() : py_object()
{
	// initialize
	if(!create_lambda)
	{
		PyObject* main_module = PyImport_ImportModule("__main__");  // Get the main module
		PyObject* global_dict = PyModule_GetDict(main_module);  // Get the global dictionary
		
		// Get the create_lambda function
		PyObject* pyFunc = PyDict_GetItemString(global_dict, "create_lambda");
		
		if(!pyFunc)
		{
			throw std::runtime_error("failed to create or import create_lambda python function. The function is defined in metaffi pypi package in __init__.py");
		}
		
		if(!PyCallable_Check(pyFunc))
		{
			throw std::runtime_error("create_lambda is not callable - something is wrong");
		}
		
		create_lambda = pyFunc;
	}
}

py_metaffi_callable::py_metaffi_callable(const cdt_metaffi_callable& cdt_callable) : py_metaffi_callable()
{
	// Convert the void* to PyObject*
	py_int py_pxcall(((void**)cdt_callable.val)[0]);
	py_int py_pcontext(((void**)cdt_callable.val)[1]);
	
	// Convert the metaffi_type arrays to PyTuple*
	py_tuple py_param_types(cdt_callable.params_types_length);
	for (int i = 0; i < cdt_callable.params_types_length; i++) {
		py_param_types.set_item(i, py_int(cdt_callable.parameters_types[i]).detach());
	}
	
	py_tuple py_retval_types(cdt_callable.retval_types_length);
	for (int i = 0; i < cdt_callable.retval_types_length; i++) {
		py_retval_types.set_item(i, py_int(cdt_callable.retval_types[i]).detach());
	}
	
	// call create_lambda python function
	
	py_tuple argsTuple(4);  // Create a tuple that holds the arguments
	
	argsTuple.set_item(0, py_pxcall.detach());
	argsTuple.set_item(1, py_pcontext.detach());
	argsTuple.set_item(2, py_param_types.detach());
	argsTuple.set_item(3, py_retval_types.detach());
	
	// Call "create_lambda"
	PyObject* lambda_to_mffi_callable = PyObject_CallObject(create_lambda, (PyObject*)argsTuple);
	std::string err = check_python_error();
	if(!err.empty())
	{
		throw std::runtime_error(err);
	}
	
	instance = lambda_to_mffi_callable;
}

py_metaffi_callable::py_metaffi_callable(py_metaffi_callable&& other) noexcept : py_object(std::move(other))
{
}

py_metaffi_callable& py_metaffi_callable::operator=(const py_metaffi_callable& other)
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

bool py_metaffi_callable::check(PyObject* obj)
{
	// TODO: Implement this
	std::cout << "+++ type name: " << obj->ob_type->tp_name << std::endl;
	return false;
}