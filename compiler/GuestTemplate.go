package main

const GuestHeaderTemplate = `
# Code generated by MetaFFI. Modify only in marked places.
# Guest code for {{.IDLFilenameWithExtension}}
`

const GuestImportsTemplate = `
import traceback
import sys
import platform
import os
from typing import Any
from ctypes import *

{{range $mindex, $i := .Imports}}
import {{$i}}{{end}}
`

const GuestHelperFunctions = `
python_plugin_handle = None
def load_python_plugin():
	global python_plugin_handle
	
	if python_plugin_handle == None:
		python_plugin_handle = cdll.LoadLibrary(get_filename_to_load('xllr.python312'))
		python_plugin_handle.set_entrypoint.argstype = [c_char_p, c_void_p]
		python_plugin_handle.xcall_params_ret.argstype = [c_int, py_object, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong)]
		python_plugin_handle.xcall_params_no_ret.argstype = [c_int, py_object, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong)]
		python_plugin_handle.xcall_no_params_ret.argstype = [c_int, py_object, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong)]
		python_plugin_handle.xcall_no_params_no_ret.argstype = [c_int, py_object, POINTER(c_char_p), POINTER(c_ulonglong)]

def get_filename_to_load(fname):
	osname = platform.system()
	if osname == 'Windows':
		return os.getenv('METAFFI_HOME')+'\\'+ fname + '.dll'
	elif osname == 'Darwin':
		return os.getenv('METAFFI_HOME')+'/' + fname + '.dylib'
	else:
		return os.getenv('METAFFI_HOME')+'/' + fname + '.so' # for everything that is not windows or mac, return .so

def dynamicTypeToMetaFFIType(obj):
	if isinstance(obj, float):
		return {{GetMetaFFITypeFromPyType "float"}}
	elif isinstance(obj, str):
		return {{GetMetaFFITypeFromPyType "str"}}
	elif isinstance(obj, int):
		return {{GetMetaFFITypeFromPyType "int"}}
	elif isinstance(obj, bool):
		return {{GetMetaFFITypeFromPyType "bool"}}
	else:
		return {{GetMetaFFITypeFromPyType "handle"}}


load_python_plugin()
`

