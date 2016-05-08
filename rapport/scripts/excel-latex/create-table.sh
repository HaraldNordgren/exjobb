#!/bin/bash

export b=bitrates.tex
export r=rate_reductions.tex
export c=costs.tex
export t=timing.tex
T=table.tex

set -e

script_dir=$(dirname $(readlink -f $0))
fix_tables="$script_dir/fix-excel-tables-in-dir.sh"

cd $1

$fix_tables

dir=$(basename $1)
dir_escaped=$(echo $dir | sed 's/_/{\\_}/g')

if [ -a $t ]; then
    files="rate_reductions costs timing"
else
    files="rate_reductions costs"
fi

echo '\begin{table}[htbp]' > $T
echo '  \centering' >> $T
echo '    \caption{'$dir_escaped'}' >> $T

cat bitrates.tex | sed 's/^/    /' >> $T
echo '  }' >> $T

for f in $files; do
    echo '  \newline\vspace*{0.5 cm}' >> $T
    cat ${f}.tex | sed 's/^/    /' >> $T
    echo '  }' >> $T
done

echo '  \label{tab:'$dir'}%' >> $T
echo '\end{table}%' >> $T
