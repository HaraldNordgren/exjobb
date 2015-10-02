import subprocess, os, sys

indentation_level = "  "

#terminal_size = subprocess.check_output("stty size", shell=True)
#terminal_width = int(terminal_size.split()[1])
terminal_width = 80

def call_indented(cmd):

	print cmd

	# Setting fold with terminal width minus 2 to compensate for indentation
	indented_cmd = "%s | fold -s -w %s | sed 's/^/%s/g'" % (cmd, terminal_width - 2, indentation_level)
	#indented_cmd = cmd
	
	subprocess.call(indented_cmd, shell=True)

	print