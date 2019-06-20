#!/bin/bash

if [ -z "$1" ]
then 
    echo "usage: $0 PROCESSNAME [N_Core]"
    exit 1
fi
PROCESSNAME=$1
NCORE=${2:-1}

echo cmssw_base: $CMSSW_BASE
echo cmssw_version: $CMSSW_VERSION|grep 10_6_0 || { echo "it is not 10_6_0... Exiting...";exit 1; }

CARD_DIR=$SHERPADAY_BASE/Sherpa/Card
if [ ! -f $CARD_DIR/Run.dat_$PROCESSNAME ]
then
    echo "no $CARD_DIR/Run.dat_$PROCESSNAME... Exiting..."
    exit 1
fi

SHERPA_DATA_DIR=$CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data
WORKING_DIR=$(mktemp -d -t ${PROCESSNAME}_XXXXXXXX --tmpdir=$SHERPA_DATA_DIR)
cd $WORKING_DIR
cp $CARD_DIR/Run.dat_$PROCESSNAME $WORKING_DIR
if [[ $SHERPADAY_USECONDOR ]]
then
    echo "time $SHERPA_DATA_DIR/MakeSherpaLibs.sh -v -o LBCR -p $PROCESSNAME -m mpirun -M '-n $NCORE'"|condor_qsub -cwd -V -l nodes=1:ppn=$NCORE -N sherpa_$PROCESSNAME
else 
    time $SHERPA_DATA_DIR/MakeSherpaLibs.sh -v -o LBCR -p $PROCESSNAME -m mpirun -M '-n $NCORE'
fi

$SHERPA_DATA_DIR/PrepareLibs.sh 
ln -sf $WORKING_DIR $SHERPADAY_BASE/Sherpa/Gridpack/$PROCESSNAME
