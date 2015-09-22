import os, subprocess
from definitions import *


subprocess.call("reset", shell=True)

guided_transcoding = os.path.abspath("python/guided_transcoding.py")

bitstream = "BQTerrace_1920x1080_1_randomaccess.bin"
bitstream_fullpath = os.path.abspath(bitstream_folder + "/" + bitstream)

gt_cmd = "python %s %s" % (guided_transcoding, bitstream_fullpath)

print "## Running transcoding ##"
subprocess.call(gt_cmd, shell=True)