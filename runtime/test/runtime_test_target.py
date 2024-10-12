import sys
import time
import typing


# basic void(void)
def hello_world() -> None:
	print('Hello World, from Python3')


# returns error
def returns_an_error() -> None:
	print('going to throw exception')
	raise Exception('Error')


# simple integer division. dividing by zero must fail
def div_integers(x: int, y: int) -> float:
	print('dividing {}/{}'.format(x, y))
	return x / y


# testing array of strings
def join_strings(arr) -> str:
	res = ','.join(arr)
	return res


# access global variable
five_seconds = 5


# pass to wait_a_bit "five_seconds" global variable
def wait_a_bit(secs: int):
	print('(supposedly) waiting for {} seconds'.format(secs))
	return None


def return_null():
	return None


# testing a container
class testmap:
	def __init__(self):
		self.curdict = dict()
		self.name = 'name1'
	
	def set(self, k: str, v):
		self.curdict[k] = v
	
	def get(self, k: str):
		v = self.curdict[k]
		return v
	
	def contains(self, k: str):
		return k in self.curdict


# simple class to be used for "objects" testing
class SomeClass:
	def print(self):
		print("Hello from inner class")


# test getting an array of objects
def get_some_classes():
	return [SomeClass() for _ in range(3)]


# test expects getting a list of 3 SomeClass instances
def expect_three_some_classes(arr):
	if len(arr) != 3:
		raise ValueError("Array length is not 3")
	
	# verify each element is SomeClass
	for el in arr:
		if not isinstance(el, SomeClass):
			raise ValueError(f"Element is not of type SomeClass, but of type {type(el)}")


# expects a list of 3 buffers
def expect_three_buffers(buffers):
	if len(buffers) != 3:
		raise ValueError("Buffers length is not 3")
	
	# make sure each element is a buffer
	for buf in buffers:
		if not isinstance(buf, bytes):
			raise ValueError(f"Element is not of type bytes, but of type {type(buf)}")
	
	# make sure 1st buffer is [1, 2]
	if buffers[0] != bytes([1, 2]):
		raise ValueError("1st buffer is not [1, 2]")
	
	# make sure 2nd buffer is [3, 4, 5]
	if buffers[1] != bytes([3, 4, 5]):
		raise ValueError("2nd buffer is not [3, 4, 5]")
	
	# make sure 3rd buffer is [6, 7, 8, 9]
	if buffers[2] != bytes([6, 7, 8, 9]):
		raise ValueError("3rd buffer is not [6, 7, 8, 9]")


# returns a list of 3 buffers
def get_three_buffers():
	buffers = [bytes([1, 2, 3, 4]), bytes([5, 6, 7]), bytes([8, 9])]
	return buffers


def returns_array_with_different_dimensions():
	# 1st element is 1dim array
	# 2nd element is int
	# 3rd element is 2dim array
	return [[1, 2, 3], 4, [[5, 6], [7, 8]]]


def returns_array_of_different_objects():
	return [1, 'string', 3.0, None, bytes([1, 2, 3]), SomeClass()]


def return_any(which_type: int) -> typing.Any:
	if which_type == 0:
		return 1
	elif which_type == 1:
		return 'string'
	elif which_type == 2:
		return 3.0
	elif which_type == 3:
		return ['list', 'of', 'strings']
	elif which_type == 4:
		return SomeClass()
	else:
		return None


def accepts_any(which_type_to_expect: int, val: typing.Any):
	if which_type_to_expect == 0:
		if not isinstance(val, int) and val == 1:
			raise ValueError(f"Expected int, got {type(val)} which is {val}")
	elif which_type_to_expect == 1:
		if not isinstance(val, str) and val == 'string':
			raise ValueError(f"Expected str, got {type(val)} which is {val}")
	elif which_type_to_expect == 2 and val == 3.0:
		if not isinstance(val, float):
			raise ValueError(f"Expected float, got {type(val)} which is {val}")
	elif which_type_to_expect == 3:
		if val is not None:
			raise ValueError(f"Expected None, got {type(val)} which is {val}")
	elif which_type_to_expect == 4:
		if not isinstance(val, bytes) and val == bytes([1, 2, 3]):
			raise ValueError(f"Expected list, got {type(val)} which is {val}")
	elif which_type_to_expect == 5:
		if not isinstance(val, SomeClass):
			raise ValueError(f"Expected SomeClass, got {type(val)} which is {val}")
	else:
		raise ValueError(f"Unsupported expected type: {which_type_to_expect}")


def call_callback_add(add_function):
	res = add_function(1, 2)
	print(f'returned from callback')
	res = res[0]
	if res != 3:
		raise Exception('expected 3, got: {}'.format(res))


def return_multiple_return_values():
	return 1, 'string', 3.0, None, bytes([1, 2, 3]), SomeClass()


class extended_test:
	def __init__(self):
		self._x = 0
	
	@property
	def x(self) -> int:
		return self._x
	
	@x.setter
	def x(self, val1: int):
		self._x = val1
	
	def positional_or_named(self, value: str) -> str:
		print(value)
		return value
	
	def list_args(self, value='default', *args) -> list:
		print(value)
		res = [value]
		for a in args:
			print(a)
			res.append(a)
		return res
	
	def dict_args(self, value='default', **named_args) -> list[str]:
		res = [value]
		print(value)
		for k, v in named_args.items():
			res.append(k)
			res.append(v)
			print('{}={}'.format(k, v))
		return res
	
	def named_only(self, *, named: str) -> str:
		print(named)
		return named
	
	def positional_only(self, v1: str, v2: str = 'default', /) -> str:
		print(v1 + ' ' + v2)
		return v1 + ' ' + v2
	
	def arg_positional_arg_named(self, value: str = 'default', *args, **named_args) -> list[str]:
		res = [value]
		print(value)
		
		for a in args:
			print(a)
			res.append(a)
		
		for k, v in named_args.items():
			print('{}={}'.format(k, v))
			res.append(k)
			res.append(v)
		return res

####################################################################################################
# Also call from runtime test to libraries:
# - deque
# - beautifulsoup4
# - numpy
# - pandas
