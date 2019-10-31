#!/bin/bash                                                                                                                                                                                                 

if [ -z "$1" -o -z "$2" -o -z "$3" ]
then
    echo "usage: $0 MG SOURCE_PROCESSNAME TARGET_PROCESSNAME [TEMPLATE]"
    echo "example: $0 MG ttH ttH_CP3 python/MG_NLO_CP3_cff.py"
    exit 1
fi

test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
source $GENERATORTOOLS_BASE/bin/functions.sh
CheckCMSSW

GENERATOR=$1
SOURCE_PROCESSNAME=$2
TARGET_PROCESSNAME=$3
TEMPLATE=$4

echo "GENERATOR=$GENERATOR"
echo "SOURCE_PROCESSNAME=$SOURCE_PROCESSNAME"
echo "TARGET_PROCESSNAME=$TARGET_PROCESSNAME"
echo "TEMPLATE=$TEMPLATE"

if [ "$GENERATOR" = MG ]
then
    [ -e $GENERATORTOOLS_BASE/$GENERATOR/Card/$TARGET_PROCESSNAME ] && { echo target process card already exist; exit 1; }
    [ -e $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$TARGET_PROCESSNAME ] && { echo target process gridpack already exist; exit 1; }
    echo "Copying Card"
    cp -r $GENERATORTOOLS_BASE/$GENERATOR/Card/$SOURCE_PROCESSNAME $GENERATORTOOLS_BASE/$GENERATOR/Card/$TARGET_PROCESSNAME
    ( cd $GENERATORTOOLS_BASE/$GENERATOR/Card/$TARGET_PROCESSNAME;    rename $SOURCE_PROCESSNAME $TARGET_PROCESSNAME * )
    rename $SOURCE_PROCESSNAME $TARGET_PROCESSNAME *
    sed -i 's/'$SOURCE_PROCESSNAME'/'$TARGET_PROCESSNAME'/g' $GENERATORTOOLS_BASE/$GENERATOR/Card/$TARGET_PROCESSNAME/*
    echo "Copying Gridpack"
    cp -r $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$SOURCE_PROCESSNAME $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$TARGET_PROCESSNAME
    ( cd $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$TARGET_PROCESSNAME;    rename $SOURCE_PROCESSNAME $TARGET_PROCESSNAME * )

    if [ -z "$TEMPLATE" ]
    then
	MG_PYTHON_DIR=$GENERATORTOOLS_BASE/external/$CMSSW_VERSION/src/MY/mg/python
	sed 's/'$SOURCE_PROCESSNAME'/'$TARGET_PROCESSNAME'/g' $MG_PYTHON_DIR/${SOURCE_PROCESSNAME}.py > $MG_PYTHON_DIR/${TARGET_PROCESSNAME}.py
	ln -sf $MG_PYTHON_DIR/${TARGET_PROCESSNAME}.py $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$TARGET_PROCESSNAME/
	( cd $GENERATORTOOLS_BASE/external/$CMSSW_VERSION/src/MY/mg; scram build )
    else
	TEMPLATE=$(realpath $TEMPLATE)
	$GENERATORTOOLS_BASE/bin/SetTune.sh MG $TARGET_PROCESSNAME $TEMPLATE
    fi
else
    echo first argument should be MG
    exit 1
fi
