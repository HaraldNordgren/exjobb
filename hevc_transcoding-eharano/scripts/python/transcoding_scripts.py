#!/usr/bin/env python

import subprocess, os, sys
import paths, time_string, downscaling, filenames
import definitions.directories  as directories
import definitions.config       as config


orig_draft = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/"
originals = []

#originals_shortpath = ["BQSquare_416x240_60.yuv", "RaceHorses_416x240_30.yuv"]
#originals_shortpath = ["BQSquare_416x240_60.yuv" ]
originals_shortpath = ["BQTerrace_1920x1080_60.yuv", "BasketballDrive_1920x1080_50.yuv", "ParkScene_1920x1080_24.yuv", "ChristmasTree_1920x1080_50.yuv"]

for seq in originals_shortpath:
    originals.append(orig_draft + seq)


""" [1], [0], [1,0] means two-thirds, half and one-third downscaling.
This corresponds to 720p, 536p and 360p for a 1080p video. """

downscale_parameter_list = [ [1], [0], [1,0] ]
#downscale_parameter_list = [ [1,0,0], [1,0,0,0] ]
#downscale_parameter_list = [ [0], [0,0] ]
#downscale_parameter_list = [ [0] ]

#QP_hq = [27]
#QP_hq = [27, 32]
QP_hq = [22, 27, 32, 37]
QP_lq = [qp + 2 for qp in QP_hq]


#bsub_output_folder = "%s/bsub_output" % directories.output_folder

for original_file in originals:

    original_file_basename = os.path.basename(original_file)
    original_file_shortpath = os.path.splitext(original_file_basename)[0]

    (width, height)  = filenames.extract_dimensions(original_file_shortpath)

    cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

    hq_bitstream_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
    hq_bitstream_mode_info = "%s_%df_%s" % (hq_bitstream_framerate_replaced, config.frames, cfg_mode)
    
    for downscale_parameters in downscale_parameter_list:

        (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
        downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

        downscale_parameters_string = str(downscale_parameters).replace(" ", "")

        for qp_lq in QP_lq:

            for qp_hq in QP_hq:

                current_time = time_string.current()
                script_id = "%s_%s_%s_%sp_%s" % (time_string.current(), hq_bitstream_mode_info, qp_hq, downscaled_height, qp_lq)

                tmp_directory = "%s/%s" % (directories.tmp_folder, script_id)
                paths.create_if_needed(tmp_directory)

                bsub_out = "%s/bsub_out.txt" % tmp_directory
                bsub_err = "%s/bsub_err.txt" % tmp_directory

                bsub_cmd = "bsub -o %s -e %s python scripts/python/guided_transcoding_modular.py %s %d %s %d %s" % \
                    (bsub_out, bsub_err, original_file, qp_hq, downscale_parameters_string, qp_lq, current_time)
                
                #print "%s\n" % bsub_cmd
                subprocess.call(bsub_cmd, shell=True)