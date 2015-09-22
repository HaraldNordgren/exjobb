#!/bin/bash

reset

#echo "## Deleting output folder"
#rm output_data -r
#echo


## These fail: ##
#bitstream=bitstreams/BQTerrace_1920x1080_5_randomaccess.bin

## These work: ##
#bitstream=bitstreams/BQTerrace_1920x1080_1_intra.bin
bitstream=BQTerrace_1920x1080_1_randomaccess.bin

echo "## Running transcoding ##"
python python/guided_transcoding.py $bitstream