package main

import (
	"fmt"

	. "github.com/MetaFFI/lang-plugin-python3/idl/py_extractor"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

var classes map[string]*IDL.ClassDefinition

// --------------------------------------------------------------------
func ExtractClasses(pyinfo *Py_Info, metaffiGuestLib string) ([]*IDL.ClassDefinition, error) {

	classes = make(map[string]*IDL.ClassDefinition)

	pyclses, err := pyinfo.Get_Classes_MetaFFIGetter()
	if err != nil {
		return nil, err
	}

	for _, c := range pyclses {
		clsName, err := c.Get_Name_MetaFFIGetter()
		if err != nil {
			return nil, err
		}

		pycls := IDL.NewClassDefinition(clsName)

		comment, err := c.Get_Comment_MetaFFIGetter()
		if err != nil {
			return nil, err
		}
		pycls.Comment = comment

		fields, err := c.Get_Fields_MetaFFIGetter()
		if err != nil {
			return nil, err
		}

		for _, f := range fields {
			name, err := f.Get_Name_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			typy, err := f.Get_Type_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			isGetter, err := f.Get_Is_Getter_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			isSetter, err := f.Get_Is_Setter_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			if isGetter && isSetter {
				fdecl := IDL.NewFieldDefinition(pycls, name, pyTypeToMFFI(typy), "Get"+name, "Set"+name, true)

				fdecl.Getter.SetTag("receiver_pointer", "true")
				fdecl.Getter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				fdecl.Getter.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+fdecl.Getter.Name)

				fdecl.Setter.SetTag("receiver_pointer", "true")
				fdecl.Setter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				fdecl.Setter.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+fdecl.Setter.Name)

				pycls.AddField(fdecl)

			} else if isGetter {
				fdecl := IDL.NewFieldDefinition(pycls, name, pyTypeToMFFI(typy), "Get"+name, "", true)

				fdecl.Getter.SetTag("receiver_pointer", "true")
				fdecl.Getter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				fdecl.Getter.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+fdecl.Getter.Name)

				pycls.AddField(fdecl)

			} else if isSetter {
				fdecl := IDL.NewFieldDefinition(pycls, name, pyTypeToMFFI(typy), "", "Set"+name, true)

				fdecl.Setter.SetTag("receiver_pointer", "true")
				fdecl.Setter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				fdecl.Setter.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+fdecl.Setter.Name)

				pycls.AddField(fdecl)
			} else {
				return nil, fmt.Errorf("Field/property %v.%v has neither getter or setter", clsName, name)
			}
		}

		pymethods, err := c.Get_Methods_MetaFFIGetter()
		if err != nil {
			return nil, err
		}

		for _, f := range pymethods {

			name, err := f.Get_Name_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			comment, err := f.Get_Comment_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			params, err := f.Get_Parameters_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			if len(params) > 1 {
				params = params[1:]
			} else {
				params = []Parameter_Info{}
			}

			retvals, err := f.Get_Return_Values_MetaFFIGetter()
			if err != nil {
				return nil, err
			}

			pymeth, err := GenerateFunctionDefinition(name, comment, params, retvals, metaffiGuestLib)
			if err != nil {
				return nil, err
			}

			if name == "__init__" {
				pymeth.Name = clsName
				cstr := IDL.NewConstructorDefinitionFromFunctionDefinition(pymeth)
				cstr.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+cstr.Name)
				cstr.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				pycls.AddConstructor(cstr)
			} else if name == "__del__" {
				pycls.Releaser = IDL.NewReleaserDefinitionFromFunctionDefinition(pycls, pymeth)
			} else {
				meth := IDL.NewMethodDefinitionWithFunction(pycls, pymeth, true)
				meth.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
				meth.SetEntityPath("entrypoint_function", "EntryPoint_"+pycls.Name+"_"+meth.Name)
				pycls.AddMethod(meth)
			}
		}

		classes[pycls.Name] = pycls
	}

	res := make([]*IDL.ClassDefinition, 0)
	for _, c := range classes {

		c.Releaser.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
		c.Releaser.SetEntityPath("entrypoint_function", "EntryPoint_"+c.Name+"_"+c.Releaser.Name)

		res = append(res, c)
	}

	return res, nil
}

//--------------------------------------------------------------------
