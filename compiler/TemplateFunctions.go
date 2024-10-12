package main

import "C"
import (
	"fmt"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
	"os"
	"strings"
)

var templatesFuncMap = map[string]interface{}{
	"ConvertToCPythonType":         convertToCPythonType,
	"ConvertToPythonType":          convertToPythonType,
	"ConvertToPythonTypeFromField": convertToPythonTypeFromField,
	"GetEnvVar":                    getEnvVar,
	"Add":                          add,
	"GetMetaFFIType":               getMetaFFIType,
	"GenerateCodeAllocateCDTS":     generateCodeAllocateCDTS,
	"GenerateCodeXCall":            generateCodeXCall,
	"GenerateCodeReturnValues":     generateCodeReturnValues,
	"GenerateCodeGlobals":          generateCodeGlobals,
	"GenerateCodeReturn":           generateReturn,
	"GetCFuncType":                 getCFuncType,
	"GenerateCEntryPoint":          generateCEntryPoint,
	"GenerateMethodSignature":      generateMethodSignature,
	"GetMetaFFITypeFromPyType":     getMetaFFITypeFromPyType,
}

// --------------------------------------------------------------------
func generateMethodSignature(meth *IDL.MethodDefinition) string {
	//{{if $f.InstanceRequired}}self{{end}}{{range $index, $elem := $f.Parameters}}{{if gt $index 0}}{{if $index}},{{end}} {{$elem.Name}}:{{ConvertToPythonTypeFromField $elem}}{{end}}{{end}}->{{ConvertToPythonTypeFromField $restype}}
	params := make([]string, 0)
	retvals := make([]string, 0)

	for i, p := range meth.Parameters {
		if i == 0 && meth.InstanceRequired {
			params = append(params, "self")
		} else {
			params = append(params, fmt.Sprintf("%v:%v", p.Name, convertToPythonTypeFromField(p)))
		}
	}

	for _, r := range meth.ReturnValues {
		retvals = append(retvals, convertToPythonTypeFromField(r))
	}

	paramsString := strings.Join(params, ",")

	retvalString := ""
	if len(retvals) == 0 {
		retvalString = "None"
	} else if len(retvals) == 1 {
		retvalString = retvals[0]
	} else { // > 1
		retvalString = fmt.Sprintf("Tuple[%v]", strings.Join(retvals, ","))
	}

	return fmt.Sprintf("(%v)->%v", paramsString, retvalString)
}

// --------------------------------------------------------------------
func generateCEntryPoint(name string, params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition, isMethod bool, indent int) string {

	indentStr := ""
	for i := 0; i < indent; i++ {
		indentStr += "\t"
	}

	res := ""

	isMethodBool := 0
	if isMethod{
		isMethodBool = 1
	}


	if len(params) > 0 && len(retvals) > 0 {
		res += fmt.Sprintf("%v@CFUNCTYPE(None, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong))\n", indentStr)
		res += fmt.Sprintf("%vdef CEntryPoint_%v(cdts, out_err, out_err_len):\n", indentStr, name)
		res += fmt.Sprintf("%v\tglobal python_plugin_handle\n", indentStr)
		res += fmt.Sprintf("%v\tpython_plugin_handle.xcall_params_ret(%v, py_object(EntryPoint_%v), c_void_p(cdts), out_err, out_err_len)\n", indentStr, isMethodBool, name)
		res += fmt.Sprintf("python_plugin_handle.set_entrypoint('EntryPoint_%v'.encode(), CEntryPoint_%v)\n", name, name)
	} else if len(params) > 0 {
		res += fmt.Sprintf("%v@CFUNCTYPE(None, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong))\n", indentStr)
		res += fmt.Sprintf("%vdef CEntryPoint_%v(cdts, out_err, out_err_len):\n", indentStr, name)
		res += fmt.Sprintf("%v\tglobal python_plugin_handle\n", indentStr)
		res += fmt.Sprintf("%v\tpython_plugin_handle.xcall_params_no_ret(%v, py_object(EntryPoint_%v), c_void_p(cdts), out_err, out_err_len)\n", indentStr, isMethodBool, name)
		res += fmt.Sprintf("python_plugin_handle.set_entrypoint('EntryPoint_%v'.encode(), CEntryPoint_%v)\n", name, name)
	} else if len(retvals) > 0 {
		res += fmt.Sprintf("%v@CFUNCTYPE(None, c_void_p, POINTER(c_char_p), POINTER(c_ulonglong))\n", indentStr)
		res += fmt.Sprintf("%vdef CEntryPoint_%v(cdts, out_err, out_err_len):\n", indentStr, name)
		res += fmt.Sprintf("%v\tglobal python_plugin_handle\n", indentStr)
		res += fmt.Sprintf("%v\tpython_plugin_handle.xcall_no_params_ret(%v, py_object(EntryPoint_%v), c_void_p(cdts), out_err, out_err_len)\n", indentStr, isMethodBool, name)
		res += fmt.Sprintf("python_plugin_handle.set_entrypoint('EntryPoint_%v'.encode(), CEntryPoint_%v)\n", name, name)
	} else {
		res += fmt.Sprintf("%v@CFUNCTYPE(None, POINTER(c_char_p), POINTER(c_ulonglong))\n", indentStr)
		res += fmt.Sprintf("%vdef CEntryPoint_%v(out_err, out_err_len):\n", indentStr, name)
		res += fmt.Sprintf("%v\tglobal python_plugin_handle\n", indentStr)
		res += fmt.Sprintf("%v\tpython_plugin_handle.xcall_no_params_no_ret(%v, py_object(EntryPoint_%v), out_err, out_err_len)\n", indentStr, isMethodBool, name)
		res += fmt.Sprintf("python_plugin_handle.set_entrypoint('EntryPoint_%v'.encode(), CEntryPoint_%v)\n", name, name)
	}

	return res
}

