import os

import subprocess
import filenames
import paths

from binaries import *
from definitions import *


if os.path.basename(os.path.normpath(os.getcwd())) != "hevc_transcoding-eharano":
	print "Run from \"hevc_transcoding-eharano\" directory"
	sys.exit(1)


original_file  = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQTerrace_1920x1080_60.yuv"

original_file_basename = os.path.basename(original_file)
original_file_shortpath = os.path.splitext(original_file_basename)[0]

cfg_file_basename = os.path.basename(cfg_file)
cfg_mode = filenames.extract_cfg_mode(cfg_file_basename)

(width, height)  = filenames.extract_dimensions(original_file_shortpath)


output_file_shortpath = filenames.replace_framerate(original_file_shortpath, framerate)
output_file = "%s/%s_%s.bin" % (bitstream_folder, output_file_shortpath, cfg_mode)

paths.create_if_needed(bitstream_folder)

encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -hgt %s -wdt %s -SBH 1" % (hm_encoder, cfg_file, original_file, output_file, framerate, frames, height, width)
subprocess.call(encode_cmd, shell=True)