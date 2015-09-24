#!/usr/bin/python

import os, subprocess

import python.definitions.directories
import python.paths
import python.guided_transcoding


python.paths.assert_hevc_directory()

subprocess.call("reset", shell=True)

#bitstream = "/BQTerrace_1920x1080_1_randomaccess.bin"
bitstream = "/BQTerrace_1920x1080_10_intra.bin"

bitstream_fullpath = os.path.abspath(python.definitions.directories.bitstream_folder + bitstream)

python.guided_transcoding.transcode(bitstream_fullpath)