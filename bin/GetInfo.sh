#!/bin/bash

if [ -z "$1" -o -z "$2" ]
then
    echo "usage: $0 Sherpa|MG PROCESSNAME"
    echo "example: $0 Sherpa ttH"
    exit 1
fi
GENERATOR=$1
PROCESSNAME=$2

function GetCpus { 
    rootoutput=$(echo '.L script/util.cc
int aa=GetCpus("'$GENERATORTOOLS_BASE/$1/Gridpack/$2'");
cout<<aa;
.q'|root -l -b)
    echo $rootoutput
}
function GetGridpackGenerationSpeed { 
    rootoutput=$(echo '.L script/util.cc
double aa=GetGridpackGenerationSpeed("'$GENERATORTOOLS_BASE/$1/Gridpack/$2'");
cout<<aa;
.q'|root -l -b)
    echo $rootoutput
}
function GetTune {
    PYTHON_CFF=$GENERATORTOOLS_BASE/$1/Gridpack/$2/${2}.py
    if grep -q pythia8CUEP8M1Settings $PYTHON_CFF
    then
	echo $(grep -o pythia8CUEP8M1Settings $PYTHON_CFF|tail -n1)
    elif grep -q "pythia8CP[^ ]*Settings" $PYTHON_CFF
    then
	echo $(grep -o "pythia8CP[^ ]*Settings" $PYTHON_CFF|tail -n1)
    else
	echo Only for MG
    fi
}	
function GetCrossSection { 
    rootoutput=$(echo '.L script/util.cc
tuple<double,double> aa=GetCrossSectionAndStatError("'$GENERATORTOOLS_BASE/$1/Event/$2'");
cout<<get<0>(aa)<<" +- "<<get<1>(aa);
.q'|root -l -b)
    echo $rootoutput
}
function GetNEvent {
    rootoutput=$(echo '.L script/util.cc
int aa=GetNEvent("'$GENERATORTOOLS_BASE/$1/Event/$2'");
cout<<aa;
.q'|root -l -b)
    echo $rootoutput
}    
function GetEventGenerationSpeed { 
    rootoutput=$(echo '.L script/util.cc
tuple<double,double> aa=GetEventGenerationSpeedAndError("'$GENERATORTOOLS_BASE/$1/Event/$2'");
cout<<get<0>(aa)<<" +- "<<get<1>(aa);
.q'|root -l -b)
    echo $rootoutput
}
function GetNegativeWeight { 
    rootoutput=$(echo '.L script/util.cc
tuple<double,double> aa=GetNegativeWeightAndError("'$GENERATORTOOLS_BASE/$1/Event/$2'");
cout<<get<0>(aa)<<" +- "<<get<1>(aa);
.q'|root -l -b)
    echo $rootoutput
}

echo $GENERATOR $PROCESSNAME
if [ -e $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME ]
then
    echo "gridpack generation speed = "$(GetGridpackGenerationSpeed $GENERATOR $PROCESSNAME)" min using "$(GetCpus $GENERATOR $PROCESSNAME) "cores"
    [ "$GENERATOR" = MG ] && echo "tune = "$(GetTune $GENERATOR $PROCESSNAME)
else
    echo "NO GRIDPACK in $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME"
fi
if [ -e $GENERATORTOOLS_BASE/$GENERATOR/Event/$PROCESSNAME ]
then
    echo "nevent = "$(GetNEvent $GENERATOR $PROCESSNAME)
    echo "cross section = "$(GetCrossSection $GENERATOR $PROCESSNAME)" pb"
    echo "event generation speed = "$(GetEventGenerationSpeed $GENERATOR $PROCESSNAME)" events/day"
    echo "negative weight fraction = "$(GetNegativeWeight $GENERATOR $PROCESSNAME)
else
    echo "NO EVENT in $GENERATORTOOLS_BASE/$GENERATOR/Gridpack/$PROCESSNAME"
fi
