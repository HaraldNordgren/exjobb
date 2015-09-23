#!/bin/bash


api=d65_lib/d65_api.h

function regex_replace {
    perl -pi -e 's/(#define '$1'\s*)\d/${1}'$2'/' $api
}

function build_version {
    name=$1
    dec_order=$2
    pruning=$3
    residual_reconstruct=$4

    regex_replace 'D65_OUTPUT_IN_DEC_ORDER' $dec_order
    regex_replace 'GT_RESIDUAL_PRUNING'     $pruning
    regex_replace 'GT_RESIDUAL_RECONSTRUCT' $residual_reconstruct

    mkdir build_$name
    cd build_$name
    
    cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release .. 
    make
}

(build_version dec_order            1 0 0)
(build_version pruning              0 1 0)
(build_version residual_reconstruct 0 0 1)
