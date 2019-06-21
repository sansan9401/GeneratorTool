#!/bin/bash

export SHERPADAY_BASE=`pwd`
if [[ $HOSTNAME == *"tamsa"* ]]
then
    export SHERPADAY_USECONDOR=1
fi
export SHERPADAY_USECONDOR

source /cvmfs/cms.cern.ch/cmsset_default.sh

echo "[SherpaDay] check CMSSW_10_6_0 for sherpa"
if [ ! -d "$SHERPADAY_BASE/Tool/CMSSW_10_6_0" ]
then
    echo "[SherpaDay]   No CMSSW_10_6_0; Cloning CMSSW_10_6_0;"
    cd $SHERPADAY_BASE/Tool
    cmsrel CMSSW_10_6_0
    cd CMSSW_10_6_0/src/
    ( eval `scramv1 runtime -sh`;
	git cms-addpkg GeneratorInterface/SherpaInterface;
	echo "[SherpaDay] PATCH: chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh";
	chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh; )
    cd $SHERPADAY_BASE 
fi

echo "[SherpaDay] check genproductions for madgraph"
if [ ! -d "$SHERPADAY_BASE/Tool/genproductions" ]
then
    echo "[SherpaDay]   No gen productions; Cloning genproductions;"
    cd $SHERPADAY_BASE/Tool
    git clone https://github.com/cms-sw/genproductions
    EXECUTABLE=$SHERPADAY_BASE/Tool/genproductions/bin/MadGraph5_aMCatNLO/gridpack_generation.sh
    echo "[SherpaDay] PATCH: chmod +x $EXECUTABLE"
    chmod +x $EXECUTABLE
    echo "[SherpaDay] PATCH: enable to set nb_core"
    echo [SherpaDay] sed -i '/set run_mode 2/a echo "set nb_core \${NB_CORE:=16}" >> mgconfigscript' $EXECUTABLE
    sed -i '/set run_mode 2/a \          echo "set nb_core \${NB_CORE:=16}" >> mgconfigscript' $EXECUTABLE
    if cat /etc/*release|grep ^VERSION_ID|grep 7
    then
	echo "[SherpaDay] PATCH: change to slc7"
	echo [SherpaDay] sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
	sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
    fi
    cd $SHERPADAY_BASE
fi

if [[ "$1" != "nocmsenv" ]]
then
    echo "[SherpaDay] setup cmsenv"
    cd $SHERPADAY_BASE/Tool/CMSSW_10_6_0/src
    eval `scramv1 runtime -sh`
    cd $SHERPADAY_BASE
fi
