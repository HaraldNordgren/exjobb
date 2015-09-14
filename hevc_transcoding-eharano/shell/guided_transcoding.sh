#!/bin/bash

if [ $# -ne 6 ]; then
	echo "Usage: $0 <hq-bitstream> <decoded-hq-video> <height> <width> <config-file> <transcoded-output>"
	exit 1
fi

#Define binaries
HOME_PATH="${PWD}/../../../.."
#HOME_PATH="C:/Users/eharano/hevc_transcoding-eharano"
hm_encoder="${HOME_PATH}/bin/windows/HM-16.6/TAppEncoder.exe"
hm_decoder="${HOME_PATH}/bin/windows/HM-16.6/TAppDecoder.exe"
downscaler="${HOME_PATH}/bin/windows/downScale/downScale.exe"
d65_gt_pruning="${HOME_PATH}/d65_gt/bin/d65_dec_pruning.exe"
d65_gt_dec_order="${HOME_PATH}/d65_gt/bin/d65_dec_dec_order.exe"
d65_gt_res_reconstruct="${HOME_PATH}/d65_gt/bin/d65_dec_residual_reconstruct.exe"

#Catch input arguments
hq_bitstream=$1
hq_bitstream_base=$(basename $hq_bitstream)
hq_bitstream_shortpath="${hq_bitstream_base%.*}"

decoded_file=$2
decoded_file_base=$(basename $decoded_file)
decoded_file_shortpath="${decoded_file_base%.*}"

width=$3
height=$4
cfg=$5

transcoded_file=$6
transcoded_folder="${transcoded_file%/*}"

#Calculate downscaled dimensions
half_width=$(($width/2))
half_height=$(($height/2))

#Define filenames
downscaled_file="${transcoded_folder}/${hq_bitstream_shortpath}_downscaled_to_${half_width}x${half_height}"
rdoq_0_file="${downscaled_file}_rdoq_0"
pruned_file="${rdoq_0_file}_pruned"
hq_bitstream_decoded_dec_order="${transcoded_folder}/${hq_bitstream_shortpath}_decoded_dec_order"
hq_bitstream_decoded_dec_order_downscaled="${hq_bitstream_decoded_dec_order}_downscaled_to_${half_width}x${half_height}"
reconstructed_file="${transcoded_folder}/${hq_bitstream_shortpath}_guided_transcoding_size_${half_width}x${half_height}"

#Downscale to half-size
$downscaler $width $height $decoded_file ${downscaled_file}.yuv 0

#Re-encode with RDOQ=0
options="-fr 5 -f 10 -wdt $half_width -hgt $half_height --RDOQ=0 -SBH 0 --RDOQTS=0"
$hm_encoder -c $cfg -i ${downscaled_file}.yuv -b ${rdoq_0_file}.bin $options

#Prune
$d65_gt_pruning -i ${rdoq_0_file}.bin -n ${pruned_file}.bin

#Decode HQ bitstream in decoding order and downscale
$d65_gt_dec_order -i $hq_bitstream -o ${hq_bitstream_decoded_dec_order}.yuv
$downscaler $width $height ${hq_bitstream_decoded_dec_order}.yuv \
	${hq_bitstream_decoded_dec_order_downscaled}.yuv 0

#Perform guided transcoding and decode
$d65_gt_res_reconstruct -i ${pruned_file}.bin -u ${hq_bitstream_decoded_dec_order_downscaled}.yuv -n ${reconstructed_file}.bin
$hm_decoder -b ${reconstructed_file}.bin -o $transcoded_file

#Clean-up
#rm ${downscaled_file}.yuv
rm ${rdoq_0_file}.bin
rm ${pruned_file}.bin
rm ${hq_bitstream_decoded_dec_order}.yuv
rm ${hq_bitstream_decoded_dec_order_downscaled}.yuv
rm ${reconstructed_file}.bin