#!/usr/bin/env python

import subprocess

orig_draft = "sample_videos/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23"
#originals = [ orig_draft + "/BQSquare_416x240_60.yuv", orig_draft + "/RaceHorses_416x240_30.yuv"]
originals = [ orig_draft + "/BQSquare_416x240_60.yuv" ]

""" [1], [0], [1,0] means two-thirds, half and one-third downscaling.
This corresponds to 720p, 536p and 360p for a 1080p video. """

#downscale_parameter_list = [ [1], [0], [1,0] ]
#downscale_parameter_list = [ [1,0,0], [1,0,0,0] ]
#downscale_parameter_list = [ [0], [0,0] ]
downscale_parameter_list = [ [0] ]

#QP_hq = [22, 27, 32, 37]
QP_hq = [27]
QP_lq = [qp + 2 for qp in QP_hq]



for original_file in originals:
    for qp_hq in QP_hq:
        for downscale_parameters in downscale_parameter_list:
            for qp_lq in QP_lq:

            	bsub_cmd = "bsub python scripts/python/guided_transcoding_modular.py %s %d %s %d" % (original_file, qp_hq, downscale_parameters, qp_lq)
            	print bsub_cmd
            	#subprocess.call(bsub_cmd, shell=True)