import gc
import os
import unittest
import collections
import sys
import ctypes

import metaffi
import metaffi.metaffi_runtime
import metaffi.metaffi_module
import metaffi.metaffi_types

runtime: metaffi.metaffi_runtime.MetaFFIRuntime | None = None
test_runtime_module: metaffi.metaffi_module.MetaFFIModule | None = None
test_map_module: metaffi.metaffi_module.MetaFFIModule | None = None


def add_callback(x, y):
	return x + y


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


def init():
	global runtime
	global test_runtime_module
	global test_map_module
	
	runtime = metaffi.metaffi_runtime.MetaFFIRuntime('openjdk')
	runtime.load_runtime_plugin()
	test_runtime_module = runtime.load_module('sanity/TestRuntime.class')
	test_map_module = runtime.load_module('sanity/TestMap.class')


def fini():
	global runtime
	assert runtime is not None
	runtime.release_runtime_plugin()


class TestSanity(unittest.TestCase):
	
	@classmethod
	def setUpClass(cls):
		init()
	
	@classmethod
	def tearDownClass(cls):
		fini()
	
	def test_hello_world(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# load hello world
		hello_world = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=helloWorld', None, None)
		hello_world()
		
		del hello_world
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')  # make sure xcall has been freed
	
	def test_returns_an_error(self):
		global test_runtime_module
		assert test_runtime_module is not None

		returns_an_error = None
		
		try:
			returns_an_error = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=returnsAnError', None, None)
			returns_an_error()
			self.fail('Test should have failed')
		except:
			pass
		
		self.assertIsNotNone(returns_an_error)
		
		del returns_an_error
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_div_integers(self):
		global test_runtime_module
		assert test_runtime_module is not None

		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int32_type),
		               metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int32_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_float32_type)]
		
		div_integers = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=divIntegers', params_type, ret_type)
		
		res = div_integers(1, 2)
		self.assertEqual(0.5, res)
		
		try:
			div_integers(1, 0)
			self.fail('Expected an error - divisor is 0')
		except:
			pass
		
		del div_integers
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_join_strings(self):
		global test_runtime_module
		assert test_runtime_module is not None

		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_array_type, dims=1)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		
		joinStrings = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=joinStrings', params_type, ret_type)
		
		res = joinStrings(['one', 'two', 'three'])
		self.assertEqual('one,two,three', res)
		
		del joinStrings
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_wait_a_bit(self):
		global test_runtime_module
		assert test_runtime_module is not None

		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int32_type)]
		
		getFiveSeconds = test_runtime_module.load_entity('class=sanity.TestRuntime,field=fiveSeconds,getter', None, ret_type)
		
		fiveSeconds = getFiveSeconds()
		self.assertEqual(5, fiveSeconds)
		
		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int32_type)]
		
		waitABit = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=waitABit', params_type, None)
		waitABit(fiveSeconds)
		
		del getFiveSeconds
		del waitABit
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_test_map(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# load functions
		
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)]
		newTestMap = test_runtime_module.load_entity('class=sanity.TestMap,callable=<init>', None, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type)]
		testMapSet = test_runtime_module.load_entity('class=sanity.TestMap,callable=set,instance_required', param_type, None)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type)]
		testMapGet = test_runtime_module.load_entity('class=sanity.TestMap,callable=get,instance_required', param_type, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_bool_type)]
		testMapContains = test_runtime_module.load_entity('class=sanity.TestMap,callable=contains,instance_required', param_type, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		testMapNameSetter = test_runtime_module.load_entity('class=sanity.TestMap,field=name,instance_required,setter', param_type, None)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		testMapNameGetter = test_runtime_module.load_entity('class=sanity.TestMap,field=name,instance_required,getter', param_type, ret_type)
		
		map = newTestMap()
		
		testMapSet(map, 'x', 250)
		
		res = testMapContains(map, 'x')
		self.assertTrue(res)
		
		res = testMapGet(map, 'x')
		self.assertEqual(250, res)
		
		deq = collections.deque()
		deq.append(600)
		testMapSet(map, 'z', deq)
		
		mapped_deq = testMapGet(map, 'z')
		self.assertIsInstance(mapped_deq, collections.deque)
		assert isinstance(mapped_deq, collections.deque)  # for the type checker
		val = mapped_deq.pop()
		self.assertEqual(600, val)
		
		testMapNameSetter(map, 'MyName')
		
		name = testMapNameGetter(map)
		self.assertEqual('MyName', name)

		del map
		del newTestMap
		del testMapSet
		del testMapGet
		del testMapContains
		del testMapNameSetter
		del testMapNameGetter
		
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_callback(self):
		
		self.skipTest('Callback from python is not implemented yet')
		
		def add(x: int, y: int) -> int:
			print('in python add from java - {}+{}'.format(x, y), file=sys.stderr)
			return x + y
		
		# make "add_callback" metaffi callable
		metaffi_callable = metaffi.metaffi_module.make_metaffi_callable(add)
		
		# load call_callback_add from Java
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_callable_type)]
		testMapNameGetter = test_runtime_module.load_entity('class=sanity.TestRuntime,callable=callCallback', param_type, None)
		
		# call call_callback_add passing add_callback
		testMapNameGetter(metaffi_callable)


if __name__ == '__main__':
	unittest.main()
