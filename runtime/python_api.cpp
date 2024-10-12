#include "cdts_python3.h"
#include "metaffi_package_importer.h"
#include "py_list.h"
#include "py_tuple.h"
#include "utils.h"
#include <boost/filesystem.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <map>
#include <regex>
#include <runtime/runtime_plugin_api.h>
#include <sstream>
#include <utility>
#include <utils/foreign_function.h>
#include <utils/entity_path_parser.h>

#ifdef _DEBUG
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif


using namespace metaffi::utils;

#define handle_err(err, desc)                           \
	{                                                   \
		auto err_len = strlen(desc);                    \
		*err = (char*)xllr_alloc_string(desc, err_len); \
	}

#define handle_err_str(err, descstr)                        \
	{                                                       \
		auto err_len = descstr.length();                    \
		*err = xllr_alloc_string(descstr.c_str(), err_len); \
	}

#define handle_py_err(err)                   \
	std::string pyerr(check_python_error()); \
	handle_err_str(err, pyerr);

#define TRUE 1
#define FALSE 0

std::vector<foreign_function_entrypoint_signature_params_ret_t> params_ret_functions;
std::vector<foreign_function_entrypoint_signature_params_no_ret_t> params_no_ret_functions;
std::vector<foreign_function_entrypoint_signature_no_params_ret_t> no_params_ret_functions;
std::vector<foreign_function_entrypoint_signature_no_params_no_ret_t> no_params_no_ret_functions;

enum attribute_action
{
	attribute_action_getter,
	attribute_action_setter
};

struct python3_context {
	attribute_action foreign_object_type;
	bool is_instance_required;
	std::vector<std::string> attribute_path;
	PyObject* entrypoint;
	PyObject* attribute_holder;
	std::vector<metaffi_type_info> params_types;
	std::vector<metaffi_type_info> retvals_types;
	bool is_varargs = false;
	bool is_named_args = false;

	[[nodiscard]] uint8_t params_count() const { return params_types.size(); };
	[[nodiscard]] uint8_t retvals_count() const { return retvals_types.size(); };

