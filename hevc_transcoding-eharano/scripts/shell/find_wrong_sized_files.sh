#!/bin/bash

clear

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

echo Non-zero log files:
for simulation in *; do
    sim_counter=0

    mkdir $err/$simulation

    for branch in $simulation/*; do
        log=$branch/bsub.err
 
        if [ -a $log ]; then
            size=$(stat $log --printf="%s")
            if [ $size != 0 ]; then
                echo $log
                cp $log $err/$branch.err
                ((++sim_counter))
            fi
        fi

    done

    if [ $sim_counter == 0 ]; then
        rmdir $err/$simulation
    else
        ((++err_counter))
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
