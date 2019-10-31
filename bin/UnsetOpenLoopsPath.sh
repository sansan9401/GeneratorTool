#!/bin/bash
test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }

LIST=($(find $GENERATORTOOLS_BASE/Sherpa/Card -name "Run.dat*"))

for file in "${LIST[@]}"
do
    sed -i '/^[ ]*OL_PREFIX/d' $file
done


