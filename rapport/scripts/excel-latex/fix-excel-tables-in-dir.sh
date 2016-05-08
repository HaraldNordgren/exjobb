#!/bin/bash

#b=bitrates.tex
#r=rate_reductions.tex
#c=costs.tex
#t=timing.tex

perl -pi -e 's/(^\\begin)/\\makebox[\\textwidth][c]\{$1/' *.tex

sed -i 's/{rrrrrrrrrr}/{l|rrrr|rrr|rr}/' $b
sed -i 's/{rrrrrrrr}/{l|rrr|rrr|r}/' $r $c

sed -i 's/multicolumn{3}{c}/multicolumn{3}{c|}/g' $b $r $c
sed -i 's/multicolumn{4}{c}/multicolumn{4}{c|}/g' $b

sed -i 's/Kimono1/Kimono/g' *.tex

sed -i 's/{QP}/{$QP_{base}$}/g' $b $r $c
sed -i 's/{QP+2}/{$QP_{base}+2$}/g' $b $r $c

if [ -a $t ]; then
    sed -i 's/{rrrrrrrr}/{l|rr|rr|rr|r}/' $t
    sed -i 's/multicolumn{2}{c}/multicolumn{2}{c|}/g' $t
fi

