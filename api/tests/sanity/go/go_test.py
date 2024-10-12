import gc
import platform
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


def init():
	global runtime
	global test_runtime_module
	global test_map_module

	runtime = metaffi.metaffi_runtime.MetaFFIRuntime('go')
	runtime.load_runtime_plugin()
	if platform.system() == 'Windows':
		test_runtime_module = runtime.load_module('TestRuntime_MetaFFIGuest.dll')
	elif platform.system() == 'Linux':
		test_runtime_module = runtime.load_module('TestRuntime_MetaFFIGuest.so')
	else:
		raise Exception(f'Unsupported system {platform.system()}')


def fini():
	global runtime
	assert runtime is not None

	runtime.release_runtime_plugin()



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
		hello_world = test_runtime_module.load_entity('callable=HelloWorld', None, None)
		hello_world()
		
		del hello_world
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_returns_an_error(self):
		global test_runtime_module
		assert test_runtime_module is not None

		returns_an_error: metaffi.MetaFFIEntity
		
		try:
			returns_an_error = test_runtime_module.load_entity('callable=ReturnsAnError', None, None)
			returns_an_error()
			self.fail('Test should have failed')
		except:
			pass
		
		del returns_an_error
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_div_integers(self):
		global test_runtime_module
		assert test_runtime_module is not None
		
		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type),
		               metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_float32_type)]
		
		div_integers = test_runtime_module.load_entity('callable=DivIntegers', params_type, ret_type)
		
		res = div_integers(1, 2)
		self.assertEqual(res, 0.5, 'Expected 0.5, got: ' + str(res))
		
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
		
		joinStrings = test_runtime_module.load_entity('callable=JoinStrings', params_type, ret_type)
		
		res = joinStrings(['one', 'two', 'three'])
		self.assertEqual(res, 'one,two,three', 'Expected one,two,three, got: ' + str(res))
			
		del joinStrings
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_wait_a_bit(self):
		global test_runtime_module
		assert test_runtime_module is not None

		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)]
		
		getFiveSeconds = test_runtime_module.load_entity('global=FiveSeconds,getter', None, ret_type)
		
		fiveSeconds = getFiveSeconds()
		self.assertEqual(fiveSeconds, 5000000000, 'Expected 5000000000, got: {}'.format(fiveSeconds))
		
		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_int64_type)]
		
		waitABit = test_runtime_module.load_entity('callable=WaitABit', params_type, None)
		
		waitABit(fiveSeconds)
		
		del getFiveSeconds
		del waitABit
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_test_map(self):
		global test_runtime_module
		assert test_runtime_module is not None

		# load functions
		
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)]
		newTestMap = test_runtime_module.load_entity('callable=NewTestMap', None, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type)]
		testMapSet = test_runtime_module.load_entity('callable=TestMap.Set,instance_required', param_type, None)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_any_type)]
		testMapGet = test_runtime_module.load_entity('callable=TestMap.Get,instance_required', param_type, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_bool_type)]
		testMapContains = test_runtime_module.load_entity('callable=TestMap.Contains,instance_required', param_type, ret_type)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
		              metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		testMapNameSetter = test_runtime_module.load_entity('field=TestMap.Name,instance_required,setter', param_type, None)
		
		param_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		testMapNameGetter = test_runtime_module.load_entity('field=TestMap.Name,instance_required,getter', param_type, ret_type)
		
		map = newTestMap()
		
		testMapSet(map, 'x', 250)
		
		res = testMapContains(map, 'x')
		self.assertTrue(res, 'Map should contain x')
		
		res = testMapGet(map, 'x')
		self.assertEqual(res, 250, 'x should be 250')
		
		deq = collections.deque()
		deq.append(600)
		testMapSet(map, 'z', deq)
		
		mapped_deq = testMapGet(map, 'z')
		self.assertIsInstance(mapped_deq, collections.deque, f'z should be a deque')
		assert isinstance(mapped_deq, collections.deque), 'z should be a deque' # for the type checker
		val = mapped_deq.pop()
		self.assertEqual(val, 600, 'mapped_deq should contain 600')
		
		testMapNameSetter(map, 'MyName')
		
		name = testMapNameGetter(map)
		assert isinstance(name, str), 'name should be a string'
		self.assertEqual(name, 'MyName', 'Expected name is MyName. Received: ' + name)
		
		del newTestMap
		del testMapSet
		del testMapGet
		del testMapContains
		del testMapNameSetter
		del testMapNameGetter
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
	
	def test_test_map_with_cpp_object(self):
		pass


if __name__ == '__main__':
	unittest.main()