const GuestFunctionXLLRTemplate = `
{{range $mindex, $m := .Modules}}

{{range $findex, $f := $m.Globals}}
{{if $f.Getter}}{{$retvalLength := len $f.Getter.ReturnValues}}
{{GenerateCEntryPoint $f.Getter.GetNameWithOverloadIndex $f.Getter.Parameters $f.Getter.ReturnValues false 0}}
def EntryPoint_{{$f.Getter.Name}}{{$f.Getter.GetOverloadIndexIfExists}}():
	ret_val_types = ({{range $index, $elem := $f.Getter.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem false}}{{end}}{{if eq $retvalLength 1}},{{end}})
	return (None, ret_val_types, {{$f.Getter.EntityPath.module}}.{{$f.Name}})

{{end}}{{/* end getter */}}
{{if $f.Setter}}{{$retvalLength := len $f.Setter.ReturnValues}}
{{GenerateCEntryPoint $f.Setter.GetNameWithOverloadIndex $f.Setter.Parameters $f.Setter.ReturnValues false 0}}
def EntryPoint_{{$f.Setter.Name}}{{$f.Setter.GetOverloadIndexIfExists}}(*handle):
	ret_val_types = ({{range $index, $elem := $f.Setter.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem false}}{{end}}{{if eq $retvalLength 1}},{{end}})
	if len(handle) != 1:
		raise ValueError('received parameter in {{$f.Setter.Name}} expects exactly one parameter')
	{{$f.Setter.EntityPath.module}}.{{$f.Name}} = handle[0]
	return (None, ret_val_types)

{{end}}{{/* end setter */}}
{{end}}{{/* end globals */}}


{{range $findex, $f := $m.Functions}}
# Call to foreign {{$f.Name}}
{{GenerateCEntryPoint $f.GetNameWithOverloadIndex $f.Parameters $f.ReturnValues false 0}}
def EntryPoint_{{$f.Name}}{{$f.GetOverloadIndexIfExists}}(*vals, **named_vals):
	try:
		# call function
		{{range $index, $elem := $f.ReturnValues}}{{if $index}},{{end}}{{$elem.Name}}{{end}}{{if $f.ReturnValues}} = {{end}}{{$f.EntityPath.module}}.{{$f.Name}}(*vals, **named_vals)
		{{$retvalLength := len $f.ReturnValues}}
		ret_val_types = ({{range $index, $elem := $f.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem false}}{{end}}{{if eq $retvalLength 1}},{{end}})

		return ( None, ret_val_types {{range $index, $elem := $f.ReturnValues}}, {{$elem.Name}}{{end}})

	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)

{{end}}{{/* End function */}}

{{range $classindex, $c := $m.Classes}}
{{range $cstrindex, $f := $c.Constructors}}
{{GenerateCEntryPoint (print $c.Name "_" $f.GetNameWithOverloadIndex) $f.Parameters $f.ReturnValues true 0}}
def EntryPoint_{{$c.Name}}_{{$f.Name}}{{$f.GetOverloadIndexIfExists}}(*vals, **named_vals):
	try:
		# call constructor
		{{range $index, $elem := $f.ReturnValues}}{{if $index}},{{end}}{{$elem.Name}}{{end}}{{if $f.ReturnValues}} = {{end}}{{$f.EntityPath.module}}.{{$f.Name}}(*vals, **named_vals)
		
		{{$retvalLength := len $f.ReturnValues}}
		ret_val_types = ({{range $index, $elem := $f.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem false}}{{end}}{{if eq $retvalLength 1}},{{end}})

		return ( None, ret_val_types {{range $index, $elem := $f.ReturnValues}}, {{$elem.Name}}{{end}})

	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)
{{end}}{{/* End Constructors */}}

{{range $findex, $f := $c.Fields}}
{{if $f.Getter}}
{{GenerateCEntryPoint (print $c.Name "_" $f.Getter.GetNameWithOverloadIndex) $f.Getter.Parameters $f.Getter.ReturnValues true 0}}
def EntryPoint_{{$c.Name}}_{{$f.Getter.Name}}{{$f.Getter.GetOverloadIndexIfExists}}(*obj):
	try:

		{{$retvalLength := len $f.Getter.ReturnValues}}
		ret_val_types = ({{range $index, $elem := $f.Getter.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem true}}{{end}}{{if eq $retvalLength 1}},{{end}})

		if len(obj) != 1:
			raise ValueError('received parameter in {{$c.Name}}_{{$f.Getter.Name}} expects exactly one parameter')

		return (None, ret_val_types, obj[0].{{$f.Name}})
	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)

{{end}}{{/* End Getter */}}
{{if $f.Setter}}
{{GenerateCEntryPoint (print $c.Name "_" $f.Setter.GetNameWithOverloadIndex) $f.Setter.Parameters $f.Setter.ReturnValues true 0}}
def EntryPoint_{{$c.Name}}_{{$f.Setter.Name}}{{$f.Setter.GetOverloadIndexIfExists}}(*vals):
	try:

		if len(vals) != 2:
			raise ValueError('received parameters in {{$c.Name}}_{{$f.Setter.Name}} expects exactly two parameters')

		vals[0].{{$f.Name}} = vals[1]

		{{$retvalLength := len $f.Setter.ReturnValues}}
		ret_val_types = ({{range $index, $elem := $f.Setter.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem true}}{{end}}{{if eq $retvalLength 1}},{{end}})

		return (None, ret_val_types)
	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)

{{end}}{{/* End Setter */}}
{{end}}{{/* End Fields */}}

{{range $methindex, $f := $c.Methods}}
{{GenerateCEntryPoint (print $c.Name "_" $f.GetNameWithOverloadIndex) $f.Parameters $f.ReturnValues true 0}}
def EntryPoint_{{$c.Name}}_{{$f.Name}}{{$f.GetOverloadIndexIfExists}}(*vals, **named_vals):
	try:
		# call method
		{{$ParamsLength := len $f.Parameters}}
		{{if gt $ParamsLength 1}}
		{{range $index, $elem := $f.ReturnValues}}{{if $index}},{{end}}{{$elem.Name}}{{end}}{{if $f.ReturnValues}} = {{end}}vals[0].{{$f.Name}}(*vals[1:], **named_vals)
		{{else}}
		{{range $index, $elem := $f.ReturnValues}}{{if $index}},{{end}}{{$elem.Name}}{{end}}{{if $f.ReturnValues}} = {{end}}vals[0].{{$f.Name}}(**named_vals)
		{{end}}
		
		{{$retvalLength := len $f.ReturnValues}}
		ret_val_types = ({{range $index, $elem := $f.ReturnValues}}{{if $index}}, {{end}}{{GetMetaFFIType $elem false}}{{end}}{{if eq $retvalLength 1}},{{end}})

		return ( None, ret_val_types {{range $index, $elem := $f.ReturnValues}}, {{$elem.Name}}{{end}})
		
	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)
	
{{end}}{{/* End methods */}}

{{if $c.Releaser}}
{{GenerateCEntryPoint (print $c.Name "_" $c.Releaser.GetNameWithOverloadIndex) $c.Releaser.Parameters $c.Releaser.ReturnValues true 0}}
def EntryPoint_{{$c.Name}}_{{$c.Releaser.GetNameWithOverloadIndex}}{{$c.Releaser.GetOverloadIndexIfExists}}(*vals):
	try:

		if len(vals) != 1:
			raise ValueError('received parameter in {{$c.Name}}_{{$c.Releaser.GetNameWithOverloadIndex}} expects exactly one parameter')

		# xcall release object
		python_plugin_handle.release_object(vals[0])
	except Exception as e:
		errdata = traceback.format_exception(*sys.exc_info())
		return ('\n'.join(errdata),)
{{end}}{{/* End Releaser */}}

{{end}}{{/* End Classes */}}

{{end}}{{/* End modules */}}
`
