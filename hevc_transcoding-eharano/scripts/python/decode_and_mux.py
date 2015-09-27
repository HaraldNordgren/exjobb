#!/usr/bin/env python

import os, sys, raw_video

if len(sys.argv) != 2:
    print "Usage: %s <bitstream/yuv-file>" % os.path.basename(sys.argv[0])
    sys.exit(1)

input_file = sys.argv[1]
(input_file_basename, file_extension) = os.path.splitext(input_file)

bin_ext = ".bin"
yuv_ext = ".yuv"

if not (file_extension == bin_ext or file_extension == yuv_ext):
    print "Supply bin or yuv file"
    sys.exit(1)

if file_extension == bin_ext:
    decoded_file = raw_video.decode(input_file)
    input_file = decoded_file

raw_video.mux(input_file)