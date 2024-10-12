package main

import "github.com/MetaFFI/plugin-sdk/compiler/go/IDL"

var MetaFFITypeToPython3Type = map[IDL.MetaFFIType]string{

	IDL.FLOAT64: "float",
	IDL.FLOAT32: "float",

	IDL.INT8: "int",
	IDL.INT16: "int",
	IDL.INT32: "int",
	IDL.INT64: "int",

	IDL.UINT8: "int",
	IDL.UINT16: "int",
	IDL.UINT32: "int",
	IDL.UINT64: "int",

	IDL.BOOL: "bool",

	IDL.STRING8: "str",
	IDL.STRING16: "str",
	IDL.STRING32: "str",

	IDL.SIZE: "int",
}