	// fills entrypoint and attribute holders
	// this returns correctly only if the context is of an attribute
	bool find_attribute_holder_and_attribute(PyObject* root)
	{
		if(attribute_path.empty())
		{
			return entrypoint && attribute_holder;
		}

		PyObject* parent = root;
		PyObject* pyobj = root;

		for(const std::string& step: attribute_path)
		{
			PyObject* temp = PyObject_GetAttrString(pyobj, step.c_str());
			parent = pyobj;
			pyobj = temp;
			if(!pyobj)
			{
				return false;
			}
		}

		entrypoint = PyUnicode_FromString(attribute_path[attribute_path.size() - 1].c_str());

		attribute_holder = parent;
		attribute_path.clear();

		return true;
	}
};
//--------------------------------------------------------------------
std::unordered_map<std::string, void*> foreign_entities;
void set_entrypoint(const char* entrypoint_name, void* pfunction)
{
	foreign_entities[entrypoint_name] = pfunction;
}
//--------------------------------------------------------------------
void initialize_environment()
{
	std::string curpath(boost::filesystem::current_path().string());

	PyObject* sys_path = PySys_GetObject("path");
	if(!sys_path)
	{
		// Handle error: sys.path not retrieved
		throw std::runtime_error("sys.path not retrieved");
	}

	PyObject* curpath_pystr = PyUnicode_FromString(curpath.c_str());
	if(PyList_Append(sys_path, curpath_pystr) == -1)
	{
		// Handle error: failed to append curpath
		throw std::runtime_error("failed to append curpath");
	}
	Py_DECREF(curpath_pystr);// Decrement reference count

	const char* metaffi_home = getenv("METAFFI_HOME");
	if(metaffi_home)
	{
		PyObject* metaffi_home_pystr = PyUnicode_FromString(metaffi_home);
		if(PyList_Append(sys_path, metaffi_home_pystr) == -1)
		{
			// Handle error: failed to append METAFFI_HOME
			throw std::runtime_error("failed to append METAFFI_HOME");
		}
		Py_DECREF(metaffi_home_pystr);// Decrement reference count
		PyRun_SimpleString("import sys\nprint('Loaded python pathes:', sys.path)\n");
	}
	else
	{
		// Handle error: METAFFI_HOME not set
		throw std::runtime_error("METAFFI_HOME not set");
	}

	// add "site.getusersitepackages" and "site.getsitepackages" to sys.path
	const char* code = R"(
import site
import sys 

# Split the existing sys.path into a set for faster membership checks
existing_paths = set(sys.path)

# Add any missing site-packages directories to sys.path
site_dir = site.getusersitepackages()
if site_dir not in existing_paths:
	sys.path.append(site_dir)
	print(f"Added '{site_dir}' to sys.path")

for site_dir in site.getsitepackages():
	if site_dir not in existing_paths:
		sys.path.append(site_dir)
		print(f"Added '{site_dir}' to sys.path")

print('Loaded python pathes: ', sys.path)
)";

	PyRun_SimpleString(code);
	if(PyErr_Occurred())
	{
		PyErr_Print();
		PyErr_Clear();

		throw std::runtime_error("Failed to add site-packages directories to sys.path");
	}

	import_metaffi_package();
}
//--------------------------------------------------------------------
PyThreadState* _save = nullptr;
bool g_loaded = false;
bool g_loaded_embeded = false;
void load_runtime(char** err)
{
	if(g_loaded)
	{
		return;
	}

	// load python runtime
	if(!Py_IsInitialized())
	{
		Py_InitializeEx(0);// Do not install signal handlers

		// https://stackoverflow.com/questions/75846775/embedded-python-3-10-py-finalizeex-hangs-deadlock-on-threading-shutdown/
		PyEval_SaveThread();

		g_loaded_embeded = true;
	}

	auto gil = PyGILState_Ensure();

	try
	{
		initialize_environment();
	}
	catch(std::exception& e)
	{
		auto err_len = strlen(e.what());
		*err = (char*)calloc(sizeof(char), err_len + 1);
		strncpy(*err, e.what(), err_len);
	}
	

	g_loaded = true;

	//_save = PyEval_SaveThread();
	PyGILState_Release(gil);
}


