#!/usr/bin/python

import os, subprocess
import python.definitions


subprocess.call("reset", shell=True)

#bitstream = "BQTerrace_1920x1080_1_randomaccess.bin"
bitstream = "BQTerrace_1920x1080_10_intra.bin"

bitstream_fullpath = os.path.abspath(python.definitions.bitstream_folder + "/" + bitstream)

guided_transcoding = os.path.abspath("scripts/python/guided_transcoding.py")

gt_cmd = "python %s %s" % (guided_transcoding, bitstream_fullpath)
subprocess.call(gt_cmd, shell=True)