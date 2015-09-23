import os, subprocess
from definitions import *


subprocess.call("reset", shell=True)

#bitstream = "BQTerrace_1920x1080_1_randomaccess.bin"
bitstream = "BQTerrace_1920x1080_10_intra.bin"

bitstream_fullpath = os.path.abspath(bitstream_folder + "/" + bitstream)

guided_transcoding = os.path.abspath("python/guided_transcoding.py")

gt_cmd = "python %s %s" % (guided_transcoding, bitstream_fullpath)
subprocess.call(gt_cmd, shell=True)