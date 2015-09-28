#!/usr/bin/env python

import os, sys, subprocess
import filenames, paths, command_line

import definitions.config       as config
import definitions.binaries     as binaries
import definitions.directories  as directories

paths.assert_hevc_directory()

if len(sys.argv) != 2:
    print "Usage: %s <yuv-file>" % os.path.basename(sys.argv[0])
    sys.exit(1)


# Input file

original_file = sys.argv[1]
#original_file  = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQTerrace_1920x1080_60.yuv"

original_file_basename = os.path.basename(original_file)
original_file_shortpath = os.path.splitext(original_file_basename)[0]

cfg_file_basename = os.path.basename(config.cfg_file)
cfg_mode = filenames.extract_cfg_mode(cfg_file_basename)

(width, height)  = filenames.extract_dimensions(original_file_shortpath)


# Output file

output_file_new_framerate = filenames.replace_framerate(original_file_shortpath, config.framerate)
output_file_shortpath = "%s_%df_%s" % (output_file_new_framerate, config.frames, cfg_mode)

output_folder = "%s/%s" % (directories.bitstream_folder, output_file_shortpath)
paths.create_if_needed(output_folder)

output_file = "%s/%s.bin" % (output_folder, output_file_shortpath)
print output_file

encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -wdt %s -hgt %s -SBH 1 --SEIDecodedPictureHash=2" % \
	(binaries.hm_encoder, config.cfg_file, original_file, output_file, config.framerate, config.frames, width, height)
#subprocess.call(encode_cmd, shell=True)
command_line.call_indented(encode_cmd)