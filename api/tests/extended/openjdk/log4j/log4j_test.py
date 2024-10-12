import gc
import unittest
import metaffi
import metaffi.metaffi_runtime
import metaffi.metaffi_module
import metaffi.metaffi_types

runtime: metaffi.metaffi_runtime.MetaFFIRuntime | None = None


def init():
	global runtime
	runtime = metaffi.metaffi_runtime.MetaFFIRuntime('openjdk')


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


class Log4jlogger:
	def __init__(self, logger_name: str):
		global runtime
		assert runtime is not None
		log4j_api_module = runtime.load_module('log4j-api-2.21.1.jar;log4j-core-2.21.1.jar')
		
		# load methods
		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		ret_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type, 'org.apache.logging.log4j.Logger')]
		getLogger = log4j_api_module.load_entity('class=org.apache.logging.log4j.LogManager,callable=getLogger', params_type, ret_type)
		
		params_type = [metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_handle_type),
						metaffi.metaffi_types.metaffi_type_info(metaffi.metaffi_types.MetaFFITypes.metaffi_string8_type)]
		self.perror = log4j_api_module.load_entity('class=org.apache.logging.log4j.Logger,callable=error,instance_required', params_type, None)
		self.pwarning = log4j_api_module.load_entity('class=org.apache.logging.log4j.Logger,callable=warn,instance_required', params_type, None)
		
		# call constructor
		self.myLoggerHandle = getLogger(logger_name)
	
	def error(self, msg: str):
		self.perror(self.myLoggerHandle, msg)
		
	def __del__(self):
		del self.myLoggerHandle
		del self.perror
		del self.pwarning
	

class TestLog4j(unittest.TestCase):
	
	@classmethod
	def setUpClass(cls):
		init()
	
	@classmethod
	def tearDownClass(cls):
		fini()
	
	def test_log(self):
		logger = Log4jlogger('Log4j Logger')
		logger.error('Logging error From Python!')
		del logger
		assert_objects_not_loaded_of_type(self, 'MetaFFIEntity')
