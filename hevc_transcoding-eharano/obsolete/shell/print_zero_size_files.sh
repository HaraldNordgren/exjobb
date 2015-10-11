#!/bin/bash

reset
set -e

shopt -s globstar

output=output_data
cd $output

for f in **; do

    filename=$(basename "$f")
    ex="${filename##*.}"

    if [ $ex == "bin" ] || [ $ex == "yuv" ] ; then
        size=$(stat $f --printf="%s")
        if [ $size == 0 ]; then
            echo $f
        fi
    fi

done
