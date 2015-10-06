#!/usr/bin/env python

import subprocess, os
import paths, time_string, downscaling, filenames

import definitions.directories  as directories
import definitions.config       as config


orig_draft = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/"
originals = []

originals_shortpath = ["BQSquare_416x240_60.yuv", "RaceHorses_416x240_30.yuv"]
#originals_shortpath = ["BQSquare_416x240_60.yuv" ]
#originals_shortpath = ["BQTerrace_1920x1080_60.yuv","BasketballDrive_1920x1080_50.yuv","ParkScene_1920x1080_24.yuv","ChristmasTree_1920x1080_50.yuv"]

for seq in originals_shortpath:
    originals.append(orig_draft + seq)


""" [1], [0], [1,0] means two-thirds, half and one-third downscaling.
This corresponds to 720p, 536p and 360p for a 1080p video. """

#downscale_parameter_list = [ [1], [0], [1,0] ]
#downscale_parameter_list = [ [1,0,0], [1,0,0,0] ]
downscale_parameter_list = [ [0], [0,0] ]
#downscale_parameter_list = [ [0] ]

#QP_hq = [27]
#QP_hq = [27, 32]
QP_hq = [22, 27, 32]
#QP_hq = [22, 27, 32, 37]
QP_lq = [qp + 2 for qp in QP_hq]

bsub_cmds = []

for original_file in originals:

    original_file_basename = os.path.basename(original_file)
    original_file_shortpath = os.path.splitext(original_file_basename)[0]

    (width, height)  = filenames.extract_dimensions(original_file_shortpath)

    cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

    hq_bitstream_framerate_replaced = filenames.replace_framerate(original_file_shortpath, config.framerate)
    hq_bitstream_mode_info = "%s_%df_%s" % (hq_bitstream_framerate_replaced, config.frames, cfg_mode)
    
    for qp_hq in QP_hq:
        
        for downscale_parameters in downscale_parameter_list:

            (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
            downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

            downscale_parameters_string = str(downscale_parameters).replace(" ", "")
            
            for qp_lq in QP_lq:

                script_id = "%s_%s_%sp_%s" % (original_file_shortpath, qp_hq, downscaled_height, qp_lq)
                current_time = time_string.current()

                tmp_directory = "%s/%s_%s" % (directories.tmp_folder, current_time, script_id)
                paths.create_if_needed(tmp_directory)

                bsub_out = "%s/bsub.out" % tmp_directory
                bsub_err = "%s/bsub.err" % tmp_directory

                python_args = "%s %s %d %d %d %s %s %d %d %d %s %s" % \
                    (original_file, original_file_shortpath, width, height, qp_hq, hq_bitstream_mode_info, downscale_parameters_string, 
                    downscaled_width, downscaled_height, qp_lq, tmp_directory, current_time)

                bsub_cmd = "bsub -J %s -o %s -e %s python -u scripts/python/guided_transcoding_modular.py %s" % \
                    (script_id, bsub_out, bsub_err, python_args)

                bsub_cmds.append(bsub_cmd)


for i in range(len(QP_lq)):
    qp_lqs = bsub_cmds[i::len(QP_lq)]

    for j in range(len(downscale_parameter_list)):
        downscale_parameters = qp_lqs[j::len(downscale_parameter_list)]

        for k in range(len(QP_hq)):
            qp_hqs = downscale_parameters[k::len(QP_hq)]
            
            for job in qp_hqs:
                subprocess.call(job, shell=True)