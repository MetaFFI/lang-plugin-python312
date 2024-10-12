package main

import "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"

func TestGetIDL() {
	idl := IDL.NewIDLDefinition("py_extractor.json", "python3")
	module := IDL.NewModuleDefinition("py_extractor")
	idl.AddModule(module)

	variable_info := IDL.NewClassDefinition("variable_info")
	variable_info.AddField(IDL.NewFieldDefinition(variable_info, "name", IDL.STRING8, "get_name", "", true))
	variable_info.AddField(IDL.NewFieldDefinition(variable_info, "type", IDL.STRING8, "get_type", "", true))
	module.AddClass(variable_info)

	parameter_info := IDL.NewClassDefinition("parameter_info")
	parameter_info.AddField(IDL.NewFieldDefinition(parameter_info, "name", IDL.STRING8, "get_name", "", true))
	parameter_info.AddField(IDL.NewFieldDefinition(parameter_info, "type", IDL.STRING8, "get_type", "", true))
	module.AddClass(parameter_info)

	function_info := IDL.NewClassDefinition("function_info")
	function_info.AddField(IDL.NewFieldDefinition(function_info, "name", IDL.STRING8, "get_name", "", true))
	function_info.AddField(IDL.NewFieldDefinition(function_info, "comment", IDL.STRING8, "get_comment", "", true))
	function_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(function_info, "parameters", IDL.HANDLE_ARRAY, 1, "variable_info", "get_parameters", "", true))
	function_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(function_info, "return_values", IDL.STRING8_ARRAY, 1, "", "get_return_values", "", true))
	module.AddClass(function_info)

	class_info := IDL.NewClassDefinition("class_info")
	class_info.AddField(IDL.NewFieldDefinition(class_info, "name", IDL.STRING8, "get_name", "", true))
	class_info.AddField(IDL.NewFieldDefinition(class_info, "comment", IDL.STRING8, "get_comment", "", true))
	class_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(class_info, "fields", IDL.HANDLE_ARRAY, 1, "variable_info", "get_fields", "", true))
	class_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(class_info, "methods", IDL.HANDLE_ARRAY, 1, "function_info", "get_methods", "", true))
	module.AddClass(class_info)

	py_info := IDL.NewClassDefinition("py_info")
	py_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(py_info, "globals", IDL.HANDLE_ARRAY, 1, "variable_info", "get_globals", "", true))
	py_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(py_info, "functions", IDL.HANDLE_ARRAY, 1, "function_info", "get_functions", "", true))
	py_info.AddField(IDL.NewFieldArrayDefinitionWithAlias(py_info, "classes", IDL.HANDLE_ARRAY, 1, "class_info", "get_classes", "", true))
	module.AddClass(py_info)

	py_extractor := IDL.NewClassDefinition("py_extractor")

	cstr := IDL.NewConstructorDefinition("py_extractor")
	cstr.AddParameter(IDL.NewArgDefinition("filename", IDL.STRING8))
	py_extractor.AddConstructor(cstr)

	extract := IDL.NewMethodDefinition(py_extractor, "extract", true)
	extract.AddReturnValues(IDL.NewArgDefinitionWithAlias("info", IDL.HANDLE, "py_info"))
	py_extractor.AddMethod(extract)

	module.AddClass(py_extractor)

	module.SetEntityPath("module", "py_extractor")

	idl.FinalizeConstruction()
	println(idl.ToJSON())
}
