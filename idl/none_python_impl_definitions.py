import collections
from typing import Any, Callable, DefaultDict, Deque, Dict, Iterable, List, Optional, Tuple, Set
import inspect
import sys

# This file contains the definitions of the built-in classes and functions in Python 3
# The definitions are used to generate the Python 3 implementation of the IDL

# disable all reportReturnType errors in this file
# pyright: reportReturnType=false

class __builtins__tuple:
	def __repr__(self) -> str:
		pass

	def __getitem__(self, index: int) -> Any:
		pass

	def __len__(self) -> int:
		pass

	def count(self, value: Any) -> int:
		pass

	def index(self, value: Any, start: int = 0, end: int | None = None) -> int:
		pass


class __builtins__set:
	def __repr__(self) -> str:
		pass

	def __contains__(self, value: Any) -> bool:
		pass

	def __len__(self) -> int:
		pass

	def add(self, value: Any) -> None:
		pass

	def clear(self) -> None:
		pass

	def copy(self) -> Set[Any]:
		pass

	def difference(self, *others: Any) -> Set[Any]:
		pass

	def difference_update(self, *others: Any) -> None:
		pass

	def discard(self, value: Any) -> None:
		pass

	def intersection(self, *others: Any) -> Set[Any]:
		pass

	def intersection_update(self, *others: Any) -> None:
		pass

	def isdisjoint(self, other: Any) -> bool:
		pass

	def issubset(self, other: Any) -> bool:
		pass

	def issuperset(self, other: Any) -> bool:
		pass

	def pop(self) -> Any:
		pass

	def remove(self, value: Any) -> None:
		pass

	def removesuffix(self, suffix: str) -> str:
		pass

	def removeprefix(self, prefix: str) -> str:
		pass

	def symmetric_difference(self, other: Any) -> Set[Any]:
		pass

	def symmetric_difference_update(self, other: Any) -> None:
		pass

	def union(self, *others: Any) -> Set[Any]:
		pass

	def update(self, *others: Any) -> None:
		pass


class __builtins__str:

	def __repr__(self) -> str:
		pass

	def __add__(self, other: str) -> str:
		pass

	def __contains__(self, sub: str) -> bool:
		pass

	def __eq__(self, other: str) -> bool:
		pass

	def __getitem__(self, index: int) -> str:
		pass

	def __len__(self) -> int:
		pass

	def __mul__(self, n: int) -> str:
		pass

	def capitalize(self) -> str:
		pass

	def casefold(self) -> str:
		pass

	def center(self, width: int, fillchar: str = ' ') -> str:
		pass

	def count(self, sub: str, start: int = 0, end: int | None = None) -> int:
		pass

	def encode(self, encoding: str = 'utf-8', errors: str = 'strict') -> bytes:
		pass

	def endswith(self, suffix: str, start: int = 0, end: int | None = None) -> bool:
		pass

	def expandtabs(self, tabsize: int = 8) -> str:
		pass

	def find(self, sub: str, start: int = 0, end: int | None = None) -> int:
		pass

	def format(self, *args: Any, **kwargs: Any) -> str:
		pass

	def format_map(self, mapping: Any) -> str:
		pass

	def index(self, sub: str, start: int = 0, end: int | None = None) -> int:
		pass

	def isalnum(self) -> bool:
		pass

	def isalpha(self) -> bool:
		pass

	def isascii(self) -> bool:
		pass

	def isdecimal(self) -> bool:
		pass

	def isdigit(self) -> bool:
		pass

	def isidentifier(self) -> bool:
		pass

	def islower(self) -> bool:
		pass

	def isnumeric(self) -> bool:
		pass

	def isprintable(self) -> bool:
		pass

	def isspace(self) -> bool:
		pass

	def istitle(self) -> bool:
		pass

	def isupper(self) -> bool:
		pass

	def join(self, iterable: Any) -> str:
		pass

	def ljust(self, width: int, fillchar: str = ' ') -> str:
		pass

	def lower(self) -> str:
		pass

	def lstrip(self, chars: int | None = None) -> str:
		pass

	def partition(self, sep: str) -> tuple:
		pass

	def replace(self, old: str, new: str, count: int = -1) -> str:
		pass

	def rfind(self, sub: str, start: int = 0, end: int | None = None) -> int:
		pass

	def rindex(self, sub: str, start: int = 0, end: int | None = None) -> int:
		pass

	def rjust(self, width: int, fillchar: str = ' ') -> str:
		pass

	def rpartition(self, sep: str) -> tuple:
		pass

	def rsplit(self, sep: int | None = None, maxsplit: int = -1) -> list:
		pass

	def rstrip(self, chars: int | None = None) -> str:
		pass

	def split(self, sep: int | None = None, maxsplit: int = -1) -> list:
		pass

	def splitlines(self, keepends: bool = False) -> list:
		pass

	def startswith(self, prefix: str, start: int = 0, end: int | None = None) -> bool:
		pass

	def strip(self, chars: str | None = None) -> str:
		pass

	def swapcase(self) -> str:
		pass

	def title(self) -> str:
		pass

	def translate(self, table: Any) -> str:
		pass

	def upper(self) -> str:
		pass

	def zfill(self, width: int) -> str:
		pass