// --------------------------------------------------------------------
func getCFuncType(params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) string {

	if len(params) > 0 && len(retvals) > 0 {
		return "cfunctype_params_ret"
	} else if len(params) > 0 {
		return "cfunctype_params_no_ret"
	} else if len(retvals) > 0 {
		return "cfunctype_no_params_ret"
	} else {
		return "cfunctype_no_params_no_ret"
	}
}

// --------------------------------------------------------------------
func generateCodeGlobals(name string, indent int) string {

	indentStr := ""
	for i := 0; i < indent; i++ {
		indentStr += "\t"
	}

	code := fmt.Sprintf("global xllr_handle\n%vglobal %v_id\n%vglobal runtime_plugin\n%vglobal python_plugin_handle", indentStr, name, indentStr, indentStr)

	return code
}

// --------------------------------------------------------------------
func generateReturn(retvals []*IDL.ArgDefinition) string {
	code := "return "
	retcode := make([]string, 0)
	for i, _ := range retvals {
		retcode = append(retcode, fmt.Sprintf("ret_vals[%v]", i))
	}
	code += strings.Join(retcode, ",")

	return code
}

// --------------------------------------------------------------------
func generateCodeReturnValues(parameters []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition) string {
	/*
		{{if gt $retLength 0}}
		# unpack results
		ret_vals = python_plugin_handle.convert_host_return_values_from_cdts(c_void_p(return_values_buffer), {{len $f.Getter.ReturnValues}})
		return {{range $index, $elem := $f.Getter.ReturnValues}}{{if $index}},{{end}}ret_vals[{{$index}}]{{end}}
		{{end}}
	*/

	if len(retvals) == 0 {
		return ""
	}

	// return values are always at index 1
	return fmt.Sprintf(`ret_vals = python_plugin_handle.convert_host_return_values_from_cdts(c_void_p(xcall_params), 1)`)

}

