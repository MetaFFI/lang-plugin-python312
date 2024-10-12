package main

import (
	"fmt"
	"path"
	"path/filepath"
	"strings"

	. "github.com/MetaFFI/lang-plugin-python3/idl/py_extractor"
	"github.com/MetaFFI/plugin-sdk/compiler/go/IDL"
)

// --------------------------------------------------------------------
type PyIDLCompiler struct {
	sourceCode         string
	sourceCodeFilePath string
	pyfile             *Py_Extractor

	idl *IDL.IDLDefinition
}

// --------------------------------------------------------------------
func NewPyIDLCompiler() *PyIDLCompiler {
	MetaFFILoad("py_extractor_MetaFFIGuest")
	return &PyIDLCompiler{}
}

// --------------------------------------------------------------------
func (this *PyIDLCompiler) getModulesPathInSitePackages(pathToPyFile string) []string {

	if !strings.Contains(strings.ToLower(pathToPyFile), "site-packages") {
		return nil
	}

	res := make([]string, 0)

	//file := filepath.Base(pathToPyFile)
	idlpath := filepath.Dir(pathToPyFile)
	//fileNoExt := file[:len(file)-len(filepath.Ext(file))]
	//res = append(res, fileNoExt)

	for true {
		fmt.Printf("%v\n", idlpath)
		curPath := filepath.Base(idlpath)

		if strings.ToLower(curPath) == "site-packages" { // got to site-packages
			break
		}

		tmp := make([]string, 1, 1)
		tmp[0] = curPath
		res = append(tmp, res...) // prepend
		idlpath = filepath.Dir(idlpath)
	}

	return res

}

// --------------------------------------------------------------------
func (this *PyIDLCompiler) ParseIDL(sourceCode string, filePath string) (*IDL.IDLDefinition, bool, error) {

	MetaFFILoad("py_extractor")
	pyfile, err := NewPy_Extractor(filePath)
	if err != nil {
		return nil, true, err
	}

	this.sourceCode = sourceCode
	this.sourceCodeFilePath = strings.ReplaceAll(filePath, "\\", "/")
	this.pyfile = pyfile

	this.idl = IDL.NewIDLDefinition(this.sourceCodeFilePath, "python3")

	pyinfo, err := this.pyfile.Extract()
	if err != nil {
		return nil, true, err
	}

	globals, err := ExtractGlobals(&pyinfo, this.idl.MetaFFIGuestLib)
	if err != nil {
		return nil, true, err
	}

	classes, err := ExtractClasses(&pyinfo, this.idl.MetaFFIGuestLib)
	if err != nil {
		return nil, true, err
	}

	functions, err := ExtractFunctions(&pyinfo, this.idl.MetaFFIGuestLib)
	if err != nil {
		return nil, true, err
	}

	// to_py_tuple AST and to_cdts IDLDefinition

	module := IDL.NewModuleDefinition(this.idl.IDLSource)

	module.AddGlobals(globals)
	module.AddFunctions(functions)

	for _, c := range classes {
		module.AddClass(c)
	}

	this.idl.AddModule(module)

	guestCodeModule := strings.ReplaceAll(path.Base(this.sourceCodeFilePath), path.Ext(this.sourceCodeFilePath), "")

	// set package function path if from "site-packages"
	if strings.Contains(this.idl.IDLFullPath, "site-packages") {

		pathToModule := this.getModulesPathInSitePackages(this.idl.IDLFullPath)
		pathToModule = append(pathToModule, guestCodeModule)

		fullPathToModule := strings.Join(pathToModule, ".")

		module.AddExternalResource(fullPathToModule)
		module.SetEntityPath("module", strings.Replace(fullPathToModule, "\\", "/", -1))
	} else {
		module.AddExternalResource(guestCodeModule)
		module.SetEntityPath("module", strings.Replace(guestCodeModule, "\\", "/", -1))
	}

	this.idl.FinalizeConstruction()

	return this.idl, false, nil
}

//--------------------------------------------------------------------
