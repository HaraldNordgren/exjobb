import subprocess, os, sys

indentation_level = "  "

def call_indented(cmd, err_log_path=None):

	print cmd

	terminal_size = subprocess.check_output("stty size", shell=True)
	terminal_width = int(terminal_size.split()[1])
	#terminal_width = 80

	# Setting fold with terminal width minus 2 to compensate for indentation
	indented_cmd = "%s | fold -s -w %s | sed 's/^/%s/g'" % (cmd, terminal_width - 2, indentation_level)
	#indented_cmd = cmd
	
	if err_log_path:
		err_log = open(err_log_path, 'a+')
		subprocess.call(indented_cmd, shell=True, stderr=err_log)
		err_log.close()	
	else:
		subprocess.call(indented_cmd, shell=True)

	print