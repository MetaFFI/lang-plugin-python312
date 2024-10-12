import builtins
from inspect import getmembers, isfunction, isclass, signature, getcomments, isgetsetdescriptor, ismethoddescriptor, ismodule
import importlib
import os
import sys
from typing import List
import types
import re
import none_python_impl_definitions

ignored_builtins = {'False', 'Ellipsis', 'None', 'True', 'NotImplemented', 'super'}


def _dummy_generic_sig(*args, **named_args):
	pass

class variable_info:
	name: str
	type: str
	is_getter: bool
	is_setter: bool

	def __init__(self):
		self.is_getter = True
		self.is_setter = True

	def __str__(self):
		return '{}:{} (get: {} | set:{})'.format(self.name, self.type, self.is_getter, self.is_setter)

	def __repr__(self):
		return self.__str__()

class parameter_info:
	name: str
	type: str
	is_default_value: bool
	is_optional: bool
	kind: str

	def __init__(self):
		self.is_default_value = False
		self.is_optional = False
		self.kind = ''

	def __str__(self):
		return '{}:{}'.format(self.name, self.type)

	def __repr__(self):
		return self.__str__()


class function_info:
	name: str
	comment: str | None
	parameters: List[parameter_info]
	return_values: List[str]

	def __init__(self):
		self.parameters = list()
		self.return_values = list()

	def __str__(self):
		return self.name

	def __repr__(self):
		return self.__str__()


class class_info:
	name: str
	comment: str | None
	fields: List[variable_info]
	methods: List[function_info]

	def __init__(self):
		self.fields = list()
		self.methods = list()

	def __str__(self):
		return self.name

	def __repr__(self):
		return self.__str__()


class py_info:
	globals: List[variable_info]
	functions: List[function_info]
	classes: List[class_info]

	def __init__(self):
		self.globals = list()
		self.functions = list()
		self.classes = list()


