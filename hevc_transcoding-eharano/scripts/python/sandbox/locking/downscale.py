#!/usr/bin/env python

import subprocess

original = "original/BQSquare_208x120_60.yuv"
downscaled = "output/BQSquare_104x60_60.yuv"

cmd = "downScale 208 120 %s %s 0" % (original, downscaled)
subprocess.call(cmd, shell=True)
