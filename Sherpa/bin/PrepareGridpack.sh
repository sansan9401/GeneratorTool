#!/bin/bash

echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "it is not 10_6_0... Exiting...";exit 1; }

SHERPA_DATA_DIR=$CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data
GRIDPACK_DIR=$SHERPADAY_BASE/Sherpa/Gridpack
SHERPA_PYHTON_DIR=$CMSSW_BASE/src/MY/sherpa/python
mkdir -p $SHERPA_PYHTON_DIR
ls -1 $GRIDPACK_DIR|while read PROCESSNAME
do
    ORIGIN=$(readlink $GRIDPACK_DIR/$PROCESSNAME)
    if [ -f $ORIGIN/sherpa_${PROCESSNAME}_MASTER.tgz ] 
    then
	echo "$PROCESSNAME is already prepared... pass...";
    elif [ -f $ORIGIN/sherpa_${PROCESSNAME}_libs.tgz ]
    then
	cd $ORIGIN
	$SHERPA_DATA_DIR/PrepareSherpaLibs.sh
	sed -i '/FetchSherpack/ s/False/True/' sherpa_${PROCESSNAME}_MASTER_cff.py
	sed -i '/SherpackLocation/ s@\./@'$(pwd)'@' sherpa_${PROCESSNAME}_MASTER_cff.py
	mv sherpa_${PROCESSNAME}_MASTER_cff.py $SHERPA_PYHTON_DIR
	ln -sf $SHERPA_PYHTON_DIR/sherpa_${PROCESSNAME}_MASTER_cff.py sherpa_${PROCESSNAME}_MASTER_cff.py
	echo "Preparing $PROCESSNAME is done"
    else
	echo "$PROCESSNAME is not ready... pass..."
    fi
done
echo "Compile... scram build"
cd $SHERPA_PYHTON_DIR
scram b
