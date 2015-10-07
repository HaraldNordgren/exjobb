#!/usr/bin/env python

import subprocess, pty, os

"""
p = subprocess.Popen("./test_out.py", shell=True, stdout=subprocess.PIPE, bufsize=0)
#p = subprocess.call("./test_out.py", shell=True, stdout=subprocess.PIPE)

line = p.stdout.readline()
while line:
    print line
    line = p.stdout.readline()

#for line in p.stdout:
#    print line
"""

(master, slave) = pty.openpty()

p = subprocess.Popen("./test_out.py", shell=True, stdout=slave, close_fds=True)
stdout = os.fdopen(master)

line = stdout.readline()
while line:
    print line
    line = stdout.readline()
