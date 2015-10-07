#!/bin/bash

reset

set -e
shopt -s globstar

time=`date +%y%m%d_%H%M`

err_shortpath=error_data/$time
err=$PWD/$err_shortpath

err_counter=0

cd output_data

echo Zero size output files:
for f in **; do
    filename=$(basename "$f")
    ex="${filename##*.}"

    if [ $ex == "bin" ] || [ $ex == "yuv" ] ; then
        size=$(stat $f --printf="%s")
        if [ $size == 0 ]; then
            echo $f
            ((++err_counter))
        fi
    fi
done

if [ $err_counter == 0 ]; then
    echo None
fi
echo

err_counter=0

cd tmp
mkdir -p $err

echo "Non-zero log files:"
for folder in *; do
    log=${folder}/bsub.err
    if [ -a $log ]; then
        size=$(stat $log --printf="%s")
        if [ $size != 0 ]; then
            echo $log
            cp $log $err/$folder.err
            ((++err_counter))
        fi
    fi
done

if [ $err_counter == 0 ]; then
    echo None
    rmdir $err
else
    echo
    echo "(Copied to $err_shortpath)"
fi

echo
