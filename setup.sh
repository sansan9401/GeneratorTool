#!/bin/bash

## If you want to generate many events, use external storage.
#GENERATORTOOLS_STORAGE=""

## CMSSW version for Sherpa generation
export GENERATORTOOLS_CMSSW_VERSION=CMSSW_10_2_18

## You can set genproductions brach name, 
GENPRODUCTIONS_BRANCH=""

export GENERATORTOOLS_BASE=`pwd`

DIRECTORIES=( "/MG/Gridpack" "/MG/Event" "/Sherpa/Event" "/Sherpa/Gridpack" )
if [ -n "$GENERATORTOOLS_STORAGE" ];then
    echo "Using external storage $GENERATORTOOLS_STORAGE"
    for DIRECTORY in "${DIRECTORIES[@]}";do
	if [ ! -L $GENERATORTOOLS_BASE$DIRECTORY ] && [ -d $GENERATORTOOLS_BASE$DIRECTORY ];then
	    echo "  [WARNING] $DIRECTORY already exist"
	else
	    mkdir -p $GENERATORTOOLS_STORAGE$DIRECTORY
	    ln -nsf $GENERATORTOOLS_STORAGE$DIRECTORY $GENERATORTOOLS_BASE$DIRECTORY
	fi
    done
else
    for DIRECTORY in "${DIRECTORIES[@]}";do
	mkdir -p $GENERATORTOOLS_BASE$DIRECTORY
    done
fi

if condor_status &> /dev/null
then
    echo "[GeneratorTools] using condor"
    export GENERATORTOOLS_USECONDOR=1
fi

source /cvmfs/cms.cern.ch/cmsset_default.sh
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
    [ -n "$GENPRODUCTIONS_BRANCH" ] && ( cd genproductions; git checkout -t origin/$GENPRODUCTIONS_BRANCH; )

    GRIDPACK_GENERATION=$GENERATORTOOLS_BASE/external/genproductions/bin/MadGraph5_aMCatNLO/gridpack_generation.sh
    echo "[GeneratorTools] PATCH: chmod +x $GRIDPACK_GENERATION"
    chmod +x $GRIDPACK_GENERATION

    echo "[GeneratorTools] PATCH: enable to set nb_core"
    echo [GeneratorTools] sed -i '/set run_mode 2/a echo "set nb_core \${NB_CORE:=1}" >> mgconfigscript' $GRIDPACK_GENERATION
    sed -i '/set run_mode 2/a \          echo "set nb_core \${NB_CORE:=1}" >> mgconfigscript' $GRIDPACK_GENERATION

    RUNCMSGRID_NLO=$GENERATORTOOLS_BASE/external/genproductions/bin/MadGraph5_aMCatNLO/runcmsgrid_NLO.sh
    echo "[GeneratorTools] PATCH: do not run extra systematics"
    echo [GeneratorTools] sed -i '/systematics/d' $RUNCMSGRID_NLO
    sed -i '/systematics/d' $RUNCMSGRID_NLO

    cd $GENERATORTOOLS_BASE
fi

## TH4D 
if [ -e $GENERATORTOOLS_BASE/external/TH4D/TH4D.h ];then
    ( cd $GENERATORTOOLS_BASE/external/TH4D; make; )
    export LD_LIBRARY_PATH=$GENERATORTOOLS_BASE/external/TH4D:$LD_LIBRARY_PATH
fi

## workaround for MG error in gridpack generation with reweighting
[ -z "${PYTHONPATH:-}" ] && export PYTHONPATH=""

export ROOT_INCLUDE_PATH=$GENERATORTOOLS_BASE:$ROOT_INCLUDE_PATH
