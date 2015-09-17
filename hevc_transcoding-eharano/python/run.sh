#!/bin/bash

reset

echo "## Deleting output folder"
rm output_data -r
echo

echo "## Running transcoding ##"
#python python/guided_transcoding.py bitstreams/BQTerrace_1920x1080_60_cfg_encoder_intra_main_framerate_10.bin HM-16.6/cfg/encoder_intra_main.cfg
python python/guided_transcoding.py bitstreams/BQMall_832x480_60_cfg_encoder_intra_main_framerate_5.bin HM-16.6/cfg/encoder_intra_main.cfg