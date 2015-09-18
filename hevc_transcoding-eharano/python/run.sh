#!/bin/bash

reset

echo "## Deleting output folder"
rm output_data -r
echo


#bitstream=bitstreams/BQMall_832x480_60_cfg_encoder_intra_main_framerate_5.bin
bitstream=bitstreams/BQTerrace_1920x1080_5_randomaccess.bin
#bitstream=bitstreams/backup/backup_1/BQMall_832x480_60_cfg_encoder_randomaccess_main_framerate_5.bin

echo "## Running transcoding ##"
python python/guided_transcoding.py $bitstream