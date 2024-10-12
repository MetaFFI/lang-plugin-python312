# python script to run unitests for api using subprocess

import importlib
import sys

def ensure_package(package_name, pip_package_name=None):
	try:
		importlib.import_module(package_name)
	except ImportError:
		import subprocess
		import sys
		print(f"Installing {package_name}...")
		
		if pip_package_name is None:
			pip_package_name = package_name
			
		subprocess.check_call([sys.executable, "-m", "pip", "install", pip_package_name])
		
		print(f"{package_name} installed successfully!")

ensure_package("colorama")


import glob
import subprocess
import os
import sys
from colorama import init, Fore, Style
import platform

# Initialize colorama
init()

# Get the current path of this Python script
current_path = os.path.dirname(os.path.abspath(__file__))


def get_extension_by_platform() -> str:
	if platform.system() == 'Windows':
		return '.dll'
	elif platform.system() == 'Darwin':
		return '.dylib'
	else:
		return '.so'


# Define a function to run a script and print its output
def run_script(script_path):
	print(f'{Fore.CYAN}Running script: {script_path}{Fore.RESET}')
	
	if script_path.endswith('.py'):
		# Python script
		command = [sys.executable, script_path]
	else:
		raise ValueError(f'Unsupported script file type: {script_path}')
	
	script_dir = os.path.dirname(os.path.abspath(script_path))
	
	process = subprocess.Popen(command, cwd=script_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	
	stdout, stderr = process.communicate()
	print(stdout, end='')
	print(stderr, file=sys.stderr, end='')
	
	if process.returncode != 0:
		raise subprocess.CalledProcessError(process.returncode, command)


def run_unittest(script_path):
	print(f'{Fore.CYAN}Running unittest: {script_path}{Fore.RESET}')
	
	if script_path.endswith('.py'):
		# Python unittest
		command = ['py', '-3.11', script_path] if platform.system() == 'Windows' else ['python3.11', script_path]
	elif script_path.endswith('.java'):
		# Java JUnit test
		junit_jar = os.path.join(current_path, 'junit-platform-console-standalone-1.10.2.jar')
		hamcrest_jar = os.path.join(current_path, 'hamcrest-core-1.3.jar')
		bridge_jar = os.path.join(os.environ['METAFFI_HOME'], 'openjdk', 'xllr.openjdk.bridge.jar')
		api_jar = os.path.join(os.environ['METAFFI_HOME'], 'openjdk', 'metaffi.api.jar')
		class_name = os.path.splitext(os.path.basename(script_path))[0]
		class_path = f'.{os.pathsep}{junit_jar}{os.pathsep}{hamcrest_jar}{os.pathsep}{bridge_jar}{os.pathsep}{api_jar}'
		
		# Compile the Java source file
		compile_command = ['javac', '-cp', class_path, script_path]
		print(f'{Fore.BLUE}Running - {" ".join(compile_command)}{Fore.RESET}')
		subprocess.run(compile_command, check=True)
		
		# Run the JUnit test
		command = ['java', '-jar', junit_jar, '-cp', class_path, '-c', class_name]
	elif script_path.endswith('.go'):
		goget_command = ['go', 'get', '-u']
		print(f'{Fore.BLUE}Running - {" ".join(goget_command)}{Fore.RESET}')
		subprocess.run(goget_command, check=True, cwd=os.path.dirname(script_path))
		
		command = ['go', 'run', script_path]
	else:
		raise ValueError(f'Unsupported unittest file type: {script_path}')
	
	script_dir = os.path.dirname(os.path.abspath(script_path))
	
	print(f'{Fore.BLUE}Running - {" ".join(command)}{Fore.RESET}')
	process = subprocess.Popen(command, cwd=script_dir, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
	
	stdout, stderr = process.communicate()
	
	print(stdout, end='')
	print(stderr, file=sys.stderr, end='')
	
	if process.returncode != 0:
		raise subprocess.CalledProcessError(process.returncode, command)
	
	# If it's a Java unittest, delete the compiled .class file
	if script_path.endswith('.java'):
		class_files = glob.glob(os.path.join(script_dir, "*.class"))
		for file in class_files:
			os.remove(file)


def is_plugin_installed(plugin_name: str) -> bool:
	plugin_dir = os.path.join(os.environ['METAFFI_HOME'], plugin_name)
	return os.path.exists(plugin_dir)

if not is_plugin_installed('openjdk') and not is_plugin_installed('go'):
	print(f"{Fore.RED}No other plugins installed. Skipping tests...{Fore.RESET}")
	sys.exit(0)

# --------------------------------------------

# --------------------------------------------

# sanity tests

# --------------------------------------------

# run python3.11->openjdk tests
if is_plugin_installed('openjdk'):
	print(f'{Fore.MAGENTA}Testing Sanity Python3.11 -> OpenJDK{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

	# Define the paths to the scripts to be run
	build_sanity_openjdk_script_path = os.path.join(current_path, 'sanity', 'openjdk', 'build_java_code.py')
	test_sanity_openjdk_path = os.path.join(current_path, 'sanity', 'openjdk', 'openjdk_test.py')

	# Run the scripts
	run_script(build_sanity_openjdk_script_path)
	run_unittest(test_sanity_openjdk_path)

	print(f'{Fore.MAGENTA}Testing Sanity Python3.11 -> OpenJDK{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')

# --------------------------------------------

# run python3.11->Go tests
if is_plugin_installed('go'):
	print(f'{Fore.MAGENTA}Testing Sanity Python3.11 -> Go{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

	# Define the paths to the scripts to be run
	build_sanity_go_script_path = os.path.join(current_path, 'sanity', 'go', 'build_metaffi.py')
	test_sanity_go_path = os.path.join(current_path, 'sanity', 'go', 'go_test.py')

	# Run the scripts
	run_script(build_sanity_go_script_path)
	run_unittest(test_sanity_go_path)

	print(f'{Fore.MAGENTA}Testing Sanity Python3.11 -> Go{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')
	os.remove(os.path.join(current_path, 'sanity', 'go', f'TestRuntime_MetaFFIGuest{get_extension_by_platform()}'))

# --------------------------------------------

# extended tests

# --------------------------------------------

# run python3.11->OpenJDK tests
if is_plugin_installed('openjdk'):
	print(f'{Fore.MAGENTA}Testing Extended Python3.11 -> OpenJDK{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

	# Define the path to the unittest script
	test_extended_openjdk_path = os.path.join(current_path, 'extended', 'openjdk', 'log4j', 'log4j_test.py')

	# Run the script
	run_unittest(test_extended_openjdk_path)

	print(f'{Fore.MAGENTA}Testing Extended Python3.11 -> OpenJDK{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')

# --------------------------------------------

# run python3.11->Go tests
if is_plugin_installed('go'):
	print(f'{Fore.MAGENTA}Testing Extended Python3.11 -> Go{Fore.RESET} - {Fore.YELLOW}RUNNING{Fore.RESET}')

	# Define the paths to the scripts to be run
	build_extended_go_script_path = os.path.join(current_path, 'extended', 'go', 'gomcache', 'build_metaffi.py')
	test_extended_go_path = os.path.join(current_path, 'extended', 'go', 'gomcache', 'gomcache_test.py')

	# Run the scripts
	run_script(build_extended_go_script_path)
	run_unittest(test_extended_go_path)

	print(f'{Fore.MAGENTA}Testing Extended Python3.11 -> Go{Fore.RESET} - {Fore.GREEN}PASSED{Fore.RESET}')
	os.remove(os.path.join(current_path, 'extended', 'go', 'gomcache', f'mcache_MetaFFIGuest{get_extension_by_platform()}'))