//--------------------------------------------------------------------
void free_runtime(char** err)
{
	// TODO: return errors into "err"

	// if MetaFFI was loaded into a running Python interpreter, we should not finalize it
	if(!g_loaded_embeded)
	{
		return;
	}

	// Ensure we have the GIL before we interact with Python
	PyGILState_STATE gstate = PyGILState_Ensure();

	// Import the threading module
	PyObject* threadingModule = PyImport_ImportModule("threading");
	if(!threadingModule)
	{
		PyErr_Print();
		PyGILState_Release(gstate);
		return;
	}

	// Get the active count of threads
	PyObject* activeCount = PyObject_CallMethod(threadingModule, "active_count", NULL);
	if(!activeCount)
	{
		PyErr_Print();
		Py_DECREF(threadingModule);
		PyGILState_Release(gstate);
		return;
	}

	long count = PyLong_AsLong(activeCount);
	Py_DECREF(activeCount);

	if(count > 1)
	{
		printf("More than one thread is active.\n");
	}
	else if(count == 1)
	{
		PyObject* currentThread = PyObject_CallMethod(threadingModule, "current_thread", NULL);
		PyObject* currentThreadId = PyObject_GetAttrString(currentThread, "ident");
		long currentThreadIdValue = PyLong_AsLong(currentThreadId);

		PyObject* mainThreadIdObj = PyObject_CallMethod(threadingModule, "main_thread", NULL);
		PyObject* mainThreadId = PyObject_GetAttrString(mainThreadIdObj, "ident");
		long mainThreadIdValue = PyLong_AsLong(mainThreadId);

		if(currentThreadIdValue == mainThreadIdValue)
		{
			printf("Only the main thread is active. Finalizing Python.\n");

			// Correctly release Python objects before finalizing
			Py_DECREF(currentThread);
			Py_DECREF(currentThreadId);
			Py_DECREF(mainThreadIdObj);
			Py_DECREF(mainThreadId);
			Py_DECREF(threadingModule);

			// Now it's safe to finalize Python
			int res = Py_FinalizeEx();
			if(res < 0)
			{
				// Handle error during finalization
				PyErr_Print();
			}
		}
		else
		{
			printf("A non-main thread is still active.\n");

			// Release resources if not finalizing
			Py_DECREF(currentThread);
			Py_DECREF(currentThreadId);
			Py_DECREF(mainThreadIdObj);
			Py_DECREF(mainThreadId);
			Py_DECREF(threadingModule);
			PyGILState_Release(gstate);
		}
	}
	else
	{
		// Release threadingModule if no action is taken
		Py_DECREF(threadingModule);
		PyGILState_Release(gstate);
	}
}
//--------------------------------------------------------------------
void free_xcall(xcall* pxcall, char** err)
{
	void** pxcall_and_context = pxcall->pxcall_and_context;

	delete((python3_context*)pxcall_and_context[1]);// delete context
	pxcall_and_context[1] = nullptr;
	pxcall_and_context[0] = nullptr;

	delete pxcall;

	*err = nullptr;
}
//--------------------------------------------------------------------
void organize_arguments(int is_method, PyObject* params_tuple, PyObject*& out_params_tuple, PyObject*& out_kwargs_dict, bool is_varargs, bool is_kwargs)
{
	out_params_tuple = nullptr;
	out_kwargs_dict = nullptr;

	// place all positional arguments in a tuple
	Py_ssize_t all_args_count = PyTuple_Size(params_tuple);

	// if no arguments OR no varargs and no kwargs - set the tuple and return
	if(all_args_count == 0 || (!is_varargs && !is_kwargs))
	{
		out_params_tuple = params_tuple;
		return;
	}

	// either varargs or kwargs is true

	// if is_varargs, the last argument is a list (or a tuple?!) - it is positional arguments
	PyObject* varargs = nullptr;
	if(is_varargs)
	{
		// the last parameter is a list of positional arguments
		PyObject* last_arg = PyTuple_GetItem(params_tuple, all_args_count - 1);
		if(strcmp(last_arg->ob_type->tp_name, "list") == 0)
		{
			varargs = last_arg;
		}
		// if the last argument is not a list AND is_kwargs is true, maybe the one before the last arg is the list
		else if(is_kwargs && all_args_count > 1)
		{
			PyObject* before_last_arg = PyTuple_GetItem(params_tuple, all_args_count - 2);
			if(strcmp(before_last_arg->ob_type->tp_name, "list") == 0)
			{
				varargs = before_last_arg;
			}
		}

		// if not both, there's no varargs used
	}

	// if is_kwargs is true
	// if the last argument is a "dict", set it to kwargs
	if(is_kwargs)
	{
		PyObject* last_arg = PyTuple_GetItem(params_tuple, all_args_count - 1);
		if(strcmp(last_arg->ob_type->tp_name, "dict") == 0)// last argument is dict - that is kwargs
		{
			out_kwargs_dict = last_arg;
		}
	}


	// if both varargs and kwargs are NOT used, return the tuple as is
	if(!varargs && !out_kwargs_dict)
	{
		out_params_tuple = params_tuple;
		return;
	}


	// if varargs is used:
	// if kwargs is used, copy to a new tuple the first n-2 arguments
	// otherwise, copy to a new tuple the first n-1 arguments

	// calculate the new size of the tuple
	Py_ssize_t size_without_varargs_kwargs = all_args_count - (varargs ? 1 : 0) - (out_kwargs_dict ? 1 : 0);
	Py_ssize_t new_size = size_without_varargs_kwargs;
	Py_ssize_t varargs_size = 0;
	if(varargs)
	{
		if(PyTuple_Check(varargs))
		{
			varargs_size = PyTuple_Size(varargs);
		}
		else if(PyList_Check(varargs))
		{
			varargs_size = PyList_Size(varargs);
		}
		else
		{
			throw std::runtime_error("varargs should be either a list or a tuple");
		}
	}

	if(varargs_size == -1)
	{
		std::string msg = check_python_error();
		std::stringstream ss;

		ss << "Failed to get the size of varargs";
		if(!msg.empty())
		{
			ss << ": " << msg;
		}

		throw std::runtime_error(ss.str());
	}

	new_size += varargs_size;

	// Copy non-varargs and non-kwargs arguments to a new tuple
	out_params_tuple = PyTuple_New(new_size);
	for(Py_ssize_t i = 0; i < size_without_varargs_kwargs; i++)
	{
		PyObject* cur_arg = PyTuple_GetItem(params_tuple, i);
		Py_INCREF(cur_arg);
		PyTuple_SetItem(out_params_tuple, i, cur_arg);
	}

	// if varargs is used, copy the varargs to the new tuple
	if(varargs)
	{
		for(Py_ssize_t i = 0; i < varargs_size; i++)
		{
			bool is_tuple = PyTuple_Check(varargs);

			PyObject* cur_arg = is_tuple ? PyTuple_GetItem(varargs, i) : PyList_GetItem(varargs, i);
			Py_INCREF(cur_arg);

			PyTuple_SetItem(out_params_tuple, size_without_varargs_kwargs + i, cur_arg);
		}
	}
}
//--------------------------------------------------------------------
void pyxcall_params_ret(
        python3_context* pctxt,
        cdts params_ret[2],
        char** out_err)
{
	try
	{
		pyscope();

		// convert CDT to Python3
		cdts_python3 params_cdts(params_ret[0]);
		py_tuple params = params_cdts.to_py_tuple();


		// if parameter is of type "metaffi_positional_args" - pass internal tuple as "positional args"
		// if parameter is of type "metaffi_keyword_args" - pass dict as "keyword args"
		PyObject* out_params = nullptr;
		PyObject* out_kwargs = nullptr;

		organize_arguments(pctxt->is_instance_required, (PyObject*)params, out_params, out_kwargs, pctxt->is_varargs, pctxt->is_named_args);

		// call function or set variable
		PyObject* res = nullptr;
		if(PyCallable_Check(pctxt->entrypoint))
		{
			if(out_params || out_kwargs)
			{
				res = PyObject_Call(pctxt->entrypoint, out_params, out_kwargs);
			}
			else
			{
				// in case of function with default values and there are no actuals from caller
				res = PyObject_CallObject(pctxt->entrypoint, nullptr);
			}
		}
		else// attribute
		{
			// load attribute, if not loaded yet
			if(!pctxt->attribute_path.empty())
			{
				if(!pctxt->is_instance_required)
				{
					handle_err(out_err, "non-object attribute should have been found");
					return;
				}

				if(!pctxt->find_attribute_holder_and_attribute(params[0]))
				{
					handle_err(out_err, "attribute not found");
					return;
				}
			}

			if(pctxt->foreign_object_type == attribute_action_getter)
			{
				res = PyObject_GetAttr(pctxt->attribute_holder, pctxt->entrypoint);
			}
			else if(pctxt->foreign_object_type == attribute_action_setter)
			{
				PyObject_SetAttr(pctxt->attribute_holder, pctxt->entrypoint, params[1]);
			}
			else
			{
				handle_err(out_err, "Unknown foreign object type");
				return;
			}
		}

		std::string err_msg = std::move(check_python_error());
		if(!err_msg.empty())
		{
			handle_err_str((char**)out_err, err_msg);
			return;
		}

		// return values;
		cdts_python3 return_cdts(params_ret[1]);
		return_cdts.to_cdts(res, &pctxt->retvals_types[0], pctxt->retvals_types.size());

	} catch(std::exception& err)
	{
		auto err_len = strlen(err.what());
		*out_err = (char*)calloc(sizeof(char), err_len + 1);
		strncpy(*out_err, err.what(), err_len);
	}
}
//--------------------------------------------------------------------
// IMPORTANT: 	the name of the function must be different from
//				xcall_params_ret, as "xcall_params_ret" is exported by xllr
//				which makes linux choose xllr's function instead of this one
void py_api_xcall_params_ret(void* context, cdts params_ret[2], char** out_err)
{
	python3_context* pctxt = (python3_context*)context;
	pyxcall_params_ret(pctxt, params_ret, out_err);
}
//--------------------------------------------------------------------
void pyxcall_no_params_ret(
        python3_context* pctxt,
        cdts return_values[1],
        char** out_err)
{
	try
	{
		pyscope();

		PyObject* res = nullptr;
		if(pctxt->entrypoint && PyCallable_Check(pctxt->entrypoint))
		{
			res = PyObject_CallObject(pctxt->entrypoint, nullptr);
		}
		else
		{
			// load attribute, if not loaded yet
			if(!pctxt->attribute_path.empty())
			{
				handle_err(out_err, "non-object attribute should have been loaded") return;
			}

			if(pctxt->foreign_object_type == attribute_action_getter)
			{
				res = PyObject_GetAttr(pctxt->attribute_holder, pctxt->entrypoint);
			}
			else
			{
				handle_err(out_err, "Unexpected attribute action");
				return;
			}
		}

		// check error
		std::string err_msg = std::move(check_python_error());
		if(!err_msg.empty())
		{
			handle_err_str((char**)out_err, err_msg);
			return;
		}

		// convert results back to CDT
		// assume types are as expected
		if(!res)
		{
			std::stringstream ss;
			ss << "Expected return type. No return type returned";
			handle_err_str((char**)out_err, ss.str());
			return;
		}

		// return value;
		cdts_python3 return_cdts(return_values[1]);
		return_cdts.to_cdts(res, &pctxt->retvals_types[0], pctxt->retvals_types.size());

	} catch(std::exception& err)
	{
		auto err_len = strlen(err.what());
		*out_err = (char*)calloc(sizeof(char), err_len + 1);
		strncpy(*out_err, err.what(), err_len);
	}
}
//--------------------------------------------------------------------
void py_api_xcall_no_params_ret(void* context, cdts parameters[1], char** out_err)
{
	python3_context* pctxt = (python3_context*)context;
	pyxcall_no_params_ret(pctxt, parameters, out_err);
}
//--------------------------------------------------------------------
void pyxcall_params_no_ret(
        python3_context* pctxt,
        cdts parameters[1],
        char** out_err)
{
	try
	{
		pyscope();

		// convert CDT to Python3
		cdts_python3 params_cdts(parameters[0]);
		py_tuple params = params_cdts.to_py_tuple();

		PyObject* out_params = nullptr;
		PyObject* out_kwargs = nullptr;
		organize_arguments(pctxt->is_instance_required, (PyObject*)params, out_params, out_kwargs, pctxt->is_varargs,
		                   pctxt->is_named_args);

		// call function
		if(PyCallable_Check(pctxt->entrypoint))
		{
			if(out_params || out_kwargs)
			{
				PyObject_Call(pctxt->entrypoint, out_params, out_kwargs);
			}
			else
			{
				// in case of function with default values and there are no actual from caller
				PyObject_CallObject(pctxt->entrypoint, nullptr);
			}
		}
		else
		{
			// load attribute, if not loaded yet
			if(!pctxt->attribute_path.empty())
			{
				if(!pctxt->is_instance_required)
				{
					handle_err(out_err, "non-object attribute should have been found");
					return;
				}

				if(!pctxt->find_attribute_holder_and_attribute(params[0]))
				{
					handle_err(out_err, "attribute not found");
					return;
				}
			}

			if(pctxt->foreign_object_type == attribute_action_setter)
			{
				PyObject_SetAttr(pctxt->attribute_holder, pctxt->entrypoint,
				                 pctxt->is_instance_required ? params[1] : params[0]);
			}
			else
			{
				handle_err(out_err, "Unexpected attribute action");
				return;
			}
		}

		std::string err_msg = std::move(check_python_error());
		if(!err_msg.empty())
		{
			handle_err_str((char**)out_err, err_msg);
			return;
		}

	} catch(std::exception& err)
	{
		auto err_len = strlen(err.what());
		*out_err = (char*)calloc(sizeof(char), err_len + 1);
		strncpy(*out_err, err.what(), err_len);
	}
}
//--------------------------------------------------------------------
void py_api_xcall_params_no_ret(void* context, cdts return_values[1], char** out_err)
{
	python3_context* pctxt = (python3_context*)context;
	pyxcall_params_no_ret(pctxt, return_values, out_err);
}
//--------------------------------------------------------------------
void pyxcall_no_params_no_ret(
        python3_context* pctxt,
        char** out_err)
{
	try
	{
		pyscope();

		if(!PyCallable_Check(pctxt->entrypoint))
		{
			handle_err(out_err, "Expecting callable object");
			return;
		}

		// call function
		PyObject_CallObject(pctxt->entrypoint, nullptr);
		std::string err_msg = std::move(check_python_error());

		if(!err_msg.empty())
		{
			handle_err_str((char**)out_err, err_msg);
		}
	} 
	catch(std::exception& err)
	{
		auto err_len = strlen(err.what());
		*out_err = (char*)calloc(sizeof(char), err_len + 1);
		strncpy(*out_err, err.what(), err_len);
	}
}
//--------------------------------------------------------------------
void py_api_xcall_no_params_no_ret(void* context, char** out_err)
{
	python3_context* pctxt = (python3_context*)context;

	pyxcall_no_params_no_ret(pctxt, out_err);

}
//--------------------------------------------------------------------
xcall* make_callable(void* py_callable_as_py_object, metaffi_type_info* params_types, int8_t params_count, metaffi_type_info* retvals_types, int8_t retval_count, char** err)
{
	std::unique_ptr<python3_context> ctxt = std::make_unique<python3_context>();// should be deleted only when the function is released
	if(params_types)
	{
		ctxt->params_types.insert(ctxt->params_types.end(), params_types, params_types + params_count);
	}
	if(retvals_types)
	{
		ctxt->retvals_types.insert(ctxt->retvals_types.end(), retvals_types, retvals_types + retval_count);
	}


	ctxt->is_instance_required = false;

	ctxt->entrypoint = (PyObject*)py_callable_as_py_object;

	void* xcall_func = params_count > 0 && retval_count > 0 ? (void*)py_api_xcall_params_ret 	: params_count == 0 && retval_count > 0 ? (void*)py_api_xcall_no_params_ret
																								: params_count > 0 && retval_count == 0 ? (void*)py_api_xcall_params_no_ret
																																		: (void*)py_api_xcall_no_params_no_ret;
	xcall* pxcall = new xcall(xcall_func, ctxt.release());

	return pxcall;
}
//--------------------------------------------------------------------
xcall* load_entity(const char* module_path, const char* entity_path, metaffi_type_info* param_types, int8_t params_count, metaffi_type_info* ret_types, int8_t retval_count, char** err)
{
	if(!Py_IsInitialized())
	{
		load_runtime(err);
	}

	pyscope();

	metaffi::utils::entity_path_parser fp(entity_path);
	std::filesystem::path p(module_path);
	std::filesystem::path dir = p.parent_path();
	std::string dir_str = dir.string();

	// Escape the backslashes in the string
	PyObject* sysPath = PySys_GetObject((char*)"path");
	PyObject* path = PyUnicode_FromString(dir_str.c_str());
	if(path != nullptr)
	{
		if(PySequence_Contains(sysPath, path) == 0)
		{
			PyList_Append(sysPath, path);
		}
		Py_DECREF(path);
	}

	PyObject* pymod = PyImport_ImportModuleEx(p.stem().string().c_str(), Py_None, Py_None, Py_None);

	if(!pymod)
	{
		// error has occurred
		handle_py_err(err);
		return nullptr;
	}

	std::unique_ptr<python3_context> ctxt = std::make_unique<python3_context>();// should be deleted only when the function is released
	if(param_types)
	{
		ctxt->params_types.insert(ctxt->params_types.end(), param_types, param_types + params_count);
	}
	if(ret_types)
	{
		ctxt->retvals_types.insert(ctxt->retvals_types.end(), ret_types, ret_types + retval_count);
	}

	std::string load_symbol;
	if(fp.contains("callable"))
	{
		load_symbol = std::move(fp["callable"]);
		ctxt->is_instance_required = fp.contains("instance_required");

		std::vector<std::string> path_to_object;
		boost::split(path_to_object, load_symbol, boost::is_any_of("."));

		PyObject* pyobj = pymod;

		for(const std::string& step: path_to_object)
		{
			pyobj = PyObject_GetAttrString(pyobj, step.c_str());
			if(!pyobj)
			{
				std::stringstream ss;
				ss << "failed to find step \"" << step << "\" in the path " << load_symbol;
				handle_err_str(err, ss.str());
				return nullptr;
			}
		}

		if(fp.contains("callable") && !PyCallable_Check(pyobj))
		{
			handle_err(err, "Given callable is not PyCallable");
			return nullptr;
		}

		ctxt->is_varargs = fp.contains("varargs");
		ctxt->is_named_args = fp.contains("named_args");

		ctxt->entrypoint = pyobj;
	}
	else if(fp.contains("attribute"))
	{
		load_symbol = std::move(fp["attribute"]);
		ctxt->is_instance_required = fp.contains("instance_required");

		if(fp.contains("getter"))
		{
			ctxt->foreign_object_type = attribute_action_getter;
		}
		else if(fp.contains("setter"))
		{
			ctxt->foreign_object_type = attribute_action_setter;
		}
		else
		{
			handle_err(err, "missing getter or setter attribute in function path");
			return nullptr;
		}

		std::vector<std::string> path_to_object;
		boost::split(path_to_object, load_symbol, boost::is_any_of("."));

		ctxt->attribute_path = path_to_object;

		if(!ctxt->is_instance_required && !ctxt->find_attribute_holder_and_attribute(pymod))
		{
			handle_err(err, "attribute not found");
			return nullptr;
		}
	}
	else
	{
		handle_err(err, "expecting \"callable\" or \"variable\" in function path");
		return nullptr;
	}

	void* xcall_func = nullptr;

	if(params_count > 0 && retval_count > 0)
	{
		xcall_func = (void*)((void (*)(void* context, cdts params_ret[2], char** out_err)) & py_api_xcall_params_ret);
	}
	else if(params_count == 0 && retval_count > 0)
	{
		xcall_func = (void*)((void (*)(void* context, cdts return_values[1], char** out_err)) & py_api_xcall_no_params_ret);
	}
	else if(params_count > 0 && retval_count == 0)
	{
		xcall_func = (void*)((void (*)(void* context, cdts parameters[1], char** out_err)) py_api_xcall_params_no_ret);
	}
	else
	{
		xcall_func = (void*)((void (*)(void* context, char** out_err)) & py_api_xcall_no_params_no_ret);
	}

	xcall* pxcall = new xcall(xcall_func, ctxt.release());

	return pxcall;
}
//--------------------------------------------------------------------