// --------------------------------------------------------------------
func generateCodeXCall(className string, funcName string, overloadIndexString string, params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition, indent int) string {
	/*
		out_error = ({{ConvertToCPythonType "string8"}} * 1)(0)
		out_error_len = ({{ConvertToCPythonType "size"}})(0)
		xllr_handle.{{XCall $f.Getter.Parameters $f.Getter.ReturnValues}}(c_ulonglong({{$f.Getter.Name}}_id), c_void_p(xcall_params), out_error, byref(out_error_len))

		# check for error
		if out_error != None and out_error[0] != None:
			err_msg = string_at(out_error[0], out_error_len.value).decode('utf-8')
			raise RuntimeError('\n'+err_msg.replace("\\n", "\n"))
	*/
	indentStr := ""
	for i := 0; i < indent; i++ {
		indentStr += "\t"
	}

	var name string
	if className != "" {
		name += className + "_"
	}
	name += funcName

	code := fmt.Sprintf("out_error = (%v * 1)(0)\n", convertToCPythonType("string8"))
	code += fmt.Sprintf("%vout_error_len = (%v)(0)\n", indentStr, convertToCPythonType("size"))
	if len(params) > 0 || len(retvals) > 0 {
		code += fmt.Sprintf("%v%v_id(c_void_p(xcall_params), out_error, byref(out_error_len))", indentStr, name+overloadIndexString)
	} else {
		code += fmt.Sprintf("%v%v_id(out_error, byref(out_error_len))", indentStr, name+overloadIndexString)
	}
	code += fmt.Sprintf("%v\n", indentStr)
	code += fmt.Sprintf("%vif out_error is not None and out_error[0] is not None:\n", indentStr)
	code += fmt.Sprintf("%v\terr_msg = string_at(out_error[0], out_error_len.value).decode('utf-8')\n", indentStr)
	code += fmt.Sprintf("%v\traise RuntimeError('\\n'+err_msg.replace(\"\\\\n\", \"\\n\"))\n", indentStr)

	return code
}

// --------------------------------------------------------------------
func generateCodeAllocateCDTS(params []*IDL.ArgDefinition, retvals []*IDL.ArgDefinition, isObjectMember bool) string {
	/*
		{{$paramsLength := len $f.Getter.Parameters}}
		{{$retLength := len $f.Getter.ReturnValues}}

		{{if gt $paramsLength 0}}
		params = ({{range $index, $elem := $f.Getter.Parameters}}{{if $index}},{{end}} {{$elem.Name}}{{if eq $paramsLength 1}},{{end}}{{end}}) // For object, ignore first parameter and use self.obj_handle
		params_types = ({{range $index, $elem := $f.Getter.Parameters}}{{if $index}},{{end}} {{GetMetaFFIType $elem false}}{{end}}{{if eq $paramsLength 1}},{{end}})
		xcall_params = python_plugin_handle.convert_host_params_to_cdts(py_object(params), py_object(params_types), {{$retLength}})
		{{else if gt $retLength 0}}
		xcall_params = xllr_handle.alloc_cdts_buffer(0, {{len $f.Getter.ReturnValues}})
		{{end}}
	*/

	if len(params) > 0 { // use convert_host_params_to_cdts to allocate CDTS
		code := `xcall_params = python_plugin_handle.convert_host_params_to_cdts(py_object((%v,)), py_object((%v,)), %v)`

		paramsNames := make([]string, 0)
		paramsTypes := make([]string, 0)

		for i, p := range params {

			if isObjectMember && i == 0 {
				paramsNames = append(paramsNames, "self.obj_handle")
			} else {
				name := strings.Replace(p.Name, "*", "", -1) // remove "*" or "**" from the params
				paramsNames = append(paramsNames, fmt.Sprintf("%v", name))
			}

			paramsTypes = append(paramsTypes, getMetaFFIType(p, false))
		}

		return fmt.Sprintf(code, strings.Join(paramsNames, ","), strings.Join(paramsTypes, ","), len(retvals))
	} else if len(retvals) > 0 {
		code := `xcall_params = xllr_handle.alloc_cdts_buffer(0, %v)`
		return fmt.Sprintf(code, len(retvals))
	} else {
		return ""
	}

}

// --------------------------------------------------------------------
func getMetaFFIType(elem *IDL.ArgDefinition, isObjectField bool) string {

	var handle uint64
	var found bool

	if elem.Type == IDL.ANY {
		if isObjectField{
			return fmt.Sprintf("dynamicTypeToMetaFFIType(obj[0].%v)", elem.Name)
		} else {
			return fmt.Sprintf("dynamicTypeToMetaFFIType(%v)", elem.Name)
		}
	}

	if elem.Dimensions == 0 && strings.HasSuffix(string(elem.Type), "_array") {
		panic(fmt.Sprintf("Argument %v type is %v, although its dimensions are larger than 0", elem.Name, elem.Type))
	}

	if elem.Dimensions > 0 && !strings.HasSuffix(string(elem.Type), "_array") {
		elem.Type = IDL.MetaFFIType(string(elem.Type) + "_array")
	}

	handle, found = IDL.TypeStringToTypeEnum[elem.Type]

	if !found {
		panic(fmt.Sprintf("Requested type is not supported: %v. Name: %v", elem.Type, elem.Name))
	}

	return fmt.Sprintf("%v", handle)
}

