from genericpath import isdir
import platform
from typing import List, Tuple, Dict
import glob
import os
from colorama import Fore

# ret[0] - win files, ret[1] - ubuntu files

def get_files(win_metaffi_home: str, ubuntu_metaffi_home: str) -> Tuple[Dict[str, str], Dict[str, str]]:
	# get all files from $METAFFI_HOME/go - the installed dir of this project recursively
	# don't continue recursively if the directory starts with '__'

	pluginname = 'python312'
	
	win_metaffi_home = win_metaffi_home.replace('\\', '/')
	ubuntu_metaffi_home = ubuntu_metaffi_home.replace('\\', '/')

	win_files = {}
	for file in glob.glob(win_metaffi_home + f'/{pluginname}/**', recursive=True):		
		if os.path.isfile(file) and '__' not in file:
			file = file.replace('\\', '/')
			win_files[file.removeprefix(win_metaffi_home+f'/{pluginname}/')] = file

	assert len(win_files) > 0, f'No files found in {win_metaffi_home}/{pluginname}'

	ubuntu_files = {}
	for file in glob.glob(ubuntu_metaffi_home + f'/{pluginname}/**', recursive=True):
		if os.path.isfile(file) and '__' not in file:
			file = file.replace('\\', '/')
			ubuntu_files[file.removeprefix(ubuntu_metaffi_home+f'/{pluginname}/')] = file

	assert len(ubuntu_files) > 0, f'No files found in {ubuntu_metaffi_home}/{pluginname}'

	# * copy the api tests
	current_script_dir = os.path.dirname(os.path.abspath(__file__))
	api_tests_files = glob.glob(f'{current_script_dir}/api/tests/**', recursive=True)
	for file in api_tests_files:
		if '__pycache__' in file:
			continue

		if os.path.isfile(file):
			target = file.replace('\\', '/').removeprefix(current_script_dir.replace('\\', '/')+'/api/')
			win_files[target] = file
			ubuntu_files[target] = file

	# * uninstaller
	win_files['uninstall_plugin.py'] = os.path.dirname(os.path.abspath(__file__))+'/uninstall_plugin.py'
	ubuntu_files['uninstall_plugin.py'] = os.path.dirname(os.path.abspath(__file__))+'/uninstall_plugin.py'

	return win_files, ubuntu_files


def setup_environment():
	# install metaffi-api PIP package
	# install colorama (needed for tests)
	import subprocess
	import sys

	res = subprocess.run(f'{sys.executable} -m pip install metaffi-api colorama', shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	if res.returncode != 0:
		raise Exception(f"""Failed to install metaffi-api and colorama:\n{res.stdout}{res.stderr}""")


def check_prerequisites() -> bool:
	if platform.system() == 'Windows':
		if os.system('py -3.11 --version') != 0:
			print('Python3.11 is not installed')
			return False
	elif platform.system() == 'Linux':
		if os.system('python3.11 --version') != 0:
			print('Python3.11 is not installed')
			return False
	else:
		raise Exception(f'Unsupported platform: {platform.system()}')

	return True

def print_prerequisites():
	print("""Prerequisites:\n\tPython3.11 (for ubuntu python3.11-dev package is also required)""")
	