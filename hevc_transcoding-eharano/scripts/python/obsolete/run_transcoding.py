#!/usr/bin/env python

import os, subprocess

import python.definitions.directories   as directories
import python.paths                     as paths
import python.guided_transcoding        as guided_transcoding


paths.assert_hevc_directory()

subprocess.call("reset", shell=True)

#bitstream = "/BQTerrace_1920x1080_1_randomaccess.bin"
#bitstream = "/BQTerrace_1920x1080_5_intra.bin"
bitstream = "/BQTerrace_1920x1080_10_intra.bin"

#bitstream = "/backup/backup_1/BQMall_832x480_60_cfg_encoder_intra_main_framerate_5.bin"
#bitstream = "/backup/backup_1/BQMall_832x480_60_cfg_encoder_randomaccess_main_framerate_5.bin"

#bitstream = "/BQSquare_416x240_1_10f_randomaccess/BQSquare_416x240_1_10f_randomaccess.bin"

bitstream_fullpath = os.path.normpath(directories.bitstream_folder + bitstream)

print "## Running transcoding ##"
#guided_transcoding.transcode(bitstream_fullpath)
guided_transcoding.transcode()