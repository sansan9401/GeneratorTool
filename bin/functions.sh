#!/bin/bash

function CheckCMSSW {
    echo cmssw_base: $CMSSW_BASE
    echo cmssw_version: $CMSSW_VERSION
    test "$GENERATORTOOLS_CMSSW_VERSION" = "$CMSSW_VERSION" || { echo "it is not ${GENERATORTOOLS_CMSSW_VERSION}... Exiting...";exit 1; }
}

function GetNJetMax {
    NJETMAX=0
    NJETMIN=999
    while read -r line
    do
        NJET=$(echo $line|sed 's/ /\n/g'|grep -o "^j$"|wc -l)
        [ "$NJETMAX" -lt "$NJET" ] && NJETMAX=$NJET
	[ "$NJETMIN" -gt "$NJET" ] && NJETMIN=$NJET
    done < <(grep ">" $GENERATORTOOLS_BASE/$1/Card/$2/${2}_proc_card.dat)
    echo $(($NJETMAX-$NJETMIN))
}
