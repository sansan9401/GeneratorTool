#!/bin/bash
test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
if [ -z "$1" ]
then
    echo "usage: $0 OPENLOOPSPATH"
    echo "example: $0 /data4/Users/hsseo/GeneratorTools/external/OpenLoops-2.1.0"
    exit 1
fi

OL_PREFIX=$(realpath $1)
echo OL_PREFIX=$OL_PREFIX

LIST=($(find $GENERATORTOOLS_BASE/Sherpa/Card -name "Run.dat*"))

for file in "${LIST[@]}"
do
    sed -i '/^[ ]*OL_PREFIX/d' $file
    sed -i '/^[ ]*(run)/a OL_PREFIX='$OL_PREFIX $file
done


