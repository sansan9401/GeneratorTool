#!/bin/bash

#export LANG=C
export GENERATORTOOLS_BASE=`pwd`
if condor_status &> /dev/null
then
    echo "[GeneratorTools] using condor"
    export GENERATORTOOLS_USECONDOR=1
fi

source /cvmfs/cms.cern.ch/cmsset_default.sh
export GENERATORTOOLS_CMSSW_VERSION=CMSSW_10_2_18

echo "[GeneratorTools] check $GENERATORTOOLS_CMSSW_VERSION for sherpa"
if [ ! -d "$GENERATORTOOLS_BASE/external/$GENERATORTOOLS_CMSSW_VERSION" ]
then
    echo "[GeneratorTools]   No $GENERATORTOOLS_CMSSW_VERSION; Cloning $GENERATORTOOLS_CMSSW_VERSION;"
    cd $GENERATORTOOLS_BASE/external
    cmsrel $GENERATORTOOLS_CMSSW_VERSION
    cd $GENERATORTOOLS_CMSSW_VERSION/src/
    ( eval `scramv1 runtime -sh`;
	git cms-addpkg GeneratorInterface/SherpaInterface;
	echo "[GeneratorTools] PATCH: chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh";
	chmod +x $CMSSW_BASE/src/GeneratorInterface/SherpaInterface/data/MakeSherpaLibs.sh; )
    cd $GENERATORTOOLS_BASE 
fi

echo "[GeneratorTools] check genproductions for madgraph"
if [ ! -d "$GENERATORTOOLS_BASE/external/genproductions" ]
then
    echo "[GeneratorTools]   No gen productions; Cloning genproductions;"
    cd $GENERATORTOOLS_BASE/external
    git clone https://github.com/cms-sw/genproductions
    EXECUTABLE=$GENERATORTOOLS_BASE/external/genproductions/bin/MadGraph5_aMCatNLO/gridpack_generation.sh
    echo "[GeneratorTools] PATCH: chmod +x $EXECUTABLE"
    chmod +x $EXECUTABLE
    echo "[GeneratorTools] PATCH: enable to set nb_core"
    echo [GeneratorTools] sed -i '/set run_mode 2/a echo "set nb_core \${NB_CORE:=8}" >> mgconfigscript' $EXECUTABLE
    sed -i '/set run_mode 2/a \          echo "set nb_core \${NB_CORE:=8}" >> mgconfigscript' $EXECUTABLE
    if cat /etc/*release|grep ^VERSION_ID|grep 7
    then
	echo "[GeneratorTools] PATCH: change to slc7"
	echo [GeneratorTools] sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
	sed -i 's/slc6_amd64_gcc630/slc7_amd64_gcc630/' $EXECUTABLE
    fi
    cd $GENERATORTOOLS_BASE
fi

if [[ "$1" != "nocmsenv" ]]
then
    echo "[GeneratorTools] setup cmsenv"
    cd $GENERATORTOOLS_BASE/external/$GENERATORTOOLS_CMSSW_VERSION/src
    eval `scramv1 runtime -sh`
    cd $GENERATORTOOLS_BASE
fi
export ROOT_INCLUDE_PATH=$GENERATORTOOLS_BASE:$ROOT_INCLUDE_PATH
