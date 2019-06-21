#!/bin/bash

export GENERATORTOOL_BASE=`pwd`
if [[ $HOSTNAME == *"tamsa"* ]]
then
    export GENERATORTOOL_USECONDOR=1
fi
export GENERATORTOOL_USECONDOR

source /cvmfs/cms.cern.ch/cmsset_default.sh

echo "[GeneratorTool] check CMSSW_10_6_0 for sherpa"
if [ ! -d "$GENERATORTOOL_BASE/Tool/CMSSW_10_6_0" ]
then
    echo "[GeneratorTool]   No CMSSW_10_6_0; Cloning CMSSW_10_6_0;"
    cd $GENERATORTOOL_BASE/Tool
    cmsrel CMSSW_10_6_0
    cd CMSSW_10_6_0/src/
    ( eval `scramv1 runtime -sh`;
	git cms-addpkg GeneratorInterface/SherpaInterface;
	echo "[GeneratorTool] PATCH: chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh";
	chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh; )
    cd $GENERATORTOOL_BASE 
fi

echo "[GeneratorTool] check genproductions for madgraph"
if [ ! -d "$GENERATORTOOL_BASE/Tool/genproductions" ]
then
    echo "[GeneratorTool]   No gen productions; Cloning genproductions;"
    cd $GENERATORTOOL_BASE/Tool
    git clone https://github.com/cms-sw/genproductions
    EXECUTABLE=$GENERATORTOOL_BASE/Tool/genproductions/bin/MadGraph5_aMCatNLO/gridpack_generation.sh
    echo "[GeneratorTool] PATCH: chmod +x $EXECUTABLE"
    chmod +x $EXECUTABLE
    echo "[GeneratorTool] PATCH: enable to set nb_core"
    echo [GeneratorTool] sed -i '/set run_mode 2/a echo "set nb_core \${NB_CORE:=16}" >> mgconfigscript' $EXECUTABLE
    sed -i '/set run_mode 2/a \          echo "set nb_core \${NB_CORE:=16}" >> mgconfigscript' $EXECUTABLE
    if cat /etc/*release|grep ^VERSION_ID|grep 7
    then
	echo "[GeneratorTool] PATCH: change to slc7"
	echo [GeneratorTool] sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
	sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
    fi
    cd $GENERATORTOOL_BASE
fi

if [[ "$1" != "nocmsenv" ]]
then
    echo "[GeneratorTool] setup cmsenv"
    cd $GENERATORTOOL_BASE/Tool/CMSSW_10_6_0/src
    eval `scramv1 runtime -sh`
    cd $GENERATORTOOL_BASE
fi
