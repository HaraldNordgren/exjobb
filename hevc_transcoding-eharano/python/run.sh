#!/bin/bash

reset

echo "## Deleting output folder"
rm output_data -r
echo

echo "## Running transcoding ##"

#bit=bitstreams/BQMall_832x480_60_cfg_encoder_intra_main_framerate_5.bin
#bit=bitstreams/BQTerrace_1920x1080_60_cfg_encoder_intra_main_framerate_10.bin
bit=../videos/yuv/tulips_176x144_yuyv422_inter_packed_qcif.yuv

python python/guided_transcoding.py $bit HM-16.6/cfg/encoder_intra_main.cfg