// --------------------------------------------------------------------
func getMetaFFITypeFromPyType(pytype string) uint64 {

	switch pytype {
	case "str":
		return IDL.TypeStringToTypeEnum[IDL.STRING8]
	case "int":
		return IDL.TypeStringToTypeEnum[IDL.INT64]
	case "float":
		return IDL.TypeStringToTypeEnum[IDL.FLOAT64]
	case "bool":
		return IDL.TypeStringToTypeEnum[IDL.BOOL]
	case "handle":
		return IDL.TypeStringToTypeEnum[IDL.HANDLE]
	default:
		panic("Unexpected PyType: " + pytype)
	}
}

// --------------------------------------------------------------------
func add(x int, y int) int {
	return x + y
}

// --------------------------------------------------------------------
func getEnvVar(env string) string {
	return os.Getenv(env)
}

// --------------------------------------------------------------------
func convertToPythonTypeFromField(definition *IDL.ArgDefinition) string {

	if strings.Contains(string(definition.Type), "_array") {

		if !definition.IsArray() {
			panic(fmt.Sprintf("Argument %v of type %v states an array, but dimensions is %v", definition.Name, definition.Type, definition.Dimensions))
		}

		typeName := strings.ReplaceAll(string(definition.Type), "_array", "")
		return convertToPythonType(IDL.MetaFFIType(typeName), definition.IsArray())
	} else {
		return convertToPythonType(definition.Type, definition.IsArray())
	}
}

// --------------------------------------------------------------------
func convertToPythonType(metaffiType IDL.MetaFFIType, isArray bool) string {

	var res string

	switch metaffiType {
	case IDL.FLOAT64:
		fallthrough
	case IDL.FLOAT32:
		res = "float"

	case IDL.INT8:
		fallthrough
	case IDL.INT16:
		fallthrough
	case IDL.INT32:
		fallthrough
	case IDL.INT64:
		fallthrough
	case IDL.UINT8:
		fallthrough
	case IDL.UINT16:
		fallthrough
	case IDL.UINT32:
		fallthrough
	case IDL.UINT64:
		fallthrough
	case IDL.SIZE:
		res = "int"

	case IDL.BOOL:
		res = "bool"

	case IDL.STRING8:
		fallthrough
	case IDL.STRING16:
		fallthrough
	case IDL.STRING32:
		res = "str"

	case IDL.HANDLE:
		res = "py_object"

	case IDL.ANY:
		res = "Any"

	default:
		panic("Unsupported MetaFFI Type " + metaffiType)
	}

	if isArray {
		res = "List[" + res + "]"
	}

	return res
}

// --------------------------------------------------------------------
func convertToCPythonType(metaffiType IDL.MetaFFIType) string {

	switch metaffiType {
	case IDL.FLOAT64:
		return "c_double"
	case IDL.FLOAT32:
		return "c_float"
	case IDL.INT8:
		return "c_byte"
	case IDL.INT16:
		return "c_short"
	case IDL.INT32:
		return "c_int"
	case IDL.INT64:
		return "c_longlong"
	case IDL.UINT8:
		return "c_ubyte"
	case IDL.UINT16:
		return "c_ushort"
	case IDL.UINT32:
		return "c_uint"
	case IDL.UINT64:
		return "c_ulonglong"
	case IDL.SIZE:
		return "c_ulonglong"
	case IDL.BOOL:
		return "c_ubyte"

	case IDL.STRING8:
		return "c_char_p"
	case IDL.STRING16:
		return "c_wchar_p"
	case IDL.STRING32:
		return "c_wchar_p"

	default:
		panic("Unsupported MetaFFI Type " + metaffiType)
	}
}

//--------------------------------------------------------------------
