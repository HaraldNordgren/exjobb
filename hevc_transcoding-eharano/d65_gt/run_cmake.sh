#!/bin/bash

api=d65_lib/d65_api.h

function build_version {
    name=$1
    dec_order=$2
    pruning=$3
    res_rec=$4
    
    perl -pi -e 's/(#define D65_OUTPUT_IN_DEC_ORDER[\s]*)\d/${1}'${dec_order}'/' $api
    perl -pi -e 's/(#define GT_RESIDUAL_PRUNING[\s]*)\d/${1}'${pruning}'/' $api
    perl -pi -e 's/(#define GT_RESIDUAL_RECONSTRUCT[\s]*)\d/${1}'${res_rec}'/' $api
    
    mkdir build_$name
    cd build_$name
    
    cmake CMakeLists.txt -DCMAKE_BUILD_TYPE=Release .. 
    make
}

build_version dec_order    1 0 0
build_version pruning      0 1 0
build_version residual_rec 0 0 1
