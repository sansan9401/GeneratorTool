#!/bin/bash

if [ -z "$1" ]
then
    echo "usage: $0 EVENT_DIR"
    echo "example: $0 Sherpa/Event/ttH"
fi
EVENT_DIR=$1

echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "it is not 10_6_0... Exiting...";exit 1; }

files=($(find $dir -maxdepth 2 -name "*.root"|egrep "run|job"))
echo ${files[@]}
for file in "${files[@]}"
do
    i=$(dirname $file|xargs -i basename {}|grep -o "[0-9]*")
    echo 'echo -e ".L script.cc\n loop(\"'$file'\",\"'$dir'/out'$i'.root\");\n .q"|root -b'|condor_qsub -V -cwd
done

while [ $(ls -1 $dir/out*.root|wc -l) -ne ${#files[@]} ]
do 
    sleep 20
done
sleep 10
hadd -f $dir/hists.root $dir/out*.root
rm $dir/out*.root
rm STDIN.*
rm STDIN_qsub*
