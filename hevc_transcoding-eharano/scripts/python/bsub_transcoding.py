#!/usr/bin/env python

import subprocess, os
import paths, time_string, downscaling, filenames

import definitions.directories  as directories
import definitions.config       as config


configuration = 1

if configuration == 1:
    originals_shortpath = [ "BQSquare_416x240_60.yuv" ]
    #originals_shortpath = [ "BQSquare_416x240_60.yuv", "RaceHorses_416x240_30.yuv" ]
    downscale_parameter_list = [ [0], [0,0] ]
    QP_hq = [22, 27, 32, 37]
    frames = 10

elif configuration == 2:
    originals_shortpath = [ "BQTerrace_1920x1080_60.yuv" ]
    downscale_parameter_list = [ [1], [0], [1,0] ]
    QP_hq = [22, 27, 32, 37]
    frames = 2
    #frames = config.all_frames

elif configuration == 3:
    originals_shortpath = \
        [ "BQTerrace_1920x1080_60.yuv", "BasketballDrive_1920x1080_50.yuv", "ParkScene_1920x1080_24.yuv", "ChristmasTree_1920x1080_50.yuv" ]
    downscale_parameter_list = [ [1], [0], [1,0] ]
    QP_hq = [22, 27, 32, 37]
    frames = config.all_frames



QP_lqs = 2
qp_delta = 2

originals = [ "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/" + seq for seq in originals_shortpath ]

current_time = time_string.current()
simulation_directory = "%s/%s" % (directories.tmp_folder, current_time)

bsub_cmds = []



""" [1], [0], [1,0] means two-thirds, half and one-third downscaling.
This corresponds to 720p, 536p and 360p for a 1080p video. """

for original_file in originals:

    original_file_basename = os.path.basename(original_file)
    original_file_shortpath = os.path.splitext(original_file_basename)[0]

    (width, height)  = filenames.extract_dimensions(original_file_shortpath)

    cfg_mode = filenames.extract_cfg_mode(config.cfg_file)

    if config.preserve_framerate:
        framerate = filenames.extract_framerate(original_file_shortpath)
        hq_bitstream_mode_info = "%s_%df_%s" % (original_file_shortpath, frames, cfg_mode)

    else:
        framerate = config.new_framerate
        hq_bitstream_framerate_replaced = filenames.replace_framerate(original_file_shortpath, framerate)
        hq_bitstream_mode_info = "%s_%df_%s" % (hq_bitstream_framerate_replaced, frames, cfg_mode)
    
    for qp_hq in QP_hq:
        
        for downscale_parameters in downscale_parameter_list:

            (downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameters)
            downscaled_height = downscaling.get_height_divisible_by_eight(downscaled_height)

            downscale_parameters_string = str(downscale_parameters).replace(" ", "")
            
            for i in range(QP_lqs):
                qp_lq = qp_hq + i * qp_delta

                script_id = "%s_%df_qp%s_%sp_qp%s" % (original_file_shortpath, frames, qp_hq, downscaled_height, qp_lq)
                current_time = time_string.current()

                tmp_directory = "%s/%s" % (simulation_directory, script_id)
                paths.create_if_needed(tmp_directory)

                bsub_out = "%s/bsub.out" % tmp_directory
                bsub_err = "%s/bsub.err" % tmp_directory

                python_args = "%s %s %d %d %d %d %d %s %s %d %d %d %s %s" % \
                    (original_file, original_file_shortpath, width, height, qp_hq, framerate, frames, hq_bitstream_mode_info, downscale_parameters_string, 
                    downscaled_width, downscaled_height, qp_lq, tmp_directory, current_time)

                bsub_cmd = "bsub -J %s -o %s -e %s python -u scripts/python/guided_transcoding_modular.py %s" % \
                    (script_id, bsub_out, bsub_err, python_args)

                bsub_cmds.append(bsub_cmd)


for i in range(QP_lqs):
    qp_lqs = bsub_cmds[i::QP_lqs]

    for j in range(len(downscale_parameter_list)):
        downscale_parameters = qp_lqs[j::len(downscale_parameter_list)]

        for k in range(len(QP_hq)):
            qp_hqs = downscale_parameters[k::len(QP_hq)]
            
            for job in qp_hqs:
                subprocess.call(job, shell=True)