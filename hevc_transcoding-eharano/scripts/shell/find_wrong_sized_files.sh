#!/bin/bash

reset

set -e
shopt -s globstar

time=`date +%y%m%d_%H%M`

err_shortpath=error_data/$time
err=$PWD/$err_shortpath

cd output_data

echo Zero size files:
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
echo

cd tmp
mkdir -p $err

echo "Non-zero log files (Copied to $err_shortpath):"
for folder in *; do
    log=${folder}/bsub.err
    if [ -a $log ]; then
        size=$(stat $log --printf="%s")
        if [ $size != 0 ]; then
            echo $log
            cp $log $err/$folder.err
        fi
    fi
done
echo
