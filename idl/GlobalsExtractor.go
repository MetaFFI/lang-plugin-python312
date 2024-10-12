package main

import (
	. "github.com/MetaFFI/lang-plugin-python3/idl/py_extractor"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

// --------------------------------------------------------------------
func ExtractGlobals(pyfile *Py_Info, metaffiGuestLib string) ([]*IDL.GlobalDefinition, error) {

	globalsDefs := make([]*IDL.GlobalDefinition, 0)

	globals, err := pyfile.Get_Globals_MetaFFIGetter()
	if err != nil {
		return nil, err
	}

	for _, gs := range globals {
		name, err := gs.Get_Name_MetaFFIGetter()
		if err != nil {
			return nil, err
		}

		pytype, err := gs.Get_Type_MetaFFIGetter()
		if err != nil {
			return nil, err
		}

		global := IDL.NewGlobalDefinition(name, pyTypeToMFFI(pytype), "Get"+name, "Set"+name)
		global.Getter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
		global.Getter.SetEntityPath("entrypoint_function", "EntryPoint_"+global.Getter.Name)
		global.Setter.SetEntityPath("metaffi_guest_lib", metaffiGuestLib)
		global.Setter.SetEntityPath("entrypoint_function", "EntryPoint_"+global.Setter.Name)

		globalsDefs = append(globalsDefs, global)
	}

	return globalsDefs, nil
}

//--------------------------------------------------------------------