class __builtins__list:
	def __getitem__(self, index: int) -> Any:
		pass

	def __setitem__(self, index: int, value: Any) -> None:
		pass

	def __delitem__(self, index: int) -> None:
		pass

	def __len__(self) -> int:
		pass

	def __repr__(self) -> str:
		pass

	def append(self, value: Any) -> None:
		pass

	def extend(self, iterable: Any) -> None:
		pass

	def insert(self, index: int, value: Any) -> None:
		pass

	def remove(self, value: Any) -> None:
		pass

	def pop(self, index: int = -1) -> Any:
		pass

	def index(self, value: Any, start: int = 0, end: int | None = None) -> int:
		pass

	def count(self, value: Any) -> int:
		pass

	def sort(self, key: Any = None, reverse: bool = False) -> None:
		pass

	def reverse(self) -> None:
		pass

	def clear(self) -> None:
		pass


class __builtins__dict:
	def __getitem__(self, key: Any) -> Any:
		pass

	def __setitem__(self, key: Any, value: Any) -> None:
		pass

	def __delitem__(self, key: Any) -> None:
		pass

	def __len__(self) -> int:
		pass

	def __repr__(self) -> str:
		pass

	def __contains__(self, key: Any) -> bool:
		pass

	def clear(self) -> None:
		pass

	def copy(self) -> Dict[Any, Any]:
		pass

	def get(self, key: Any, default: Any = None) -> Any:
		pass

	def items(self) -> Any:
		pass

	def keys(self) -> Any:
		pass

	def pop(self, key: Any, default: Any = None) -> Any:
		pass

	def popitem(self) -> Any:
		pass

	def setdefault(self, key: Any, default: Any = None) -> Any:
		pass

	def update(self, *args: Any, **kwargs: Any) -> None:
		pass

	def values(self) -> Any:
		pass


class __collections__deque:

	def __init__(self, iterable: Optional[Iterable] = [], maxlen: Optional[int] = None):
		pass

	def append(self, x: Any) -> None:
		pass

	def appendleft(self, x: Any) -> None:
		pass

	def clear(self) -> None:
		pass

	def copy(self) -> collections.deque:
		pass

	def count(self, x: Any) -> int:
		pass

	def extend(self, iterable: Iterable) -> None:
		pass

	def extendleft(self, iterable: Iterable) -> None:
		pass

	def pop(self) -> Any:
		pass

	def popleft(self) -> Any:
		pass

	def remove(self, value: Any) -> None:
		pass

	def reverse(self) -> None:
		pass

	def rotate(self, n: int = 1) -> None:
		pass


class __collections__defaultdict:
	def __init__(self, default_factory: Optional[Callable] = None):
		pass

	def __missing__(self, key: Any) -> Any:
		pass


class __collections__Counter:
	def __init__(self, iterable: Optional[Iterable] = None):
		pass

	def elements(self) -> Iterable:
		pass

	def most_common(self, n: Optional[int] = None) -> List[Tuple[Any, int]]:
		pass

	def subtract(self, iterable: Optional[Iterable] = None):
		pass


class __collections__OrderedDict:
	def __init__(self):
		pass


class __io__StringIO:
	def __init__(self, data: Any):
		pass


# DefaultClass returns signature for methods for any class, if the class is not found
class DefaultClass:
	def __getitem__(self, item: Any) -> Any:
		pass

	def __lt__(self, other: Any) -> bool:
		pass

	def __eq__(self, other: Any) -> bool:
		pass

	def __gt__(self, other: Any) -> bool:
		pass

	def __hash__(self) -> int:
		pass


def get_method_definition(module_name: str, class_name: str, method_name: str):
	modified_class_name = '__{}__{}'.format(module_name, class_name)
	for c in inspect.getmembers(sys.modules[__name__], inspect.isclass):
		if c[0] == modified_class_name:
			for m in inspect.getmembers(c[1], inspect.isfunction):
				if m[0] == method_name:
					return m[1]

	for m in inspect.getmembers(DefaultClass):
		if m[0] == method_name:
			return m[1]

	return None
