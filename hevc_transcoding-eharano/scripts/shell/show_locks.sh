#!/bin/bash

storage=/proj/video_data/videosim/eharano/storage

if [ ! -d $storage ]; then
    echo "$storage doesn't exist!"
    exit 1
fi

cd $storage

bjobs_out=`bjobs 2>&1`

while [[ "$bjobs_out" != "No unfinished job found" ]]; do
    clear
    echo Current locks:
    find -name *.lock | sed 's/^/\n/'
    sleep 1
    bjobs_out=`bjobs 2>&1`
done

clear
