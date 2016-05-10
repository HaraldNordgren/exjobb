#!/bin/bash

for w in $1/*.svg; do
    rsvg-convert -f pdf -o ${w%.*}.pdf $w #&& rm $w
done

