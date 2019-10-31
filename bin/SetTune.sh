#!/bin/bash

if [ -z "$1" -o -z "$2" -o -z "$3" ]
then
    echo "usage: $0 MG PROCESSNAME fragment_template"
    echo "example: $0 MG ttH python/MG_NLO_CP5_cff.py"
    exit 1
fi

test -z "$GENERATORTOOLS_BASE" && { echo "Please source setup.sh";exit 1; }
source $GENERATORTOOLS_BASE/bin/functions.sh
CheckCMSSW

GENERATOR=$1
PROCESSNAME=$2
TEMPLATE=$(realpath $3)
NJETMAX=$(GetNJetMax $GENERATOR $PROCESSNAME)

echo GENERATOR=$GENERATOR
echo PROCESSNAME=$PROCESSNAME
echo TEMPLATE=$TEMPLATE
echo NJETMAX=$NJETMAX

MG_PYTHON_DIR=$GENERATORTOOLS_BASE/external/$GENERATORTOOLS_CMSSW_VERSION/src/MY/mg/python
mkdir -p $MG_PYTHON_DIR
GRIDPATH=$GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME

cp $TEMPLATE $MG_PYTHON_DIR/${PROCESSNAME}.py
sed -i 's@GRIDPACKLOCATION@'$(find $GRIDPATH -type f -name "${PROCESSNAME}_*_tarball.tar.xz")'@' $MG_PYTHON_DIR/${PROCESSNAME}.py
sed -i 's@NJETMAX@'$NJETMAX'@' $MG_PYTHON_DIR/${PROCESSNAME}.py
ln -sf $MG_PYTHON_DIR/${PROCESSNAME}.py $GRIDPATH
cd $MG_PYTHON_DIR
scram build
