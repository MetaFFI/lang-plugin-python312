import os
import subprocess
import sys
import threading


# Function to handle output
def handle_output(p, handler):
	for line in iter(p.readline, b''):
		handler(line.decode())
	p.close()


# get the current path of the current python file
current_path = os.path.dirname(os.path.abspath(__file__))

# Ensure the output directory exists
os.makedirs(current_path, exist_ok=True)

# Find all Java files in the current directory
java_files = [f for f in os.listdir(current_path) if f.endswith('.java')]

# Compile each Java file
# Define the command to compile all Java files
command = ['javac', '-d', current_path] + java_files

# Execute the command
print('Going to run: '+' '.join(command))
process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=os.environ)

# Create threads to handle stdout and stderr
stdout_thread = threading.Thread(target=handle_output, args=(process.stdout, sys.stdout.write))
stderr_thread = threading.Thread(target=handle_output, args=(process.stderr, sys.stderr.write))

# Start the threads
stdout_thread.start()
stderr_thread.start()

# Wait for the threads to finish
stdout_thread.join()
stderr_thread.join()

# Wait for the process to finish and get the exit code
exit_code = process.wait()

# If the subprocess failed, raise an error
if exit_code != 0:
	raise subprocess.CalledProcessError(exit_code, command)
