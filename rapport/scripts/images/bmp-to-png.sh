#!/bin/bash

for w in $1/*.bmp; do
    convert $w ${w%.*}.png && rm $w 
done
