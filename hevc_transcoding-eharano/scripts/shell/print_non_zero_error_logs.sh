#!/bin/bash

reset

set -e

err=$PWD/error_data/$1

tmp=output_data/tmp
cd $tmp

mkdir -p $err

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
