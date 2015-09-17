import sys, os, re
import subprocess

import downscaling
from binaries import *


if os.path.basename(os.path.normpath(os.getcwd())) != "hevc_transcoding-eharano":
	print "Run from \"hevc_transcoding-eharano\" directory"
	sys.exit(1)

if len(sys.argv) != 3:
	print "Usage: %s <hq_bitstream> <cfg-file>" % os.path.basename(sys.argv[0])
	sys.exit(1)


## Set up environment ##

hq_bitstream = sys.argv[1]
cfg          = sys.argv[2]

hq_bitstream_base = os.path.basename(hq_bitstream)
hq_bitstream_shortpath = os.path.splitext(hq_bitstream_base)[0]

(width, height) = downscaling.dimensions_from_filename(hq_bitstream_base)

"""
if height != 1080:
	raise Exception("Expected 1080p video")
"""

output_folder = "output_data"
sequence_folder = "%s/%s" % (output_folder, hq_bitstream_shortpath)

if not os.path.exists(output_folder):
	os.makedirs(output_folder)

if not os.path.exists(sequence_folder):
	os.makedirs(sequence_folder)

hq_bitstream_decoded_shortpath = "%s_dec" % hq_bitstream_shortpath
hq_bitstream_decoded = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_shortpath)


## Define transcoding parameters ##

"""Corresponds to 720, 540 and 360 for a 1080 video"""
"""
for parameter_set in [[1], [0], [1,0]]:
	downscaled_width = width
	downscaled_height = height

	for p in parameter_set:
		(downscaled_width, downscaled_height) = downscaling.convert_dimensions(width, height, downscale_parameter)
"""

downscale_parameter = [0,0]
(downscaled_width, downscaled_height) = downscaling.convert_dimensions_2(width, height, downscale_parameter)

#print (downscaled_width, downscaled_height)

## Define filenames and create folders ##


downscaled_file_shortpath = downscaling.filename_replace_dimensions(hq_bitstream_decoded_shortpath, downscaled_width, downscaled_height)
downscaled_file = "%s/%s.yuv" % (sequence_folder, downscaled_file_shortpath)

rdoq_0_file_shortpath = "%s_rdoq_0" % (downscaled_file_shortpath)
rdoq_0_file = "%s/%s.bin" % (sequence_folder, rdoq_0_file_shortpath)

pruned_file_shortpath = "%s_pruned" % rdoq_0_file_shortpath
pruned_file = "%s/%s.bin" % (sequence_folder, pruned_file_shortpath)

hq_bitstream_decoded_dec_order_shortpath = "%s_decoded_dec_order" % hq_bitstream_shortpath
hq_bitstream_decoded_dec_order = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_dec_order_shortpath)

hq_bitstream_decoded_dec_order_downscaled_shortpath = downscaling.filename_replace_dimensions(hq_bitstream_decoded_dec_order_shortpath, downscaled_width, downscaled_height)
hq_bitstream_decoded_dec_order_downscaled = "%s/%s.yuv" % (sequence_folder, hq_bitstream_decoded_dec_order_downscaled_shortpath)

reconstructed_file="${transcoded_folder}/${hq_bitstream_shortpath}_guided_transcoding_size_${half_width}x${half_height}"
reconstructed_file_shortpath = "%s_guided_transcoding" % downscaling.filename_replace_dimensions(hq_bitstream_shortpath, downscaled_width, downscaled_height)
reconstructed_file = "%s/%s.bin" % (sequence_folder, reconstructed_file_shortpath)

reconstructed_file_decoded_shortpath = "%s_dec" % reconstructed_file_shortpath
reconstructed_file_decoded = "%s/%s.yuv" % (sequence_folder, reconstructed_file_decoded_shortpath)


## Decode HQ bitstream ##
hq_decode_cmd = "%s -b %s -o %s" % (hm_decoder, hq_bitstream, hq_bitstream_decoded)
subprocess.call(hq_decode_cmd)

## Downscale ##
#downscaling_cmd = "%s %s %s %s %s %d" % (downscaler, width, height, hq_bitstream_decoded, downscaled_file, downscale_parameter)
#subprocess.call(downscaling_cmd)

downscaling.perform_downscaling(width, height, hq_bitstream_decoded, downscaled_file, downscale_parameter)
sys.exit(0)

## Re-encode with RDOQ=0 ##
rdoq_0_opts = "-fr 5 -f 10 -wdt %d -hgt %d --RDOQ=0 -SBH 0 --RDOQTS=0" % (downscaled_width, downscaled_height)
rdoq_0_cmd = "%s -c %s -i %s -b %s %s" % (hm_encoder, cfg, downscaled_file, rdoq_0_file, rdoq_0_opts)
subprocess.call(rdoq_0_cmd)

## Prune ##
prune_cmd = "%s -i %s -n %s" % (d65_gt_pruning, rdoq_0_file, pruned_file)
subprocess.call(prune_cmd)

## Decode HQ bitstream in decoding order and downscale ##
dec_order_cmd = "%s -i %s -o %s" % (d65_gt_dec_order, hq_bitstream, hq_bitstream_decoded_dec_order)
subprocess.call(dec_order_cmd)

dec_order_downscale_cmd = "%s %d %d %s %s %d" % (downscaler, width, height, hq_bitstream_decoded_dec_order, hq_bitstream_decoded_dec_order_downscaled, downscale_parameter)
subprocess.call(dec_order_downscale_cmd)

## Perform guided transcoding and decode ##
res_reconstruct_cmd = "%s -i %s -u %s -n %s" % (d65_gt_res_reconstruct, pruned_file, hq_bitstream_decoded_dec_order_downscaled, reconstructed_file)
subprocess.call(res_reconstruct_cmd)

res_reconstruct_decode_cmd = "%s -b %s -o %s" % (hm_decoder, reconstructed_file, reconstructed_file_decoded)
subprocess.call(res_reconstruct_decode_cmd)


## Cleanup ##
"""
os.remove(hq_bitstream_decoded)
os.remove(downscaled_file)
os.remove(rdoq_0_file)
os.remove(pruned_file)
os.remove(hq_bitstream_decoded_dec_order)
os.remove(hq_bitstream_decoded_dec_order_downscaled)
os.remove(reconstructed_file)
"""




"""
video_root = "//vhub.rnd.ki.sw.ericsson.se/proj/video_data/org"

tandberg   = "/tandberg/720p/RSM_Xmas_Tree.yuv"
race_horses = "/MPEG_CfP_seqs/orig-draft-cfp_2009-07-23/RaceHorses_832x480_30.yuv"

v = video_root + race_horses
"""