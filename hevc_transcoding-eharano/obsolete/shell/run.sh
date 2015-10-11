#!/bin/bash

reset

# Define bitstream

## These fail:
#bitstream=bitstreams/BQTerrace_1920x1080_5_randomaccess.bin

## These work:
#bitstream=bitstreams/BQTerrace_1920x1080_1_intra.bin
#bitstream=BQTerrace_1920x1080_1_randomaccess.bin
bitstream="BQTerrace_1920x1080_10_intra.bin"


echo "## Running transcoding ##"
python scripts/python/guided_transcoding.py $bitstream