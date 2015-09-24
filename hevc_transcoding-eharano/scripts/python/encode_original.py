import os, sys, subprocess
import filenames, paths, binaries, definitions


paths.assert_hevc_directory()

if len(sys.argv) != 2:
    print "Usage: %s <original.yuv>" % os.path.basename(sys.argv[0])
    sys.exit(1)

original_file = sys.argv[1]
#original_file  = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQTerrace_1920x1080_60.yuv"

original_file_basename = os.path.basename(original_file)
original_file_shortpath = os.path.splitext(original_file_basename)[0]

cfg_file_basename = os.path.basename(definitions.cfg_file)
cfg_mode = filenames.extract_cfg_mode(cfg_file_basename)

(width, height)  = filenames.extract_dimensions(original_file_shortpath)


output_file_shortpath = filenames.replace_framerate(original_file_shortpath, definitions.framerate)
output_file = "%s/%s_%s.bin" % (definitions.bitstream_folder, output_file_shortpath, cfg_mode)

paths.create_if_needed(definitions.bitstream_folder)

encode_cmd = "%s -c %s -i %s -b %s -fr %s -f %s -hgt %s -wdt %s -SBH 1" % \
	(binaries.hm_encoder, definitions.cfg_file, original_file, output_file, definitions.framerate, definitions.frames, height, width)
subprocess.call(encode_cmd, shell=True)