class py_extractor:
	filename: str
	mod: types.ModuleType

	def __init__(self, filename: str):
		filename = filename.replace('\\', '/')  # replace win-style slash to *nix style (which is supported in win)
		self.filename = filename

		if not self.in_site_package(filename):
			sys.path.append(os.path.dirname(filename))
			module_name = os.path.basename(filename)
			module_name = os.path.splitext(module_name)[0]

		else:  # if installed in site-package
			module_name = re.sub('.*site-packages/', '', filename)
			module_name = module_name.replace('.py', '')
			module_name = module_name.replace('/', '.')

		print('Py IDL Extractor: Going to parse ' + module_name)
		self.mod = importlib.import_module(module_name)

	def in_site_package(self, path: str) -> bool:
		return '/site-packages' in path

	def extract(self) -> py_info:
		info = py_info()
		info.globals = self._extract_globals()
		info.functions = self._extract_functions()
		info.classes = self._extract_classes()

		return info

	def _is_variable(self, obj):
		is_not_var = isfunction(obj) or isclass(obj) or ismodule(obj)
		return not is_not_var

	def _is_callable(self, obj):
		return hasattr(obj, '__call__') and not isclass(obj)

	def _extract_globals(self) -> List[variable_info]:
		all_members = getmembers(self.mod)
		global_vars = []
		for m in all_members:
			if self.mod.__name__ == 'builtins' and m[0] in ignored_builtins:  # if builtin module, skip special cases
				continue

			if not self._is_variable(m[1]):
				continue

			if m[0].startswith('_'):
				continue

			global_vars.append(self._extract_field(m, True, True))

		return global_vars

	def _extract_functions(self) -> List[function_info]:
		functions_members = getmembers(self.mod, self._is_callable)
		res = []

		for f in functions_members:
			if f[0].startswith('_'):
				continue

			res.append(self._extract_function(f, None))

		return res

	def _extract_classes(self) -> List[class_info]:
		global ignored_builtins

		res = []

		for c in getmembers(self.mod, isclass):
			clsdata = class_info()
			clsdata.name = c[0]
			clsdata.comment = getcomments(c[1])
			if clsdata.comment is not None:
				clsdata.comment = clsdata.comment.replace('#', '', 1).strip()

			constructor_found = False
			found_in_annotations = []
			for member in getmembers(c[1]):
				if member[0] in found_in_annotations:  # already found in annotations
					continue

				if member[0] == '__annotations__' and not isgetsetdescriptor(member[1]):  # fields
					for k, v in member[1].items():
						if k.startswith('_'):
							continue

						clsdata.fields.append(self._extract_field([k, v], True, True))
						found_in_annotations.append(k)

				if ismethoddescriptor(member[1]):

					# check if method is in "non_python_method_definitions"
					method_data = none_python_impl_definitions.get_method_definition(self.mod.__name__, clsdata.name, member[0])
					if method_data is None:
						# print(f'Skipping {self.mod.__name__}.{clsdata.name}.{member[0]} as it is not implemented in python, and definition not found in non_python_method_definitions')
						continue

					if member[0] == '__init__':
						constructor_found = True

					clsdata.methods.append(self._extract_function((member[0], method_data), clsdata.name))

				elif isfunction(member[1]):
					# if member[0].startswith('_') and member[0] != '__init__':
					# 	continue

					if member[0] == '__init__':
						constructor_found = True

					clsdata.methods.append(self._extract_function(member, clsdata.name))
				elif isinstance(member[1], builtins.property):
					if member[1].fget is not None or member[1].fset is not None:
						clsdata.fields.append(self._extract_field(member, member[1].fget is not None, member[1].fset is not None))
				else:
					if member[0].startswith('_'):
						continue
					else:
						# print('Skipping {} of type {}'.format('{}.{}'.format(clsdata.name, member[0]), type(member[1]).__name__))
						continue

			# make sure class has a constructor, if not, add the default one
			if not constructor_found:
				clsdata.methods.append(self._default_constructor(clsdata.name))

			res.append(clsdata)

		return res

	def _extract_field(self, m, is_getter, is_setter) -> variable_info:
		v = variable_info()
		v.name = m[0]

		if isinstance(m[1], builtins.property):
			if m[1].fget is not None:
				sig = signature(m[1].fget)
				if sig.return_annotation is None:
					v.type = 'any'
				elif isinstance(sig.return_annotation, str):
					if '|' in sig.return_annotation:
						v.type = 'any'
					else:
						v.type = sig.return_annotation
				elif sig.return_annotation == sig.empty:
					v.type = 'any'
				else:
					v.type = sig.return_annotation.__name__.strip()
			elif m[1].fset is not None:
				sig = signature(m[1].fset)
				if sig.return_annotation is None:
					v.type = 'any'
				elif isinstance(sig.return_annotation, str):
					if '|' in sig.return_annotation:
						v.type = 'any'
					else:
						v.type = sig.return_annotation
				elif sig.return_annotation == sig.empty:
					v.type = 'any'
				else:
					v.type = sig.return_annotation.__name__.strip()
			else:
				raise ValueError('property {} does not have a getter nor a setter'.format(m[0]))
		else:
			if hasattr(m[1], '__name__'):
				v.type = m[1].__name__.strip()
			elif hasattr(m[1], '__class__'):
				v.type = m[1].__class__.__name__.strip()
			else:
				raise ValueError('Failed to find the type of the field "{}". {}'.format(m[0], m[1]))

		if v.type == '_empty':
			v.type = 'any'

		v.is_getter = is_getter
		v.is_setter = is_setter

		return v

	def _default_constructor(self, clsname: str) -> function_info:
		if clsname is None:
			raise ValueError('No class name for default constructor')

		func_info = function_info()
		func_info.name = '__init__'
		func_info.comment = 'Default Constructor'
		func_info.return_values.append(clsname)

		return func_info

	def _extract_function(self, f, clsname: str | None) -> function_info:
		func_info = function_info()
		func_info.name = f[0]

		func_info.comment = getcomments(f[1])
		if func_info.comment is not None:
			func_info.comment = func_info.comment.replace('#', '', 1).strip()

		try:
			sig = signature(f[1])
		except ValueError as e:
			# print('Failed extracting signature of {}. Retry without following wrappers. Error:\n{}'.format(f[0], e))
			try:
				sig = signature(f[1], follow_wrapped=False)
			except ValueError as e:
				# print('Failed extracting signature of {}. Skip. Error:\n{}'.format(f[0], e))
				sig = signature(_dummy_generic_sig)

		# parse parameters
		for name, param in sig.parameters.items():

			pdata = parameter_info()
			pdata.name = name

			if param.annotation != param.empty:
				if isinstance(param.annotation, str):
					if '|' in param.annotation:
						pdata.type = 'any'
					else:
						pdata.type = param.annotation
				elif isinstance(param.annotation, types.UnionType):
					pdata.type = 'any'
				elif hasattr(param.annotation, '__name__'):
					pdata.type = param.annotation.__name__
				elif hasattr(param.annotation, '__class__'):
					pdata.type = param.annotation.__class__.__name__
				else:
					raise ValueError('Failed to find the type of the parameter "{}".'.format(param.annotation))
			else:
				pdata.type = 'any'

			if pdata.type == 'typing.Any' or pdata.type == 'Any':
				pdata.type = 'any'

			pdata.is_default_value = param.default != param.empty
			pdata.is_optional = 'VAR_' in param.kind.name
			pdata.kind = param.kind.name

			# cleanup the name
			pdata.name = pdata.name.replace('*', '')

			func_info.parameters.append(pdata)

		# parse return value
		if func_info.name == '__init__':
			if clsname is None:
				raise ValueError('No class name for constructor')

			func_info.return_values.append(clsname)
		else:
			if sig.return_annotation is not None:
				if isinstance(sig.return_annotation, str):
					if '|' in sig.return_annotation:
						rettype = 'any'
					else:
						rettype = sig.return_annotation
				elif sig.empty == sig.return_annotation:
					rettype = 'any'
				else:
					rettype = sig.return_annotation.__name__.strip()
					if rettype == '_empty':
						raise RuntimeError('Shouldnt reach here! what sig.return_annotation type: {}'.format(type(sig.return_annotation)))

				if rettype == 'typing.Any' or rettype == 'Any':
					rettype = 'any'

				func_info.return_values.append(rettype)

		return func_info


if '__main__' == __name__:
	pass


