package main

import (
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)
//--------------------------------------------------------------------
func pyTypeToMFFI(typename string) IDL.MetaFFIType{

	switch typename {
	case "str": return IDL.STRING8
	case "int": return IDL.INT64
	case "float": return IDL.FLOAT64
	case "bool": return IDL.BOOL
	case "Any": return IDL.ANY
	case "any": return IDL.ANY
	case "list": return IDL.ANY

	default:
		return IDL.HANDLE
	}

}
//--------------------------------------------------------------------
