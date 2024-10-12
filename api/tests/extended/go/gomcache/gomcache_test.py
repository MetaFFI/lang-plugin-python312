import gc
import os
import platform
import sys
import time
import typing
import unittest
import metaffi
import metaffi.metaffi_runtime
import metaffi.metaffi_module
import metaffi.metaffi_types

runtime: metaffi.metaffi_runtime.MetaFFIRuntime | None = None


def init():
	global runtime
	runtime = metaffi.metaffi_runtime.MetaFFIRuntime('go')


def fini():
	global runtime
	assert runtime is not None
	runtime.release_runtime_plugin()
	del runtime


def assert_objects_not_loaded_of_type(tc: unittest.TestCase, type_name: str):
	gc.collect()  # Force a garbage collection to update the object list
	all_objects = gc.get_objects()  # Get a list of all objects tracked by the GC
	
	# Convert type_name to lowercase for case-insensitive comparison
	type_name_lower = type_name.lower()
	
	# Find objects whose type name contains the type_name substring, case-insensitively
	specific_type_objects = [obj for obj in all_objects if type_name_lower in type(obj).__name__.lower()]
	
	if len(specific_type_objects) > 0:
		print(f"Found {len(specific_type_objects)} objects of type(s) containing '{type_name}'")
		for obj in specific_type_objects:
			print(f"Object: {obj}, Type: {type(obj).__name__}")
		tc.fail(f"Found {len(specific_type_objects)} objects of type(s) containing '{type_name}'")


class GoMCache:
	def __init__(self):
		global runtime
		assert runtime is not None
		
		if platform.system() == 'Windows':
			module = runtime.load_module('mcache_MetaFFIGuest.dll')
		elif platform.system() == 'Linux':
			module = runtime.load_module('mcache_MetaFFIGuest.so')
		else:
			raise Exception(f'Unsupported platform {platform.system()}')
		
		# get INFINITY to use in "set method"
		infinity_getter = module.load_entity('global=TTL_FOREVER,getter', None, (metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type),))
		self.infinity = infinity_getter()
		del infinity_getter
		
		# load constructor
		new_gomcache = module.load_entity('callable=New', None, (metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),))
		self.instance = new_gomcache()
		del new_gomcache
		
		# load methods
		self.plen = module.load_entity('callable=CacheDriver.Len,instance_required',
			(metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),),
			(metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_array_type),))
		
		self.pset = module.load_entity('callable=CacheDriver.Set,instance_required',
			[metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type),
			 metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type),
			 metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)],
			[metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)])
		
		self.pget = module.load_entity('callable=CacheDriver.Get,instance_required',
			[metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
			 metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)],
			[metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type),
			 metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_bool_type)])
	
	def __len__(self) -> int:
		l = self.plen(self.instance)
		assert isinstance(l, int)
		return l
	
	def set(self, key: str, val):
		err = self.pset(self.instance, key, val, self.infinity)
		if err is not None:
			raise Exception(f'Failed with error: {err}')
	
	def get(self, key: str) -> typing.Tuple[typing.Any, bool]:
		return self.pget(self.instance, key)
	
	def __del__(self):
		del self.instance
		del self.plen
		del self.pset
		del self.pget


class GoMCacheTest(unittest.TestCase):
	
	@classmethod
	def setUpClass(cls):
		init()
	
	@classmethod
	def tearDownClass(cls):
		fini()
	
	def test_mcache(self):
		global runtime
		
		# get field
		m = GoMCache()
		m.set('myinteger', 101)
		
		if len(m) != 1:
			self.fail(f'len should be 1, but it returned {len(m)}')
		
		val, is_found = m.get('myinteger')
		
		if not is_found:
			self.fail('did not find the key myinteger after settings it')
		
		if val != 101:
			self.fail("val expected to be 101, while it is " + str(val))
			
		del m
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')

