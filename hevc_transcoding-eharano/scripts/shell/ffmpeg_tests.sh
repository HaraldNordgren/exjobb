#!/bin/bash

test_file_basename=test_data/smala_sussie_544x304_25
test_file_yuv=${test_file_basename}.yuv

cols=`tput cols`
(( cols-=2 ))

function call {
    "$@" 2>&1 | fold -s -w $cols | sed 's/^/  /g'
    echo
}

reset

rm ${test_file_basename}.*

call ffmpeg -ss 00:34:57 -t 00:00:5 -i ~/Videos/Smala\ Sussie\ Swedish\ Dvdrip\ Xvid\ -\ Virus.avi -c:v rawvideo -pix_fmt yuv420p $test_file_yuv

call ./scripts/python/decode_and_mux.py $test_file_yuv
