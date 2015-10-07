import sys
sys.path.append("scripts/python")
sys.path.append("scripts/python/definitions")

import subprocess, os, time
import filenames, downscaling, paths, raw_video, time_string, command_line
import directories
import binaries


input_file = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/BQSquare_416x240_60.yuv"

input_file_basename = os.path.basename(input_file)
input_file_shortpath = os.path.splitext(input_file_basename)[0]

(width, height) = filenames.extract_dimensions(input_file_basename)

downscaling_folder = "%s/%s_downscaling_test_%s" % (directories.test_folder, input_file_shortpath, time_string.current())
paths.create_if_needed(downscaling_folder)

parameters  = [0,0,0]
half        = 0

counter     = 1

for p in parameters:
    (downscaled_width_1, downscaled_height_1) = downscaling.do_conversion(width, height, 0)

    if p == 1:
        downscaled_height_1 = downscaling.get_height_divisible_by_eight(downscaled_height_1)

    intermediate_file_shortpath = filenames.replace_dimensions(input_file_basename, downscaled_width_1, downscaled_height_1)
    intermediate_file = "%s/%s" % (downscaling_folder, intermediate_file_shortpath)

    #print "intermediate:\t%s" % intermediate_file
    
    downscaling_cmd_1 = "%s %s %s %s %s %d %d" % \
        (binaries.downscaler, width, height, input_file, intermediate_file, half, p)
    command_line.call_indented(downscaling_cmd_1)

    raw_video.mux(intermediate_file)

    for q in parameters:
        (downscaled_width_2, downscaled_height_2) = downscaling.do_conversion(downscaled_width_1, downscaled_height_1, half)

        if q == 1:
            downscaled_height_2 = downscaling.get_height_divisible_by_eight(downscaled_height_2)

        end_file = filenames.replace_dimensions(intermediate_file, downscaled_width_2, downscaled_height_2)
        
        (end_file_basename, ext) = os.path.splitext(end_file)
        end_file = "%s_c%d%s" % (end_file_basename, counter, ext)
        counter += 1

        downscaling_cmd_2 = "%s %s %s %s %s %d %d" % \
            (binaries.downscaler, downscaled_width_1, downscaled_height_1, intermediate_file, end_file, half, q)
        command_line.call_indented(downscaling_cmd_2)

        raw_video.mux(end_file)

        print "%s" % end_file

    print
